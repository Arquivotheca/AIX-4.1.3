static char sccsid[] = "@(#)75	1.22  src/bos/kernel/proc/POWER/complex_lock_ppc.c, sysproc, bos41J, 9516A_all 4/13/95 15:09:55";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_done_instr_ppc
 *		lock_done_ppc
 *		lock_read_instr_ppc
 *		lock_read_ppc
 *		lock_read_to_write_instr_ppc
 *		lock_read_to_write_ppc
 *		complex_lock_sleep_instr_ppc
 *		complex_lock_sleep_ppc
 *		lock_try_read_instr_ppc
 *		lock_try_read_ppc
 *		lock_try_read_to_write_instr_ppc
 *		lock_try_read_to_write_ppc
 *		lock_try_write_instr_ppc
 *		lock_try_write_ppc
 *		lock_write_instr_ppc
 *		lock_write_ppc
 *		lock_write_to_read_instr_ppc
 *		lock_write_to_read_ppc
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
 *	Read shared/Write exclusive lock.
 *	For layout of lock status word look at sys/lock_def.h
 *	The lock word can be pageable, but must be in ordinary working
 *	storage.  It must not be in a persistent segment with journalling
 *	applied.  This is because the deadlock signal is not handled by
 *	these service routines.	 Paging errors on the lock word cause
 *	the kernel to panic.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	These functions are pinned and will only fault if called on a
 *	pageable stack or passed a pageable lock word.
 *	Called from process environment only
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/lock_def.h>
#include <sys/atomic_op.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/prio_calc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/mstsave.h>
#include <sys/intr.h>
#include <sys/sleep.h>

#define RUNNING(owner) ((owner->t_state == TSRUN) && (owner->t_wtype == TNOWAIT))
#define COMPLEX_LOCK_CAN_SPIN(l,n) \
        (((((complex_lock_t)l)->_status & READ_MODE) || (((((complex_lock_t)l)->_status & OWNER_MASK) != 0) && RUNNING(THREADPTR(((complex_lock_t)l)->_status & OWNER_MASK)))) && (n<maxspin))

/*
 * NAME: complex_lock_sleep_ppc
 *	 complex_lock_sleep_instr_ppc
 *
 * FUNCTION: put a thread to sleep on a complex_lock
 *
 * Algorithm
 *	set INTERLOCK bit
 *	set WAITING bit
 *	release the kernel lock
 *	if the thread going to sleep has a priority higher than the lock's owner
 *		perform priority promotion
 *	put the thread on a hashed wait queue
 *	change thread state
 *	release INTERLOCK bit
 *	go to sleep
 *
 * RETURNS: no return 
 *
 * NOTES: This macro must be called disabled to INTMAX.
 */

int
#ifdef _INSTRUMENTATION
complex_lock_sleep_instr_ppc(complex_lock_t lockaddr,register int spincount,short wtype, boolean_t *didsleep)
#else
complex_lock_sleep_ppc(complex_lock_t lock,register int spincount,short wtype)
#endif
{
	register struct thread *myself = curthread; /* current thread pointer */
	register struct thread *ownt;	
	register int hadkernel_lock = 0;
        register tid_t owner;
        register priority_boosted = FALSE;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*lock;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);

	if ( instrumented ) {
		lock = lockaddr->_clockp;
	}
	else {
		lock = (struct lock_data_instrumented *)lockaddr;
	}
#endif


	ASSERT(csa->intpri == INTMAX);	
#ifdef _POWER_MP
	while (!test_and_set((atomic_p)lock,INTERLOCK));
