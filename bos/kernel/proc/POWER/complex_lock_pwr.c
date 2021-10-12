static char sccsid[] = "@(#)76	1.18  src/bos/kernel/proc/POWER/complex_lock_pwr.c, sysproc, bos41J, 9516A_all 4/13/95 15:10:03";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_done_pwr
 *		lock_read_pwr
 *		lock_read_to_write_pwr
 *		complex_lock_sleep_pwr
 *		lock_try_read_pwr
 *		lock_try_read_to_write_pwr
 *		lock_try_write_pwr
 *		lock_write_pwr
 *		lock_write_to_read_pwr
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

/*
 * NAME: complex_lock_sleep_pwr
 *
 * FUNCTION: put a thread to sleep on a complex_lock
 *
 * Algorithm
 *	set WAITING bit
 *	release the kernel lock
 *	if the thread going to sleep has a priority higher than the lock's owner
 *		perform priority promotion
 *	put the thread on a hashed wait queue
 *	change thread state
 *	go to sleep
 *
 * RETURNS: no return 
 *
 * NOTES: This macro must be called disabled to INTMAX.
 */

int
complex_lock_sleep_pwr(complex_lock_t lock,short wtype)			
{										  
	register struct thread *myself = curthread;	
	register struct thread *ownt;		 
	register int hadkernel_lock = 0;
        register tid_t owner;
        register priority_boosted = FALSE;

	ASSERT(csa->intpri == INTMAX);					  

        /*
         * After disabling, if the WW bit is set, the OWNER_MASK == 0,
         * and the wtype == TWLOCK, then the caller set the WW bit,
         * the lock is free, and we MUST NOT go to sleep since there
         * will be nobody to wake him up
         */

        if (((lock->_status & OWNER_MASK) != 0) ||
            ((lock->_status & WANT_WRITE) && (wtype == TWLOCKREAD))) {

		lock->_status |= WAITING; /* set WAITING bit in the lockword */	
		if (!hadkernel_lock) {					 
			hadkernel_lock = IS_LOCKED(&kernel_lock);	  
			if (hadkernel_lock) unlockl(&kernel_lock);	  
		}	
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
#endif /* _POWER_MP */

                /* boost priority ONLY if owner is known */
                owner = lock->_status & OWNER_MASK;
                if ((lock->_status & WANT_WRITE) && (owner != 0)) {
                	/*
                 	 * Since we are going to sleep on a lock, we could have
                 	 * a priority inversion problem.  This problem is addressed
                 	 * by the dispatcher.  t_lockowner is a parameter to the
                 	 * dispatcher.  
                 	 */
                	myself->t_lockowner = THREADPTR(owner);
                	ASSERT(myself->t_lockowner->t_tid == owner);
                }

		/* store real wait channel */	
		myself->t_wchan1 = (char *)lock;
		myself->t_wchan1sid = SRTOSID(mfsri((uint)lock));
		myself->t_wchan1offset = SEGOFFSET(lock);

		wait_on_lock(lockq_hash(lock),FALSE);			  

		myself->t_wtype = wtype;				  
#ifdef _POWER_MP
		simple_unlock(&proc_base_lock);
#endif
		swtch();						  
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

void
lock_write_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value	  */
	int			old_value;		/* old lockword value	  */
	register int		id;			/* current thread's id	  */
	register boolean_t	want_write = FALSE;	/* WW bit acquired	  */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	/* Check for recursion */
	if ((l->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		l->_recursion_depth += 1;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_CWRITE_TRACE,link_register);
		return;
	}

retry:
	/* lock busy: held in write mode */
	while (lockword & WANT_WRITE) {
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register);

		ipri = i_disable(INTMAX);
		lockword = LOADWCS((int *)l);
		hadkernel_lock |= complex_lock_sleep_pwr(l,TWLOCKREAD);
		i_enable(ipri);
		
		/* Read lock word in a volatile fashion */
		lockword = old_value = l->_status;
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
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register);
		want_write = TRUE;

		/* Read lock word in a volatile fashion */
		lockword = old_value = l->_status;
retry2:
		while (lockword & READ_MODE) {
			
			ipri = i_disable(INTMAX);
			lockword = LOADWCS((int *)l);
			hadkernel_lock |= complex_lock_sleep_pwr(l,TWLOCK);
			i_enable(ipri);
		
			/* Read lock word in a volatile fashion */
			lockword = old_value = l->_status;
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

	TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_CWRITE_TRACE,link_register);

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

void
lock_read_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value	  */
	int			old_value;		/* old lockword value	  */
	register int		id;			/* current thread's id	  */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	/* Check for recursion */
	if ((l->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		l->_recursion_depth += 1;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_READ_TRACE,link_register);
		return;
	}

retry:
	while (lockword & WANT_WRITE) {
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_READ_TRACE,link_register);

		/* check for deadlock */
		ASSERT((lockword & READ_MODE) || ((lockword & OWNER_MASK) != id));

		ipri = i_disable(INTMAX);
		lockword = LOADWCS((int *)l);
		hadkernel_lock |= complex_lock_sleep_pwr(l,TWLOCKREAD);
		i_enable(ipri);
		
		/* Read lock word in a volatile fashion */
		lockword = old_value = l->_status;
	}

	lockword++;		/* increment read_count */
	lockword |= READ_MODE;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}

	TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_READ_TRACE,link_register);

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

