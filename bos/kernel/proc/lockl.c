static char sccsid[] = "@(#)53	1.57  src/bos/kernel/proc/lockl.c, sysproc, bos41J, 9519B_all 5/11/95 15:02:22";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: klockl_instr
 *		klockl
 *		kunlockl_instr
 *		kunlockl
 *		lockl_mine
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * NOTES:
 *
 *	All lock words have the same format.  The lock word is either
 *	LOCK_AVAIL, or contains the process ID of the last entry in
 *	a circular chain of proc structs.  Since the list is circular,
 *	the last entry points to the first entry.  Processing is FIFO.
 *	This is desireable to avoid starvation and convoying.  The chain
 *	pointer is p_next, which is a proc structure pointer.
 *
 *	The lock word can be pageable, but must be in ordinary working
 *	storage.  It must not be in a persistent segment with journalling
 *	applied.  This is because the deadlock signal is not handled by
 *	these service routines.  Paging errors on the lock word cause
 *	the kernel to panic.
 *								
 * EXECUTION ENVIRONMENT:
 *
 *	These functions are pinned and will only fault if called on a
 *	pageable stack or passed a pageable lock word.
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/atomic_op.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/sleep.h>
#include <sys/prio_calc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/systemcfg.h>
#include "sig.h"

extern lock_t	kernel_lock;			/* global kernel lock */
extern void     remove_lock_list();
extern void 	prio_requeue();


/*
 * NAME: klockl_instr
 *       klockl
 *
 * FUNCTION: acquire logical lock.
 *
 * RETURNS:	LOCK_SUCC	if lock newly acquired by this thread
 *		LOCK_NEST	if this thread already owned the lock
 *		LOCK_FAIL	if LOCK_NDELAY specified and lock not available
 *		Lock_Sig	If Lock wait interrupted by a signal
 */

