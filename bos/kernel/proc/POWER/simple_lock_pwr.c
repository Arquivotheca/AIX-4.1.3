static char sccsid[] = "@(#)80	1.17  src/bos/kernel/proc/POWER/simple_lock_pwr.c, sysproc, bos41J, 9516A_all 4/13/95 15:10:13";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_mine_pwr
 *		slock_pwr
 *		sunlock_pwr
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
 *	     These functions are pinned and will only fault if called on a
 *	     pageable stack or passed a pageable lock word.
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/thread.h>
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

/*
 * NAME:     slock
 *
 * FUNCTION: acquire logical lock.
 *
 * EXECUTION ENVIRONMENT:
 *	     This routine is only called from the fast path assembler
 *	     routine (simple_lock) if the lock was busy.
 *
 * NOTES:
 *	     see assembler fast/path and front end in simple_lock_power.s
 *	     This routine is entered from simple_lock disabled to INTMAX.
 *
 * RETURNS:  nothing
 *
 */

void
slock_pwr(register simple_lock_t l,register int ipri, register int link_register)

{
	register struct thread *myself = curthread;	/* current thread */
	register struct thread *ownt;			/* lock's owner	  */
	register int lockword;				/* lockword value */
	register int hadkernel_lock = 0;		/* had kernel_lock */
        register int priority_boosted = FALSE;
        register tid_t owner;

	/* we come in disabled--verify that caller of simple_lock() was at INTBASE */
	ASSERT(ipri == INTBASE);

	/* the kernel lock is an automatic short term lock -
	 * it is never held across waits EXCEPT for kernel data
	 * page faults.	 The test for !hadkernel_lock is 
	 * in case we do the for loop more than once !
	 */
	if (! hadkernel_lock) {
		hadkernel_lock = IS_LOCKED(&kernel_lock);
		if (hadkernel_lock) unlockl(&kernel_lock);
	}
	
	for(;;){

		/* Read lock word in a volatile fashion */
		lockword =  LOADWCS((int *)l);

#ifdef _POWER_MP
		ASSERT(csa->intpri == INTMAX);
		simple_lock(&proc_base_lock);

		/* Read lock word in a volatile fashion */
		lockword =  LOADWCS((int *)l);
#endif

                owner = (lockword & OWNER_MASK);
                if (!owner){

			/* lock free: set owner id
			 * The counter of acquired lock is
			 * updated as well
			 */

			l->_slock |= myself->t_tid;
			myself->t_lockcount++;
#ifdef _POWER_MP
			simple_unlock(&proc_base_lock);
#endif
			break;
		}
			/* lock miss */
		else {

                	/*
                 	 * Since we are going to sleep on a lock, we could have
                 	 * a priority inversion problem.  This problem is addressed
                 	 * by the dispatcher.  t_lockowner is a parameter to the
                 	 * dispatcher.  
                 	 */
                	myself->t_lockowner = THREADPTR(owner);
                	ASSERT(myself->t_lockowner->t_tid == owner);

			/* set WAITING bit in the lockword */
			l->_slock |= WAITING;

			/* store real wait channel */
			myself->t_wchan1 = (char *)l;
			myself->t_wchan1sid = SRTOSID(mfsri((uint)l));
			myself->t_wchan1offset = SEGOFFSET(l); 

			wait_on_lock(lockq_hash(l),FALSE);

			myself->t_wtype = TWLOCK;
#ifdef _POWER_MP
			simple_unlock(&proc_base_lock);
#endif
			swtch();
		}
	}

	i_enable(ipri);

	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
}


/*
 * NAME:     sunlock
 *
 * FUNCTION: release logical lock.
 *
 * EXECUTION ENVIRONMENT:
 *	     This routine is only called from the fast path assembler
 *	     routine (simple_unlock) if there are waiters to be processed.
 *
 * NOTES: 
 *	     see assembler fast/path and front end in simple_lock_power.s
 *	     This routine now supports being called directly.
 *
 *	     dispatch() will call sunlock() to unlock the proc_int_lock
 *	     which it may not own (but the owner is the thread being
 *	     taken out of execution).  As such, ownership is no longer
 *	     asserted here (still asserted in simple_unlock).  Also,
 *	     sunlock() can no longer assume the WAITING bit is set as it
 *	     could before. [ONLY IN DEBUG]
 *
 * RETURNS:  nothing
 *
 */

void
sunlock_pwr(simple_lock_t l, register int link_register)

{
	register int ipri;			/* old priority	  */
	register boolean_t waiting = FALSE;	/* more than one thread waiting */

	ipri = i_disable(INTMAX);

	/* if sunlock() is being called directly, WAITING bit may not be set */
#ifdef DEBUG
	if (l->_slock & WAITING)
#endif /* DEBUG */
	{
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
		simple_lock(&proc_int_lock);
#endif
		waiting = wakeup_lock(lockq_hash(l), (void *)l, WAKEUP_LOCK_SIMPLE);
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
		simple_unlock(&proc_base_lock);
#endif
	}

	/*
	 * decrement lockcount
	 */
	curthread->t_lockcount--;

	/* Only the highest priority thread is awakened;
	 * if other threads are waiting for the lock to
	 * be released the WAITING bit is not cleared
	 */
	if (waiting) {
		l->_slock = SIMPLE_LOCK_AVAIL_WAITERS;
	}
	else {
		l->_slock = SIMPLE_LOCK_AVAIL;
	}

	i_enable(ipri);
}


/*
 * NAME:     lock_mine
 *
 * FUNCTION: check if the current thread/process is the lock's owner
 *
 * EXECUTION ENVIRONMENT:
 *	     This routine is exported and can be called from anywhere.
 *
 * NOTES: 
 *
 * RETURNS: 
 *	     TRUE if owns the lock
 *	     FALSE otherwise
 */

boolean_t
lock_mine_pwr(void *l)

{
	register int lockword = LOADWCS((int *)l);

	return (((lockword & OWNER_MASK) == curthread->t_tid) &&
		(!(lockword & READ_MODE)));
}