void
lock_done_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register boolean_t	waiting = FALSE;	/* more than one thread waiting */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	ASSERT ((l->_status & OWNER_MASK) != 0);

	TRCHKL3T(HKWD_KERN_UNLOCK,l,l->_status,link_register);
	if (l->_recursion_depth != 0) {
		ASSERT(l->_status & WANT_WRITE);
		ASSERT((l->_status & OWNER_MASK) == myself->t_tid);
		l->_recursion_depth -= 1;
		return;
	}

	ipri = i_disable(INTMAX);
	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	if (lockword & READ_MODE) {
		lockword--;
		if ((lockword & READ_COUNT_MASK) == 0) {
			lockword &= ~READ_MODE;
			if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
				simple_lock(&proc_base_lock);
				simple_lock(&proc_int_lock);
#endif
				waiting = wakeup_lock(lockq_hash(l), (void *)l,
						      WAKEUP_LOCK_WRITER); 
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
		l->_flags &= ~RECURSIVE;

		/* reset WANT_WRITE bit and OWNER_MASK */
		lockword &= (~WANT_WRITE & ~OWNER_MASK);

		if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
			simple_lock(&proc_base_lock);
			simple_lock(&proc_int_lock);
#endif
			waiting = wakeup_lock(lockq_hash(l), (void *)l,
					      WAKEUP_LOCK_ALL);
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

	/* compare_and_swap() only on REAL MPs */
	ASSERT(l->_status == old_value);
	l->_status = lockword;
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

boolean_t
lock_read_to_write_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register int		id;			/* current thread's id */
	register boolean_t	want_write = FALSE;	/* WW bit acquired */
	register boolean_t	waiting = FALSE;	/* thread still sleeping */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	if (l->_flags & RECURSIVE) { 
		ASSERT(l->_status & WANT_WRITE);
		ASSERT((l->_status & OWNER_MASK) == id);

		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
		return(FALSE);
	}

	ipri = i_disable(INTMAX);
	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	ASSERT (lockword & READ_MODE); 

	if (lockword & WANT_WRITE) {
		lockword--;	/* decrement read count */
		if ((lockword & READ_COUNT_MASK) == 0) {
			lockword &= ~READ_MODE;
			
			if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
				simple_lock(&proc_base_lock);
				simple_lock(&proc_int_lock);
#endif
				waiting = wakeup_lock(lockq_hash(l), (void *)l,
						      WAKEUP_LOCK_WRITER);
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
				simple_unlock(&proc_base_lock);
#endif
				wakeup = FALSE;
			}
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
			
			if (!waiting)
				lockword &= ~WAITING;
		}
		/* compare_and_swap only on REAL MPs */
		ASSERT(l->_status == old_value);
		l->_status = lockword;
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
		/* compare_and_swap() only on REAL MPs */
		ASSERT(l->_status == old_value);
		l->_status = lockword;

		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
		want_write = TRUE;

		/* Read lock word in a volatile fashion */
		lockword = old_value = l->_status;

		while (lockword & READ_MODE) {

			hadkernel_lock |= complex_lock_sleep_pwr(l,TWLOCK);

			/* Read lock word in a volatile fashion */
			lockword = old_value = l->_status;
			ASSERT(lockword & WANT_WRITE);
		}
	}

	lockword |= id;

	/* compare_and_swap() only on REAL MPs */
	ASSERT(l->_status == old_value);
	l->_status = lockword;
	i_enable(ipri);
		
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
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

void
lock_write_to_read_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value */
	int			old_value;		/* old lockword value */
	register boolean_t	waiting = FALSE;	/* more than one thread waiting */
	register boolean_t	wakeup = TRUE;		/* call wakeup_lock()? */
	register int		id;			/* current thread's id */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	ASSERT(!(l->_status & READ_MODE) && 
	       (l->_status & WANT_WRITE) && ((l->_status & OWNER_MASK) == id));

	if (l->_flags & RECURSIVE) {
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_DOWNGRADE_TRACE,link_register,0);
		return;
	}
	
	ipri = i_disable(INTMAX);
	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;
	
	/* reset WANT_WRITE bit and OWNER_MASK */
	lockword &= (~WANT_WRITE & ~OWNER_MASK);
	lockword |= ONE_READER;		/* set read_count to 1 */

	if ((lockword & WAITING) && wakeup) {
#ifdef _POWER_MP
		simple_lock(&proc_base_lock);
		simple_lock(&proc_int_lock);
#endif
		waiting = wakeup_lock(lockq_hash(l), (void *)l,
				      WAKEUP_LOCK_ALL);
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
		simple_unlock(&proc_base_lock);
#endif
		wakeup = FALSE;
	}
	if (!waiting)
		lockword &= ~WAITING;

	/* compare_and_swap() only on REAL MPs */
	ASSERT(l->_status == old_value);
	l->_status = lockword;
	i_enable(ipri);
		
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_DOWNGRADE_TRACE,link_register,0);
}