#ifdef _INSTRUMENTATION
int
klockl_instr(register lock_t *lock_word, register int flags, register int ipri)
#else
int
klockl(register lock_t *lock_word, register int flags, register int ipri) 
#endif
{
	register struct thread *t = curthread;	/* current thread pointer */
	register struct thread *ownt;		/* lock owner thread */
	register int 	owner;			/* lock owner tid on entry */
	register int 	rc;			/* return code */
	register int	hadkernel_lock = 0;	/* held the kernel lock */
	int 		old_val;		/* original lockword value */
	register int	link_register;
        register int    priority_boosted = FALSE;
#ifdef _INSTRUMENTATION
	register boolean_t didsleep = FALSE;
	register struct lock_data_instrumented *l = lockl_hash(lock_word);
#endif

	ASSERT(csa->prev == NULL);	
	ASSERT(csa->intpri == INTMAX); 
	ASSERT(!IS_LOCKED(lock_word));		/* handled by front end */
	ASSERT(!(flags & LOCK_NDELAY)); 	/* handled by front end */
#ifdef _POWER_MP
	ASSERT(!lock_mine(&proc_int_lock));
	ASSERT(!lock_mine(&proc_base_lock));
#endif

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	for (;;)  {			/* keep trying until successful */

		/*
		 * Try to store my TID in the lock word, if it is available
		 */
					/* WARNING: this may page fault */
					/* so touch carefully */
		old_val = LOADWCS(lock_word);	/* special case for APC */

		owner = old_val & LOCKL_OWNER_MASK;

		if (old_val == LOCK_AVAIL)  {	/* lock was available? */
                        if (!compare_and_swap( (atomic_p)lock_word,
					       &old_val, t->t_tid) )
				/* compare_and_swap failed */
                                continue;
#ifdef _POWER_MP
#ifdef _POWER_PC
			if (__power_pc())
				isync_601();
#endif
#endif
			t->t_lockcount++;
			/* record lock taken */
			TRCHKL5T(HKWD_KERN_LOCKL|hkwd_LOCK_TAKEN,lock_word,
				 *lock_word,t->t_tid,link_register,flags);
			rc = LOCK_SUCC;		/* successfully acquired lock */
			break;
		}

		/* the kernel lock is an automatic short term lock -
		 * it is never held across waits EXCEPT for kernel data
		 * page faults.  Note that if we have gotten here AND
		 * hold the kernel_lock, this lock request can't be for
		 * the kernel_lock.  The test for !hadkernel_lock is incase
		 * we do the for loop more than once !
		 */
		if (! hadkernel_lock) {
			hadkernel_lock = IS_LOCKED(&kernel_lock);
			if (hadkernel_lock) unlockl(&kernel_lock);
		}

#ifdef _POWER_MP
		/* The INTERLOCK bit cannot be set under two conditions:
		 * (1) already set by another thread
		 * (2) lock is freed by another thread (value == LOCK_AVAIL)
		 *
		 * If (1) is the case, it won't hurt to retry the lock...
		 * 
		 * If (2) is the case, a 'deadlock' can occur as we will
		 * NEVER get the INTERLOCK bit unless another thread is lucky
		 * enough to grab it within the lwarx/stwcx pair inside
		 * test_and_set() [which would be rare].
		 *
		 * In either case, if we cannot set the INTERLOCK bit, we
		 * retry from the top of the loop.  
		 */ 
		if (!test_and_set((atomic_p)lock_word, INTERLOCK))
			continue;

		simple_lock(&proc_base_lock);
		old_val = LOADWCS(lock_word);
#endif

		/* record miss of lock */
		TRCHKL5T(HKWD_KERN_LOCKL|hkwd_LOCK_MISS,lock_word,
			 *lock_word,t->t_tid,link_register,flags);

		owner = old_val & LOCKL_OWNER_MASK;

		/*
		 *  Check for wait with signals enabled?
		 */
		if (flags & (LOCK_SIGWAKE|LOCK_SIGRET))  {
			register struct proc *p = t->t_procp;

			/* If signal is available */
			if (SIG_MAYBE(t,p) && check_sig(t, 1)) { 
				t->t_flags |= TSIGAVAIL;
				t->t_flags &= ~TWAKEONSIG;
#ifdef _POWER_MP
				simple_unlock(&proc_base_lock);
				fetch_and_and((atomic_p)lock_word, ~INTERLOCK);
#endif
				if (flags & LOCK_SIGRET) {
					rc = LOCK_SIG;
					break;
				}
				else {
					i_enable(INTBASE);
					longjmpx(EINTR);
				}	
			}
			else
				t->t_flags |= TWAKEONSIG;
		}

                /*
		 * Since we are going to sleep on a lock, we could have
		 * a priority inversion problem.  This problem is addressed
		 * by the dispatcher.  t_lockowner is a parameter to the 
		 * dispatcher.  
                 */
		t->t_lockowner = THREADPTR(owner);
		ASSERT(t->t_lockowner->t_tid == owner);
		
		/*
		 * We will have to wait.  Therefore, we should not be
		 * on a locklist chain (if this is not the first time
		 * around the loop, we were either awaken by unlockl or
		 * we were signalled.  unlockl takes us off the list
		 * and, if signalled, we took ourselves off the list).
		 */
		ASSERT(t->t_slist == NULL);

		/* Indicate at least one sleeper on this lock */
		*lock_word |= WAITING;

		/* store real wait channel */
		t->t_wchan1 = (char *)lock_word;
		t->t_wchan1sid = SRTOSID(mfsri((uint)lock_word));
		t->t_wchan1offset = SEGOFFSET(lock_word);
 		wait_on_lock(locklq_hash(lock_word),
				(flags & (LOCK_SIGWAKE|LOCK_SIGRET)));

		if (t->t_procp->p_pid > 0)	/* if we're not booting  */
			U.U_ru.ru_nvcsw++;

#ifdef _INSTRUMENTATION
		didsleep = TRUE;
#endif

                /*
                 * record local threads sleeping
                 */
                if (t->t_flags & TLOCAL)
                        t->t_procp->p_local--;
                ASSERT(t->t_procp->p_local >=0);

		/*  Put this thread to sleep in a lock wait.  The
		 *  thread regains control after the call to swtch().
		 */
		t->t_wtype = TWLOCK;

                /*
                 * The proc_base_lock needs to be held into the dispatcher
                 * for interruptible sleeps to forestall intervening SIGSTOPs
                 * and SIGCONTs to the process.  Otherwise the thread will
                 * be put onto the runqueue twice.  The first time by cont()
                 * the second time by the dispatcher.
                 */
#ifdef _POWER_MP
                if (t->t_flags & TWAKEONSIG)
                        locked_swtch();
                else {
                        simple_unlock(&proc_base_lock);
                        swtch();
                }
                simple_lock(&proc_base_lock);
#else
                swtch();
#endif

                if (t->t_flags & TLOCAL)
                        t->t_procp->p_local++;

		/*
		 * Unset TWAKEONSIG flag; it is less efficient to record and
		 * check if TWAKEONSIG was set before the swtch().
		 */
		t->t_flags &= ~TWAKEONSIG;

		/*
		 * if we are still on the list take ourselves off the list.
                 * This occurs only as the result of a signal.
                 */
		if (t->t_slist != NULL) {
			register struct thread *prevt; 

			prevt = t->t_slist; 	/* start with next in list */

			/* loop around 'til we find us */
			while(prevt->t_slist != t)
				prevt = prevt->t_slist;

			remove_lock_list(prevt, t);
		}
		ASSERT(t->t_slist == NULL);

#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif
	}

	i_enable(ipri);			/* end critical section */

#ifdef _INSTRUMENTATION
	if (rc == LOCK_SUCC) {
		l->acquisitions++;
	}
	l->misses++;
	if (didsleep) {
		l->sleeps++;
	}
#endif
	/* this can't deadlock since NOONE can hold the kernel_lock while
	 * waiting for any other lock - or in fact anything else except
	 * a kernel page fault.  The page fault handler and device drivers
 	 * never wait for the kernel_lock
 	 */
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);

	return(rc);			/* critical section return code */
}