#endif

        /*
         * After setting the interlock bit, if the WW bit is set, the OWNER_MASK == 0
         * and the wtype == TWLOCK, then the caller set the WW bit.  The lock is free, 
	 * and we MUST NOT go to sleep since there will be nobody to wake him up.
         */
        owner = ((complex_lock_t)lock)->_status & OWNER_MASK;

        if ((owner != 0) || ((((complex_lock_t)lock)->_status & WANT_WRITE) && (wtype == TWLOCKREAD))) {
		if (COMPLEX_LOCK_CAN_SPIN(((complex_lock_t)lock),spincount)) {	
#ifdef _POWER_MP
			fetch_and_and((atomic_p)lock,~INTERLOCK);
#endif
		}		
		else {	
                        fetch_and_or((atomic_p)lock,WAITING); /* set WAITING bit atomically */

			if (!hadkernel_lock) {		
				hadkernel_lock = IS_LOCKED(&kernel_lock);
				if (hadkernel_lock) unlockl(&kernel_lock);
			}
#ifdef _POWER_MP
			simple_lock(&proc_base_lock);
#endif /* _POWER_MP */

                        /* boost priority ONLY if owner is known */
                        owner = ((complex_lock_t)lock)->_status & OWNER_MASK;
                        if ((((complex_lock_t)lock)->_status & WANT_WRITE) && (owner != 0)) {
                		/*
                 		 * Since we are going to sleep on a lock, we could have
                 		 * a priority inversion problem.  This problem is 
				 * addressed by the dispatcher.  t_lockowner is a 
				 * parameter to the dispatcher.  
                 		 */
                		myself->t_lockowner = THREADPTR(owner);
                		ASSERT(myself->t_lockowner->t_tid == owner);
                        }
#ifdef _INSTRUMENTATION
			*didsleep = TRUE;

			/* store real wait channel */
			myself->t_wchan1 = (char *)lockaddr;
			myself->t_wchan1sid = SRTOSID(mfsri((uint)lockaddr));
			myself->t_wchan1offset = SEGOFFSET(lockaddr); 
			wait_on_lock(lockq_hash(lockaddr),FALSE);	
#else
			/* store real wait channel */	
			myself->t_wchan1 = (char *)lock;
			myself->t_wchan1sid = SRTOSID(mfsri((uint)lock));
			myself->t_wchan1offset = SEGOFFSET(lock);
			wait_on_lock(lockq_hash(lock),FALSE);	
#endif
			myself->t_wtype = wtype;

#ifdef _POWER_MP
			simple_unlock(&proc_base_lock);
#endif

			swtch();
			spincount = 0;	
		}	
	}
	else {
#ifdef _POWER_MP
		fetch_and_and((atomic_p)lock,~INTERLOCK);
#endif
	}
	return (hadkernel_lock);
}

/*
 * NAME: lock_write
 *
 * FUNCTION: lock a complex lock for write exclusive access
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_write_instr_ppc(complex_lock_t lockaddr)
#else
void
lock_write_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register boolean_t	want_write = FALSE;	/* WW bit acquired */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		spincount = 0;		/* counter of spinning cycles */
	register int		link_register;
	register int 		lname = 0;	 	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);
	register boolean_t	didmiss = FALSE;
	boolean_t		didsleep = FALSE;

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

	/* Check for recursion */
	if ((((complex_lock_t)l)->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		((complex_lock_t)l)->_recursion_depth += 1;
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
		return;
	}

retry:
	/* lock busy: held in write mode */
	while (lockword & WANT_WRITE) {
#ifdef _INSTRUMENTATION
		didmiss = TRUE;
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
		if (!COMPLEX_LOCK_CAN_SPIN(l,spincount)) {

			/* Need to disable before taking the INTERLOCK
			 * bit to ensure we don't get dispatched out while
			 * holding the INTERLOCK bit.
			 * 
			 * Also, touch the lockword immediately in case
			 * of page fault.
			 */
			
			ipri = i_disable(INTMAX);
			lockword = LOADWCS((int *)l);
#ifdef _INSTRUMENTATION
			hadkernel_lock |= complex_lock_sleep_instr_ppc(lockaddr,spincount,TWLOCKREAD, &didsleep);
#else
			hadkernel_lock |= complex_lock_sleep_ppc(l,spincount,TWLOCKREAD);
#endif
			i_enable(ipri);
		}
		spincount++;

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;
	}

	/* acquire write access */
	lockword |= WANT_WRITE;

	/* wait for readers to release the lock */
	if (lockword & READ_MODE) {
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;
	
			/* compare_and_swap failed */
			goto retry;
		}
#ifdef _INSTRUMENTATION
		didmiss = TRUE;
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
		spincount = 0;
		want_write = TRUE;

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;
retry2:
		while (lockword & READ_MODE) {
			if (!COMPLEX_LOCK_CAN_SPIN(l,spincount)) {
				
				/* Need to disable before taking the INTERLOCK
				 * bit to ensure we don't get dispatched out while
				 * holding the INTERLOCK bit.
				 * 
				 * Also, touch the lockword immediately in case
				 * of page fault.
				 */
				
				ipri = i_disable(INTMAX);
				lockword = LOADWCS((int *)l);
#ifdef _INSTRUMENTATION
				hadkernel_lock |= complex_lock_sleep_instr_ppc(lockaddr,spincount,TWLOCK,&didsleep);
#else
				hadkernel_lock |= complex_lock_sleep_ppc(l,spincount,TWLOCK);
#endif
				i_enable(ipri);
			}
			spincount++;

			/* Re-read lock word in a volatile fashion */
			lockword = old_value = ((volatile Complex_lock *)l)->_status;
			ASSERT(lockword & WANT_WRITE);
		}
	}

	/* record id */
	lockword |= id;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;
	
		/* compare_and_swap failed */
		if (want_write == TRUE) {
			goto retry2;
		} else {
			goto retry;
		}
	}
