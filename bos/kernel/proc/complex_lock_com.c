static char sccsid[] = "@(#)81	1.5  src/bos/kernel/proc/complex_lock_com.c, sysproc, bos411, 9428A410j 4/5/94 13:18:28";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_clear_recursive
 *		lock_clear_recursive_instr
 *		lock_init
 *		lock_init_instr
 *		lock_islocked
 *		lock_islocked_instr
 *		lock_set_recursive
 *		lock_set_recursive_instr
 *
 *   ORIGINS: 27, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * NOTES:
 *      There is no functional difference between the POWER and the
 *      POWERPC version of these functions.  These functions will only be
 *      defined here for both POWER and POWERPC (no extension or branch
 *      table is necessary other than for INSTRUMENTATION).
 *
 *      Read shared/Write exclusive lock.
 *      For layout of lock status word look at sys/lock_def.h
 *      The lock word can be pageable, but must be in ordinary working
 *      storage.  It must not be in a persistent segment with journalling
 *      applied.  This is because the deadlock signal is not handled by
 *      these service routines.  Paging errors on the lock word cause
 *      the kernel to panic.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      These functions are pinned and will only fault if called on a
 *      pageable stack or passed a pageable lock word.
 *      Called from process environment only
 */

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/atomic_op.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/prio_calc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/mstsave.h>
#include <sys/sleep.h>

#ifdef _INSTRUMENTATION
#ifdef DEBUG
extern int lock_instr_dbg_wanted;
#endif
#endif

/*
 * NAME: lock_init
 *
 * FUNCTION: initialize complex lock
 *
 * RETURNS: no return
 *
 * NOTES:
 */

#ifdef _INSTRUMENTATION
void
lock_init_instr(complex_lock_t lockaddr, boolean_t can_sleep)
#else
void
lock_init(complex_lock_t l, boolean_t can_sleep)
#endif

{
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;

	if (lockaddr->_status & INSTR_ON) {
		l = lockaddr->_clockp;
		fetch_and_add(&family_lock_statistics[l->lock_id].acquisitions, l->acquisitions);
		fetch_and_add(&family_lock_statistics[l->lock_id].misses, l->misses);
		fetch_and_add(&family_lock_statistics[l->lock_id].sleeps, l->sleeps);

		l->acquisitions = 0;
		l->misses = 0;
		l->sleeps = 0;
	}
	else {
#ifdef DEBUG
		if (lock_instr_dbg_wanted) {
			printf("Warning:  complex_lock not allocated\n");
			brkpoint();
		}
#endif
		l = (struct lock_data_instrumented *)lockaddr;
	}
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	((complex_lock_t)l)->_status = COMPLEX_LOCK_AVAIL;
	((complex_lock_t)l)->_flags = 0;
	((complex_lock_t)l)->_recursion_depth = 0;
}

/*
 * NAME: lock_set_recursive
 *
 * FUNCTION: set the RECURSIVE bit in the lock status word
 *
 * NOTE: Recursion is meaningful only when accessing the lock
 *       with write permission
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_set_recursive_instr(complex_lock_t lockaddr)
#else
void
lock_set_recursive(complex_lock_t l)
#endif

{
        register struct thread  *myself = curthread;    /* current thread pointer */
        register int            lockword;               /* lockword value */
	register int		link_register;
	register int		lname = NULL;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */

	if (lockaddr->_status & INSTR_ON) {
		l = lockaddr->_clockp;
		lname = l->lockname;
	}
	else
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	/* Read lock word in a volatile fashion */
	lockword = ((complex_lock_t)l)->_status;

	ASSERT((lockword & WANT_WRITE) && ((lockword & OWNER_MASK) == myself->t_tid));

	((complex_lock_t)l)->_flags |= RECURSIVE;

#ifdef _INSTRUMENTATION
	TRCHKL3T(HKWD_KERN_SETRECURSIVE|hkwd_SETRECURSIVE,lockaddr,link_register,lname);
#else
	TRCHKL3T(HKWD_KERN_SETRECURSIVE|hkwd_SETRECURSIVE,l,link_register,lname);
#endif
}


/*
 * NAME: lock_clear_recursive
 *
 * FUNCTION: clear the RECURSIVE bit in the lock status word
 *
 * NOTES:
 *
 * RETURNS: no return
 */

#ifdef _INSTRUMENTATION
void
lock_clear_recursive_instr(complex_lock_t lockaddr)
#else
void
lock_clear_recursive(complex_lock_t l)
#endif

{
        register struct thread  *myself = curthread;    /* current thread pointer */
        register int            lockword;               /* lockword value */
        register int            id;                     /* current thread's id */
	register int		link_register;
	register int		lname = NULL;
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */

        if (lockaddr->_status & INSTR_ON) {
                l = lockaddr->_clockp;
                lname = l->lockname;
        }
        else
                l = (struct lock_data_instrumented *)lockaddr;

#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	/* Read lock word in a volatile fashion */
	lockword = ((complex_lock_t)l)->_status;

	id = myself->t_tid ;

        ASSERT((lockword & OWNER_MASK) == id);

	if(((complex_lock_t)l)->_recursion_depth == 0) {
		((complex_lock_t)l)->_flags &= ~RECURSIVE;
#ifdef _INSTRUMENTATION
		TRCHKL3T(HKWD_KERN_SETRECURSIVE|hkwd_CLEARRECURSIVE,lockaddr,link_register,lname);
#else
		TRCHKL3T(HKWD_KERN_SETRECURSIVE|hkwd_CLEARRECURSIVE,l,link_register,lname);
#endif
	}

}

/*
 * NAME: lock_islocked
 *
 * FUNCTION: test if the lock is busy
 *
 * NOTES:
 * 
 * RETURNS: TRUE if locked
 *          FALSE otherwise
 */

#ifdef _INSTRUMENTATION
boolean_t
lock_islocked_instr(complex_lock_t lockaddr)
#else
boolean_t
lock_islocked(complex_lock_t l)
#endif

{
        register int            lockword;               /* lockword value */
	register int    	rc;			/* return value */
#ifdef _INSTRUMENTATION
	register struct lock_data_instrumented	*l;	/* pointer to secondary lock struct */

	if (lockaddr->_status & INSTR_ON)
		l = lockaddr->_clockp;
	else
		l = (struct lock_data_instrumented *)lockaddr;
#endif

	ASSERT(csa->prev == NULL); /* interrupt handlers not allowed */

	/* Read lock word in a volatile fashion */
	lockword = ((complex_lock_t)l)->_status;

	if (lockword & (READ_MODE | WANT_WRITE)) {
		/* lock is held in read mode or (being) taken in write mode */
		rc = TRUE;
	}
	else {
		/* lock is not held */
		rc = FALSE;
	}
	return(rc);
}