/*
 * NAME: kunlockl_instr
 *       kunlockl
 *
 * FUNCTION: release a logical lock and ready all waiters.
 *
 */

#ifdef _INSTRUMENTATION
void
kunlockl_instr(lock_t *lock_word, register int ipri)
#else
void
kunlockl(lock_t *lock_word, register int ipri)
#endif
{
	register struct thread *t = curthread; /* current thread pointer */
	register struct thread *lastt;	/* last thread on list */
	register int	lock_value;	/* holds lock word */
	register int	link_register;

	ASSERT (csa->prev == NULL);	/* MUST be a thread to lock */
	ASSERT (csa->intpri == INTMAX); /* MUST be disabled to INTMAX */

#ifdef _POWER_MP
	/*
	 * callers of lockl() and unlockl() cannot be holding the
	 * proc_int_lock as we can deadlock on it if we need to go to sleep.
	 */
	ASSERT(!lock_mine(&proc_base_lock));
	ASSERT(!lock_mine(&proc_int_lock));
#endif

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	/*
	 *  The first reference to the lock word within this critical
	 *  section may page fault.  Because the "assert" does a load
	 *  before the new value is stored, the store will not fault,
	 *  so a misaligned lock word will not be partially updated.
	 */
	lock_value = LOADWCS(lock_word); /* carefully touch the lock word */

#ifdef _POWER_MP
	while (!test_and_set((atomic_p)lock_word,INTERLOCK));
	lock_value = LOADWCS(lock_word);
	simple_lock(&proc_base_lock);
	simple_lock(&proc_int_lock);
#endif

	/*
	 * At least one thread MUST BE waiting on the newly released lock.
	 */
	ASSERT(lock_value & WAITING);

	(void) wakeup_lock(locklq_hash(lock_word), lock_word, WAKEUP_LOCK_ALL);
	
	*lock_word = LOCK_AVAIL;
	t->t_lockcount--;

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock); /* end critical section */
}

#ifndef _INSTRUMENTATION
/*
 * NAME: lockl_mine
 *
 * FUNCTION: check if the current thread is the lockl lock's owner
 *
 * RETURNS:  TRUE if owns the lock
 *           FALSE otherwise
 *
 * NOTES:
 * This file is compiled twice:  w/w/o _INSTRUMENTATION defined.
 *
 * lockl_mine is instrumenation-independant, so it only needs to be compiled once.
 * The "#ifndef _INSTRUMENTATION" suppresses the duplication function warning during
 * linking.
 */
boolean_t
lockl_mine(lock_t *lock_word)
{
	return(IS_LOCKED(lock_word));
}
#endif