#ifdef _POWER_MP
	isync_601();
#endif

#ifdef _INSTRUMENTATION
	if ( instrumented ) {
		fetch_and_add((atomic_p)&(l->acquisitions), 1);
		if (didmiss) {
			fetch_and_add((atomic_p)&(l->misses), 1);
		}
		if (didsleep) {
			fetch_and_add((atomic_p)&(l->sleeps), 1);
		}
	}
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif

	myself->t_lockcount++;
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
}

/*
 * NAME: lock_read
 *
 * FUNCTION: lock a complex lock for read access
 *
 * NOTE: if the lock is held in read mode and some threads are waiting to acquire
 *	 the lock in write mode, a new reader will be allowed only if its priority
 *	 is higher than the priority of the highest priority writer waiting
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_read_instr_ppc(complex_lock_t lockaddr)
#else
void
lock_read_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		spincount = 0;		/* counter of spinning cycles */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);
	register boolean_t	didmiss = FALSE;
	boolean_t		didsleep = FALSE;

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

	/* Check for recursion */
	if ((((complex_lock_t)l)->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		((complex_lock_t)l)->_recursion_depth += 1;
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif
		return;
	}

retry:
	while (lockword & WANT_WRITE) {
#ifdef _INSTRUMENTATION
		didmiss = TRUE;
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif

		/* check for deadlock */
		ASSERT((lockword & READ_MODE) || ((lockword & OWNER_MASK) != id));

		if (!COMPLEX_LOCK_CAN_SPIN(l,spincount)) {
			
			/* Need to disable before taking the INTERLOCK
			 * bit to ensure we don't get dispatched out while
			 * holding the INTERLOCK bit.
			 * 
			 * Also, touch the lockword immediately in case
			 * of page fault.
			 */
			
			ipri = i_disable(INTMAX);
			lockword = LOADWCS((int *)l);
#ifdef _INSTRUMENTATION
			hadkernel_lock |= complex_lock_sleep_instr_ppc(lockaddr,spincount,TWLOCKREAD,&didsleep);
#else
			hadkernel_lock |= complex_lock_sleep_ppc(l,spincount,TWLOCKREAD);
#endif
			i_enable(ipri);
		}
		spincount++;

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;
	}

	lockword++;		/* increment read_count */
	lockword |= READ_MODE;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}
#ifdef _INSTRUMENTATION
	if ( instrumented ) {
		fetch_and_add((atomic_p)&(l->acquisitions), 1);
		if (didmiss) {
			fetch_and_add((atomic_p)&(l->misses), 1);
		}
		if (didsleep) {
			fetch_and_add((atomic_p)&(l->sleeps), 1);
		}
	}
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif

	myself->t_lockcount++;
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
}