/*
 * NAME: lock_try_write
 *
 * FUNCTION: try to acquire a complex lock for write only access
 *
 * RETURNS: TRUE if the lock is acquired
 *	    FALSE otherwise
 */

boolean_t
lock_try_write_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value	  */
	int			old_value;		/* old lockword value	  */
	register int		id;			/* current thread's id	  */
	register int		link_register;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	/* Check for recursion */
	if ((l->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		l->_recursion_depth += 1;
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_CWRITE_TRACE,link_register,0);
		return(TRUE);
	}

retry:
	if (lockword & WANT_WRITE) {
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register,0);
		return(FALSE);
	}
	else if (!(lockword & READ_MODE)) {
		lockword |= (WANT_WRITE | id);
		if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
			lockword = old_value;

			/* compare_and_swap failed */
			goto retry;
		} else {
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_CWRITE_TRACE,link_register,0);
			myself->t_lockcount++;
			return(TRUE);
		}
	}
	
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_CWRITE_TRACE,link_register,0);
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

boolean_t
lock_try_read_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value	  */
	int			old_value;		/* old lockword value	  */
	register int		id;			/* current thread's id	  */
	register int		link_register;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	/* Check for recursion */
	if ((l->_flags & RECURSIVE) && ((lockword & OWNER_MASK) == id)) {
		ASSERT(lockword & WANT_WRITE);

		l->_recursion_depth += 1;
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_READ_TRACE,link_register,0);
		return(TRUE);
	}

retry:
	if (lockword & WANT_WRITE) {
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_READ_TRACE,link_register,0);
		return(FALSE);
	}
	lockword++;
	lockword |= READ_MODE;

	if (!compare_and_swap((atomic_p)l,&old_value,lockword)) {
		lockword = old_value;

		/* compare_and_swap failed */
		goto retry;
	}

	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_READ_TRACE,link_register,0);

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

boolean_t
lock_try_read_to_write_pwr(register complex_lock_t l)

{
	register struct thread	*myself = curthread;	/* current thread pointer */
	register int		lockword;		/* lockword value	  */
	int			old_value;		/* old lockword value	  */
	register int		id;			/* current thread's id	  */
	boolean_t		want_write = FALSE;	/* WW bit acquired	  */
	register int		hadkernel_lock = 0;	/* this process held the kernel lock */
	register int		link_register;
	register int		ipri;

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	id = myself->t_tid;

	/* Read lock word in a volatile fashion */
	lockword = old_value = l->_status;

	/* Check for recursion */
	if (l->_flags & RECURSIVE) {
		ASSERT(lockword & WANT_WRITE);
		ASSERT((lockword & OWNER_MASK) == id);

		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
		return(TRUE);
	}

retry:
	ASSERT (lockword & READ_MODE); 

	if (lockword & WANT_WRITE) {
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
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
		want_write = TRUE;

		/* Read lock word in a volatile fashion */
		lockword = old_value = l->_status;

retry2:
		while (lockword & READ_MODE) {
			TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_MISS,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);

			ipri = i_disable(INTMAX);
			lockword = LOADWCS((int *)l);
			hadkernel_lock |= complex_lock_sleep_pwr(l,TWLOCK);
			i_enable(ipri);
		
			/* Read lock word in a volatile fashion */
			lockword = old_value = l->_status;
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
	if (hadkernel_lock) lockl(&kernel_lock,LOCK_SHORT);
	TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,l,l->_status,LOCK_UPGRADE_TRACE,link_register,0);
	return(TRUE);
}
