static char sccsid[] = "@(#)79	1.18  src/bos/kernel/proc/POWER/simple_lock_ppc.c, sysproc, bos41J, 9516A_all 4/13/95 15:10:20";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_mine_instr_ppc
 *		lock_mine_ppc
 *		slock_instr_ppc
 *		slock_ppc
 *		sunlock_instr_ppc
 *		sunlock_ppc
 *
 *   ORIGINS: 27, 83
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
 *	The lock word can be pageable, but must be in ordinary working
 *	storage.  It must not be in a persistent segment with journalling
 *	applied.  This is because the deadlock signal is not handled by
 *	these service routines.	 Paging errors on the lock word cause
 *	the kernel to panic.
 *
 * EXECUTION ENVIRONMENT:
 *	These functions are pinned and will only fault if called on a
 *	pageable stack or passed a pageable lock word.
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/ppda.h>
#include <sys/lock_def.h>
#include <sys/atomic_op.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/prio_calc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>

extern int	get_processor_id();

/* 
 * a thread is allowed to spin if the owner of the lock is
 * running or if it has spun less than maxspin cycles
 */
#define SIMPLE_LOCK_CAN_SPIN(t,n)	\
	((t->t_state == TSRUN) && (t->t_wtype == TNOWAIT) && (n<maxspin))

/*
 * NAME: slock
 *
 * FUNCTION: acquire logical lock.
 *
 * NOTE: see assembler fast/path and front end in simple_lock_powerpc.s
 *	 called from thread environment only and only if the owner of
 *	 the lock is a thread
 *
 * RETURNS: no return	  
 *
 */
#ifdef _INSTRUMENTATION
void
slock_instr_ppc(register simple_lock_t lockaddr, register int link_register)
#else
void
slock_ppc(register simple_lock_t l, register int link_register)
#endif

{
	register struct thread *myself = curthread;	/* current thread */
	register struct thread *ownt;			/* lock's owner */
	register int lockword;				/* lockword value */
	register int spincount = 0;			/* count spin cycles */
	register int hadkernel_lock = 0;		/* had kernel_lock */
	int old_value;					/* initial lock value */
	register int ipri;				/* saved int priority */
	register int lname = NULL;			/* lock symbolic name */
        register int priority_boosted = FALSE;
        register tid_t owner;
#ifdef _INSTRUMENTATION
	register boolean_t didsleep = FALSE;
	register int instrumented = (lockaddr->_slock & INSTR_ON);
	register struct lock_data_instrumented	*l;	/* to secondary struct */

	if (instrumented) {
		l = lockaddr->_slockp;
		lname = l->lockname;
	}
	else
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->intpri == INTBASE);

#ifdef _INSTRUMENTATION
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lockaddr,((simple_lock_t)l)->_slock,
			LOCK_SWRITE_TRACE,link_register,lname);
#endif

	for(;;) {

		/* Read lock word in a volatile fashion */
		lockword = old_value = LOADWCS((int *)l);

		if (!(old_value & OWNER_MASK)) {

			/* lock free: set owner id and lockbit 
			 * The counter of acquired lock is
			 * updated as well
			 */

			lockword |= myself->t_tid;

			if (compare_and_swap((atomic_p)l,&old_value,lockword)) {
#ifdef _POWER_MP
				isync_601();
#endif
				myself->t_lockcount++;

#ifdef _INSTRUMENTATION
				if(instrumented) {
					l->acquisitions++;
					l->misses++;
					if (didsleep) {
						l->sleeps++;
					}
#ifdef DEBUG
					l->lock_lr = link_register;
					l->lock_caller = myself->t_tid;
					l->lock_cpuid = get_processor_id();
#endif
				}

				TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,
					((simple_lock_t)l)->_slock,LOCK_SWRITE_TRACE,
					link_register,lname);
#endif
				break;
			}
		}
			/* lock miss */
		else {
			/* unsafe test on lock owner; recheck before going to 
			 * sleep 
			 */
			if (SIMPLE_LOCK_CAN_SPIN(
				THREADPTR(lockword & OWNER_MASK),spincount)) {
				spincount++;
			}

			else {		/* must sleep */

				/* the kernel lock is an automatic short term 
				 * lock - it is never held across waits EXCEPT 
				 * for kernel data page faults.	 Note that if 
				 * we have gotten here AND hold the kernel_lock,
				 * this lock request can't be for the 
				 * kernel_lock.	 The test for !hadkernel_lock 
				 * is in case we do the for loop more than once!
				 */
				if (! hadkernel_lock) {
					hadkernel_lock=IS_LOCKED(&kernel_lock);
					if (hadkernel_lock) 
						unlockl(&kernel_lock);
				}

				/* Need to disable before taking the INTERLOCK
				 * bit to ensure we don't get dispatched out while
				 * holding the INTERLOCK bit.
				 * 
				 * Also, touch the lockword immediately in case
				 * of page fault.
				 */

				ipri = i_disable(INTMAX);
				lockword = LOADWCS((int *)l);

				/* when going to sleep, acquiring the interlock 
				 * bit is needed in order to prevent the owner
				 * to release the lock concurrently.
				 */

				while(!test_and_set((atomic_p)l,INTERLOCK)); 

				/* safe check on lock owner running; interlock 
				 * bit is set so the owner cannot disappear.
				 * Checking that the lock hasn't been released 
				 * in the meantime is also needed.
				 */

                                owner = ((simple_lock_t)l)->_slock & OWNER_MASK;
                                ownt = THREADPTR(owner);

				if (!owner || 
				    SIMPLE_LOCK_CAN_SPIN(ownt,spincount) ) {	
					fetch_and_and((atomic_p)l,~INTERLOCK);
					i_enable(ipri);
					spincount++;	    
					continue; 
				} 
#ifdef _POWER_MP
				simple_lock(&proc_base_lock);
#endif
                		/*
                 		 * Since we are going to sleep on a lock, we could have
                 		 * a priority inversion problem.  This problem is 
				 * addressed by the dispatcher.  t_lockowner is a 
				 * parameter to the dispatcher.  
                 		 */
                		myself->t_lockowner = THREADPTR(owner);
                		ASSERT(myself->t_lockowner->t_tid == owner);

				((simple_lock_t)l)->_slock |= WAITING;

#ifdef _INSTRUMENTATION
				/* store real wait channel */
				myself->t_wchan1 = (char *)lockaddr;
				myself->t_wchan1sid = SRTOSID(mfsri((uint)lockaddr));
				myself->t_wchan1offset = SEGOFFSET(lockaddr); 
				wait_on_lock(lockq_hash(lockaddr), FALSE);
				didsleep = TRUE;
#else
				/* store real wait channel */
				myself->t_wchan1 = (char *)l;
				myself->t_wchan1sid = SRTOSID(mfsri((uint)l));
				myself->t_wchan1offset = SEGOFFSET(l); 
				wait_on_lock(lockq_hash(l),FALSE);
#endif

				myself->t_wtype = TWLOCK;

#ifdef _POWER_MP
				simple_unlock(&proc_base_lock);
#endif
				swtch();

				i_enable(ipri);
			}
		}
	}

	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
}