/*
 * NAME: lock_done
 *
 * FUNCTION: release a complex lock
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_done_instr_ppc(complex_lock_t lockaddr)
#else
void
lock_done_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register boolean_t	waiting = FALSE;	/* more than one thread waiting */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */

	if ( lockaddr->_status & INSTR_ON ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	ASSERT ((((complex_lock_t)l)->_status & OWNER_MASK) != 0);

#ifdef _POWER_MP
	/* If lock is being released from write_mode, we need to sync */
	if (!(((complex_lock_t)l)->_status & READ_MODE) && (((complex_lock_t)l)->_status & WANT_WRITE))
		__iospace_sync();
#endif

#ifdef _INSTRUMENTATION
	TRCHKL4T(HKWD_KERN_UNLOCK,lockaddr,((complex_lock_t)l)->_status,link_register,lname);
#else
	TRCHKL4T(HKWD_KERN_UNLOCK,l,l->_status,link_register,lname);
#endif
	if (((complex_lock_t)l)->_recursion_depth != 0) {
		ASSERT(((complex_lock_t)l)->_status & WANT_WRITE);
		ASSERT((((complex_lock_t)l)->_status & OWNER_MASK) == myself->t_tid);
		((complex_lock_t)l)->_recursion_depth -= 1;
		return;
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

#ifdef _POWER_MP
	while (!test_and_set((atomic_p)l,INTERLOCK));
#endif

	/* Re-read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

retry:
	if (lockword & READ_MODE) {
		lockword--;
		if ((lockword & READ_COUNT_MASK) == 0) {
			lockword &= ~READ_MODE;
			if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
				simple_lock(&proc_base_lock);
				simple_lock(&proc_int_lock);
#endif
#ifdef _INSTRUMENTATION
				waiting = wakeup_lock(lockq_hash(lockaddr), (void *)lockaddr,
						      WAKEUP_LOCK_WRITER);
#else
				waiting = wakeup_lock(lockq_hash(l), (void *)l,
						      WAKEUP_LOCK_WRITER);
#endif
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
				simple_unlock(&proc_base_lock);
#endif
				wakeup = FALSE;
			}
			if (!waiting)
				lockword &= ~WAITING;
		}
	}
	else {
		ASSERT((lockword & OWNER_MASK) == myself->t_tid);

		/* This assert is for debug purposes only.
		 * Freeing the lock will reset the RECURSIVE bit
		 */
		ASSERT(!(((complex_lock_t)l)->_flags & RECURSIVE));
		((complex_lock_t)l)->_flags &= ~RECURSIVE;

		/* reset WANT_WRITE bit and OWNER_MASK */
		lockword &= (~WANT_WRITE & ~OWNER_MASK);

		if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
			simple_lock(&proc_base_lock);
			simple_lock(&proc_int_lock);
#endif
#ifdef _INSTRUMENTATION
			waiting = wakeup_lock(lockq_hash(lockaddr), (void *)lockaddr,
					      WAKEUP_LOCK_ALL);
#else
			waiting = wakeup_lock(lockq_hash(l), (void *)l,
					      WAKEUP_LOCK_ALL);
#endif
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
			simple_unlock(&proc_base_lock);
#endif
			wakeup = FALSE;
		}
		if (!waiting) {
			lockword &= ~WAITING;
		}
	}

#ifdef _POWER_MP
	lockword &= ~INTERLOCK;
	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}
#else
	/* compare_and_swap() only on REAL MPs */
	ASSERT(((complex_lock_t)l)->_status == old_value);
	((complex_lock_t)l)->_status = lockword;
#endif
	i_enable(ipri);

	myself->t_lockcount--;
}

/*
 * NAME: lock_read_to_write
 *
 * FUNCTION: Upgrade a read-only lock to one with write permission.
 *
 * NOTE: if another reader has already requested an upgrade to a write lock,
 *	 no lock is held upon return
 *
 * RETURNS: TRUE if the upgrade FAILED
 *	    FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_read_to_write_instr_ppc(complex_lock_t lockaddr)
#else
boolean_t
lock_read_to_write_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register boolean_t	want_write = FALSE;	/* WW bit acquired */
	register boolean_t	waiting = FALSE;	/* thread still sleeping */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		spincount = 0;		/* counter of spinning cycles */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);
	register boolean_t	didmiss = FALSE;
	boolean_t		didsleep = FALSE;

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	if (((complex_lock_t)l)->_flags & RECURSIVE) {
		ASSERT(((complex_lock_t)l)->_status & WANT_WRITE);
		ASSERT((((complex_lock_t)l)->_status & OWNER_MASK) == id);

#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
		return(FALSE);
	}

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;
	
retry:
	ASSERT (lockword & READ_MODE); 
	
	if (lockword & WANT_WRITE) {
		/* Failure path:  lock is already been upgraded for write
		 * need to handle the sleepers in case we're the last to
		 * unlock this lock (releasing read mode)
		 *
		 * we'll be returning TRUE (upgrade failed) below
		 */
		
		/* Need to disable before taking the INTERLOCK
		 * bit to ensure we don't get dispatched out while
		 * holding the INTERLOCK bit.
		 * 
		 * Also, touch the lockword immediately in case
		 * of page fault.
		 */
		
		ipri = i_disable(INTMAX);
		lockword = LOADWCS((int *)l);
		
		/* Read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;

#ifdef _POWER_MP
		if (lockword & INTERLOCK) {
			/* Re-read lock word in a volatile fashion */
			lockword = old_value = ((volatile Complex_lock *)l)->_status;
			i_enable(ipri);
			goto retry;
		}
		lockword |= INTERLOCK;
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			i_enable(ipri);
			goto retry;
		}

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;
#endif
		
retry_with_interlock:
		lockword--;	/* decrement read count */
		if ((lockword & READ_COUNT_MASK) == 0) {
			lockword &= ~READ_MODE;
			
			if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
				simple_lock(&proc_base_lock);
				simple_lock(&proc_int_lock);
#endif
#ifdef _INSTRUMENTATION
				waiting = wakeup_lock(lockq_hash(lockaddr), (void *)lockaddr,
						      WAKEUP_LOCK_WRITER);
#else
				waiting = wakeup_lock(lockq_hash(l), (void *)l,
						      WAKEUP_LOCK_WRITER);
#endif
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
				simple_unlock(&proc_base_lock);
#endif
				wakeup = FALSE;
			}
#ifdef _INSTRUMENTATION
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
			
			if (!waiting)
				lockword &= ~WAITING;
		}
#ifdef _POWER_MP
		lockword &= ~INTERLOCK;
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			goto retry_with_interlock;
		}
#else
		/* compare_and_swap only on REAL MPs */
		ASSERT(((complex_lock_t)l)->_status == old_value);
		((complex_lock_t)l)->_status = lockword;
#endif
		i_enable(ipri);

		myself->t_lockcount--;

		return (TRUE);
	}

	lockword--;	/* decrement read count */
	if ((lockword & READ_COUNT_MASK) == 0) {
		lockword &= ~READ_MODE;
	}
	lockword |= WANT_WRITE;

	if (lockword & READ_MODE) {
#ifdef _POWER_MP
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			goto retry;
		}
#else
		/* compare_and_swap() only on REAL MPs */
		ASSERT(((complex_lock_t)l)->_status == old_value);
		((complex_lock_t)l)->_status = lockword;
#endif
#ifdef _INSTRUMENTATION
		didmiss = TRUE;
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
		want_write = TRUE;

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;

retry2:
		while (lockword & READ_MODE) {
			if (!COMPLEX_LOCK_CAN_SPIN(l,spincount)) {

				/* Need to disable before taking the INTERLOCK
				 * bit to ensure we don't get dispatched out while
				 * holding the INTERLOCK bit.
				 * 
				 * Also, touch the lockword immediately in case
				 * of page fault.
				 */
				
				ipri = i_disable(INTMAX);
				lockword = LOADWCS((int *)l);
#ifdef _INSTRUMENTATION
				hadkernel_lock |= complex_lock_sleep_instr_ppc(lockaddr,spincount,TWLOCK, &didsleep);
#else
				hadkernel_lock |= complex_lock_sleep_ppc(l,spincount,TWLOCK);
#endif
				i_enable(ipri);
			}
			spincount++;

			/* Re-read lock word in a volatile fashion */
			lockword = old_value = ((volatile Complex_lock *)l)->_status;
			ASSERT(lockword & WANT_WRITE);
		}
	}

	lockword |= id;

#ifdef _POWER_MP
	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		if (want_write) {
			goto retry2;
		} else {
			goto retry;
		}
	}
#else
	/* compare_and_swap() only on REAL MPs */
	ASSERT(((complex_lock_t)l)->_status == old_value);
	((complex_lock_t)l)->_status = lockword;
#endif

#ifdef _INSTRUMENTATION
	if ( instrumented ) {
		if (didmiss) {
			fetch_and_add((atomic_p)&(l->misses), 1);
		}
		if (didsleep) {
			fetch_and_add((atomic_p)&(l->sleeps), 1);
		}
	}
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
	return(FALSE);
}