/*
 * NAME: sunlock
 *
 * FUNCTION: release logical lock.
 *
 * NOTES: 
 *	     see assembler fast/path and front end in simple_lock_ppc.s
 *	     This routine now supports being called directly.
 *
 *	     dispatch() will call sunlock() to unlock the proc_int_lock
 *	     which it may not own (but the owner is the thread being
 *	     taken out of execution).  As such, ownership is no longer
 *	     asserted here (still asserted in simple_unlock).  Also,
 *	     sunlock() can no longer assume the WAITING bit is set as it
 *	     could before. [ONLY IN DEBUG]
 *
 * RETURNS: no return	  
 *
 */

#ifdef _INSTRUMENTATION
void
sunlock_instr_ppc(simple_lock_t lockaddr, register int link_register)
#else
void
sunlock_ppc(simple_lock_t l, register int link_register)
#endif

{
	register int lockword;			/* lockword value */
	register boolean_t waiting = FALSE;	/* thread still waiting? */
	register int ipri;			/* saved int priority */
	register int lname = NULL;		/* lock symbolic name */
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* ptr to secondary structure */

	if (lockaddr->_slock & INSTR_ON) {
		l = lockaddr->_slockp;
		lname = l->lockname;
#ifdef DEBUG
		l->unlock_lr = link_register;
		l->unlock_caller = curthread->t_tid;
		l->unlock_cpuid = get_processor_id();
#endif
	}
	else
		l = (struct lock_data_instrumented *)lockaddr;
#endif
	ASSERT( ((simple_lock_t)l)->_slock & OWNER_MASK );

#ifdef _INSTRUMENTATION
	TRCHKL4T(HKWD_KERN_UNLOCK,lockaddr,((simple_lock_t)l)->_slock,link_register,lname);
#endif

	/* Need to disable before taking the INTERLOCK
	 * bit to ensure we don't get dispatched out while
	 * holding the INTERLOCK bit.
	 * 
	 * Also, touch the lockword immediately in case
	 * of page fault.
	 */
	
	ipri = i_disable(INTMAX);
	lockword = LOADWCS((int *)l);
	
	/* Acquiring the interlock bit is needed to serialize waking threads 
	 * sleeping on a wait queue and threads going to sleep on the same queue
	 */
	while (!test_and_set((atomic_p)l,INTERLOCK));

	/* if sunlock() is being called directly, WAITING bit may not be set */
#ifdef DEBUG
	if (((simple_lock_t)l)->_slock & WAITING)
#endif /* DEBUG */
	{
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
		simple_lock(&proc_int_lock);
#endif
#ifdef _INSTRUMENTATION
		waiting = wakeup_lock(lockq_hash(lockaddr), (void *)lockaddr,
				      WAKEUP_LOCK_SIMPLE);
#else
		waiting = wakeup_lock(lockq_hash(l), (void *)l,
				      WAKEUP_LOCK_SIMPLE);
#endif
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
		simple_unlock(&proc_base_lock);
#endif
	}

	/*
	 * decrement lockcount
	 */
	curthread->t_lockcount--;

	/* Only the highest priority thread is awakened; if other threads are 
	 * waiting for the lock to be released the WAITING bit is not cleared
	 */
	if (waiting) {
		((simple_lock_t)l)->_slock = SIMPLE_LOCK_AVAIL_WAITERS;
	}
	else {
		((simple_lock_t)l)->_slock = SIMPLE_LOCK_AVAIL;
	}

	/* INTERLOCK bit now free */
	i_enable(ipri);
}



/*
 * NAME: lock_mine
 *
 * FUNCTION: check if the current thread is the lock's owner
 *
 * NOTE: 
 *
 * RETURNS: TRUE if owns the lock
 *	    FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_mine_instr_ppc(void *lockaddr)
#else
boolean_t
lock_mine_ppc(void *l)
#endif

{
	register int lockword =

#ifdef _INSTRUMENTATION
		(((simple_lock_t)lockaddr)->_slock & INSTR_ON) ?
			LOADWCS((int *)(((simple_lock_t)lockaddr)->_slockp)) :
			LOADWCS((int *)lockaddr);
#else
		LOADWCS((int *)l);
#endif

	return (((lockword & OWNER_MASK) == curthread->t_tid) &&
		(!(lockword & READ_MODE)));
}