/*
 * NAME: lock_write_to_read
 *
 * FUNCTION: decrease complex_lock's owner privilege from write only
 *	     to read shared access
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_write_to_read_instr_ppc(complex_lock_t lockaddr)
#else
void
lock_write_to_read_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register boolean_t	waiting = FALSE;	/* more than one thread waiting */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		id;			/* current thread's id */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */

	if ( lockaddr->_status & INSTR_ON ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	ASSERT(!(((complex_lock_t)l)->_status & READ_MODE) &&
	       (((complex_lock_t)l)->_status & WANT_WRITE) && ((((complex_lock_t)l)->_status & OWNER_MASK) == id));

	if (((complex_lock_t)l)->_flags & RECURSIVE) {
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_DOWNGRADE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_DOWNGRADE_TRACE,link_register,lname);
#endif
		return;
	}

#ifdef _POWER_MP
	__iospace_sync();
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
		
#ifdef _POWER_MP
	while (!test_and_set((atomic_p)l,INTERLOCK));
#endif

	/* Re-read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;
	
retry:
	/* reset WANT_WRITE bit and OWNER_MASK */
	lockword &= (~WANT_WRITE & ~OWNER_MASK);
	lockword |= ONE_READER;		/* set read_count to 1 */

	if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
		simple_lock(&proc_int_lock);
#endif
#ifdef _INSTRUMENTATION
		waiting = wakeup_lock(lockq_hash(lockaddr), (void *)lockaddr,
				      WAKEUP_LOCK_ALL);
#else
		waiting = wakeup_lock(lockq_hash(l), (void *)l,
				      WAKEUP_LOCK_ALL);
#endif
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
		simple_unlock(&proc_base_lock);
#endif
		wakeup = FALSE;
	}
	if (!waiting)
		lockword &= ~WAITING;
#ifdef _POWER_MP
	lockword &= ~INTERLOCK;
	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}
#else
	/* compare_and_swap() only on REAL MPs */
	ASSERT(((complex_lock_t)l)->_status == old_value);
	((complex_lock_t)l)->_status = lockword;
#endif

	i_enable(ipri);

#ifdef _INSTRUMENTATION
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_DOWNGRADE_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_DOWNGRADE_TRACE,link_register,lname);
#endif
}

/*
 * NAME: lock_try_write
 *
 * FUNCTION: try to acquire a complex lock for write only access
 *
 * RETURNS: TRUE if the lock is acquired
 *	    FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_try_write_instr_ppc(complex_lock_t lockaddr)
#else
boolean_t
lock_try_write_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

	/* Check for recursion */
	if ((((complex_lock_t)l)->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		((complex_lock_t)l)->_recursion_depth += 1;
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
		return(TRUE);
	}

retry:
	if (lockword & WANT_WRITE) {
#ifdef _INSTRUMENTATION
		if ( instrumented ) 
			fetch_and_add((atomic_p)&(l->misses), 1);
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
		return(FALSE);
	}
	else if (!(lockword & READ_MODE)) {
		lockword |= (WANT_WRITE | id);
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			goto retry;
		} else {
#ifdef _INSTRUMENTATION
			if ( instrumented ) 
				fetch_and_add((atomic_p)&(l->acquisitions), 1);
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
			myself->t_lockcount++;
			return(TRUE);
		}
	}
	
#ifdef _INSTRUMENTATION
	if ( instrumented ) 
		fetch_and_add((atomic_p)&(l->misses), 1);
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lockaddr,((complex_lock_t)l)->_status,LOCK_CWRITE_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,l,l->_status,LOCK_CWRITE_TRACE,link_register,lname);
#endif
	return(FALSE);
	
}

/*
 * NAME: lock_try_read
 *
 * FUNCTION: try to acquire a complex lock for read shared access
 *
 * RETURNS: TRUE if the lock is acquired
 *	    FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_try_read_instr_ppc(complex_lock_t lockaddr)
#else
boolean_t
lock_try_read_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

	/* Check for recursion */
	if ((((complex_lock_t)l)->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		((complex_lock_t)l)->_recursion_depth += 1;
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif
		return(TRUE);
	}

retry:
	if (lockword & WANT_WRITE) {
#ifdef _INSTRUMENTATION
		if ( instrumented ) 
			fetch_and_add((atomic_p)&(l->misses), 1);
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif
		return(FALSE);
	}
	lockword++;
	lockword |= READ_MODE;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}

#ifdef _INSTRUMENTATION
	if ( instrumented ) 
		fetch_and_add((atomic_p)&(l->acquisitions), 1);
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_READ_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_READ_TRACE,link_register,lname);
#endif

	myself->t_lockcount++;
	return(TRUE);
}

/*
 * NAME: lock_try_read_to_write
 *
 * FUNCTION: try to upgrade a read-only lock to one with write permission
 *
 * NOTE: If another reader has already requested an upgrade to a write lock,
 *	 the read lock is still held upon return
 *
 * RETURNS: TRUE if the lock is acquired
 *	    FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_try_read_to_write_instr_ppc(complex_lock_t lockaddr)
#else
boolean_t
lock_try_read_to_write_ppc(complex_lock_t l)
#endif

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register boolean_t	want_write = FALSE;	/* WW bit acquired */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		spincount = 0;		/* counter of spinning cycles */
	register int		link_register;
	register int 		lname = NULL;	/* lock symbolic name */
	register int		ipri;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */
	register int 		instrumented = (lockaddr->_status & INSTR_ON);
	register boolean_t	didmiss = FALSE;
	boolean_t		didsleep = FALSE;

	if ( instrumented ) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else 
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = ((volatile Complex_lock *)l)->_status;

	/* Check recursion */
	if (((complex_lock_t)l)->_flags & RECURSIVE) {
		ASSERT(lockword & WANT_WRITE);
		ASSERT((lockword & OWNER_MASK) == id);
		
#ifdef _INSTRUMENTATION
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
		return(TRUE);
	}
	
retry:
	ASSERT (lockword & READ_MODE); 

	if (lockword & WANT_WRITE) {
#ifdef _INSTRUMENTATION
		if ( instrumented ) 
			fetch_and_add((atomic_p)&(l->misses), 1);
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
		TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
		return (FALSE);
	}
	else if (((--lockword) & READ_COUNT_MASK) == 0) {	/* decrement read_count */
		lockword &= ~READ_MODE;
	}

	lockword |= WANT_WRITE;

	if (lockword & READ_MODE) {
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			goto retry;
		}
#ifdef _INSTRUMENTATION
		didmiss = TRUE;
#endif
		want_write = TRUE;

		/* Re-read lock word in a volatile fashion */
		lockword = old_value = ((volatile Complex_lock *)l)->_status;

retry2:
		while (lockword & READ_MODE) {
#ifdef _INSTRUMENTATION
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
			TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
			if (!COMPLEX_LOCK_CAN_SPIN(l,spincount)) {

				/* Need to disable before taking the INTERLOCK
				 * bit to ensure we don't get dispatched out while
				 * holding the INTERLOCK bit.
				 * 
				 * Also, touch the lockword immediately in case
				 * of page fault.
				 */
				
				ipri = i_disable(INTMAX);
				lockword = LOADWCS((int *)l);
#ifdef _INSTRUMENTATION
				hadkernel_lock |= complex_lock_sleep_instr_ppc(lockaddr,spincount,TWLOCK,&didsleep);
#else
				hadkernel_lock |= complex_lock_sleep_ppc(l,spincount,TWLOCK);
#endif
				i_enable(ipri);
			}
			spincount++;

			/* Re-read lock word in a volatile fashion */
			lockword = old_value = ((volatile Complex_lock *)l)->_status;
		}
	}

	lockword |= id;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		if (want_write == TRUE) {
			goto retry2;
		} else {
			goto retry;
		}
	}
#ifdef _INSTRUMENTATION
	if ( instrumented ) {
		if (didmiss) { 
			fetch_and_add((atomic_p)&(l->misses), 1);
		}
		if (didsleep) {
			fetch_and_add((atomic_p)&(l->sleeps), 1);
		}
	}
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lockaddr,((complex_lock_t)l)->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#else
	TRCHKL5T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_UPGRADE_TRACE,link_register,lname);
#endif
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
	return(TRUE);

}
