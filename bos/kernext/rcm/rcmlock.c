static char sccsid[] = "@(#)39	1.12.5.1  src/bos/kernext/rcm/rcmlock.c, rcm, bos41J, 9520A_all 5/3/95 14:02:20";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Locking
 *
 * FUNCTIONS:
 *   lock_hw		- locks the hardware
 *   unlock_hw		- unlocks the hardware
 *   lock_domain	- locks a domain
 *   unlock_domain	- unlocks a domain
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/user.h>			/* user structure */
#include <sys/ioacc.h>
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/syspest.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "xmalloc_trace.h"

BUGVDEF(dbg_rcmlock,99);

/* ============================================================= */
/*
    Domain States: (in the descriptions below "owns" means "has
	    a rendering context current on the domain")

	LOCKED: a gp may or may not be doing work on
	    the domain; no gp should be able to do anything
	    to the domain except the locking gp; the gp should
	    not lose control of the domain until it specifically
	    unlocks (NOTE: locking the device consists of locking
	    all the domains of the device).  Device locks are
	    nestable at the application level.  The application
	    level individual domain lock is not currently nestable
	    (and may be scheduled for removal).

	GUARDED: the domain is "suspended" in that no activity by
	    a process in user mode can take place; however, the
	    device specific functions may be manipulating the
	    adapter.  Not currently nestable.

	SUSPENDED: the domain has a context on it, but no graphics
	    process has authority to access it, thus there is no
	    activity on the domain; the domain is available to the
	    first process that wants it, either from a fault or
	    a dispatch (NOTE: a domain can be both GUARDED and
	    SUSPENDED if the gp guarding a domain does not own it
	    at the time it guards it)

	RUNNING (not LOCKED and not GUARDED and not SUSPENDED) and
	    TIMER ON: a gp owns the domain and the timer is
	    going; the gp should not lose control of the
	    domain until the timer has expired

	RUNNING and TIMER OFF: a gp owns the domain and the timer is
	    not going; the gp can lose control of the domain at any
	    time

	SWITCHING: the domain is in the process of switching; the
	    switch was explicitly started by the device independent
	    RCM; a gp cannot do anything to the domain until the
	    switch is complete (NOTE: in at least one instance,
	    a domain can be guarded and switching... when a gp has
	    done a set_rcx)

	BLOCKED: the logical OR of:
	    1.  Adapter (ie device, hw) locked.
	    2.  Individual domain locked.
	    3.  Individual domain guarded.

*/
/* ============================================================= */

/******************************************************************/
/******************************************************************
THIS MODULE CONTAINS FUNCTIONS THAT DISABLE AND ENABLE INTERRUPTS
THEREFORE IT MUST BE PINNED (OR SOME OF IT MUST BE)

    The reason the routines disable interrupts is to ensure that
    the fault list and other structures do not get traversed or
    modified when in a unlinked state.

 ******************************************************************/
/******************************************************************/


/* ============================================================= */
/* FUNCTION: gsc_lock_hw
*/
/* PURPOSE: locks the device
*/
/* DESCRIPTION:
	This function does not force switching to any rcx owned by
	the gp requesting the device lock. It simply
	suspends the rendering contexts on all domains. This
	means that the timer for the domain is stopped, any lock
	removed (unless owned by the locking gp), etc.
	The rcx is left current however, so that any
	subsequent switches proceed correctly.
*/
/* INVOCATION:
    int gsc_lock_hw (pd, parg)
	struct phys_displays *pd;
	lock_hw 	*parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Assumes that check for domain blocked before
	a rcx switch or dispatch

	We seem to have a choice of these schemes:
	    (a) use e_wait and e_post and maintain our own
		list of processes waiting on the unlock and wake
		the process with the highest priority rcx...
		this requires additional structures
	    (b) use e_sleep and e_wakeup and let the kernel
		keep the list and wake up all gps and let them
		fight it out...
		this requires no additional structures

	FOR NOW, USING SCHEME (b)

	Locking a device consists of locking all domains. First,
	however, set the device locked flag to prevent other gps
	from locking individual domains.
*/

int gsc_lock_hw (pd, parg)

    struct phys_displays  *pd;
    lock_hw	     *parg;
{
    lock_hw	    a;
    rcmProcPtr	    pproc;
    int 	    domain;
    gscDevPtr	    pdev;
    int		    pid, flags;

    BUGLPR(dbg_rcmlock,BUGNFO,("\n==== Enter lock_hw\n"));
    gsctrace (LOCK_HW, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### lock_hw ERROR not a gp \n"));
	return (EINVAL);
    }
    RCM_TRACE (0x280, getpid (), pproc, pdev);

    BUGLPR(dbg_rcmlock,BUGACT,("====== lock_dev... found proc=0x%x\n", pproc));

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### lock_dev ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== lock_dev... wait=%d, pdev=0x%x\n", a.wait, pdev));

    /*
     *  Lock the device.  If the nowait flag is set, and rcm_lock_pdev
     *  returns a nonzero value, then that value is the pid of the
     *  process that has the device adapter-locked.
     *
     *  NOTE:  This pid information may not be timely when the
     *  user process gets it.
     */
    flags = PDEV_GUARD;			/* guard all domains */
    if (!a.wait)
	flags |= PDEV_COND;		/* conditional lock */

    pid = rcm_lock_pdev (pdev, pproc, flags);

    if (!a.wait && pid != 0)
    {
	RCM_TRACE (0x285, getpid (), pid, 0);
	suword (&parg->status, pid);			/* set user word */
	return (EIO);
    }

    BUGLPR(dbg_rcmlock,BUGNFO, ("==== Exit lock_dev... \n\n"));

    gsctrace (LOCK_HW, PTID_EXIT);
    RCM_TRACE (0x289, getpid (), 0, 0);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_unlock_hw
*/
/* PURPOSE: unlocks the device
*/
/* DESCRIPTION:
	Gets all domains running again, if possible.
*/
/* INVOCATION:
    int gsc_unlock_hw (pd, parg)
	struct phys_displays *pd;
	unlock_hw	    *parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	see NOTES for lock_hw

	Before this function is done, have the following classes
	of domains:
	    1) has rcx from gp locking device; after unlock of domain,
	       will be running with timer off, but should dispatch
	       if fault list not empty
	    2) has rcx from another gp (suspended); after unlock of
	       domain, still suspended, should dispatch if fault list
	       empty, otherwise will remain suspended
*/

int gsc_unlock_hw (pd, parg)

    struct phys_displays *pd;
    unlock_hw	    *parg;
{
    unlock_hw	    a;
    rcmProcPtr	    pproc;
    int 	    domain;
    gscDevPtr	    pdev;
    ulong	    old_int;
    int 	    rc;

    BUGLPR(dbg_rcmlock,BUGNFO,("\n==== Enter unlock_hw\n"));
    gsctrace (UNLOCK_HW, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);
    BUGLPR(dbg_rcmlock,BUGNFO, ("====== unlock_dev... pdev=0x%x\n", pdev));

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### unlock_hw ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmlock,BUGACT,("====== unlock_dev... found proc=0x%x\n", pproc));

    RCM_TRACE (0x290, getpid (), pproc, pdev);

    /*
     *  Unlock the device structure, retaining nesting,
     *  unguarding domains if exiting the last nesting level
     *  and returning an error if the lock isn't ours.
     */
    rc = rcm_unlock_pdev (pdev, pproc, PDEV_COND);

    gsctrace (UNLOCK_HW, PTID_EXIT);
    RCM_TRACE (0x299, getpid (), pproc, pdev);
    return rc;
}


/* ============================================================= */
/* FUNCTION: gsc_lock_domain
*/
/* PURPOSE: locks a domain
*/
/* DESCRIPTION:
	The calling graphics process will either get the lock
	immediately, or will wait until the domain can be
	locked.
*/
/* INVOCATION:
    int gsc_lock_domain (pd, parg)
	struct phys_displays     *pd;
	lock_domain	    *parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	The "no wait" option only applies to gp locks of device
	or domain.

	WARNING: Attempting to lock multiple domains with lock_domain
	    can result in deadlock.

*/

int gsc_lock_domain (pd, parg)

    struct phys_displays	    *pd;
    lock_domain 	    *parg;
{
    lock_domain     a;
    rcmProcPtr	    pproc;
    gscDevPtr	    pdev;
    devDomainPtr    pdom;
    ulong	    old_int;

    BUGLPR(dbg_rcmlock,BUGNFO,("\n==== Enter lock_domain\n"));
    gsctrace (LOCK_DOMAIN, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### lock_domain ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmlock,BUGACT,
	   ("====== lock_domain... found proc=0x%x\n", pproc));

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### lock_domain ERROR copyin arg \n"));
	return (EFAULT);
    }
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== lock_domain... domain=%d, wait=%d\n", a.domain, a.wait));

    /* check parms */
    if (a.domain < 0 || a.domain > (pdev->devHead.num_domains - 1)) {
	BUGPR(("###### lock_domain ERROR, bad domain \n"));
	return (EINVAL);
    }
    pdom = &pdev->domain[a.domain];
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== lock_domain... pdev=0x%x, pdom=0x%x\n", pdev, pdom));
	
    /* disable interrupts */
    old_int = i_disable (INTMAX);

    RCM_TRACE(0x240,getpid(),pdom,pdom->flags);

    /* if the domain gp locked or the device gp locked */
    if (pdom->flags & (DOMAIN_LOCKED | DOMAIN_DEV_LOCKED)) {
	
	/* if locked by caller */
	if (pdom->pLockProc == pproc) {
	    /* return OK */
	    i_enable (old_int);
	    return (0);
	}
	
	/* if no wait */
	if (! a.wait) {
	    /* return already locked */
	    i_enable (old_int);
	    suword (&parg->status, pdom->pLockProc->procHead.pid);
	    return (EIO);
	
	}
    }

    /* if just domain locked and process wants to wait need
       to signal gp owning domain gp lock - will get done in
       guard_dom().
       lock the domain, will not return until domain locked
       for calling process */
    guard_dom (pdom, pproc, old_int, GUARD_LOCK | GUARD_DISABLED);

    /* return */
    suword (&parg->status, 0);

    BUGLPR(dbg_rcmlock,BUGNFO, ("==== Exit lock_domain... \n\n"));

    gsctrace (LOCK_DOMAIN, PTID_EXIT);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_unlock_domain
*/
/* PURPOSE: unlocks a domain
*/
/* DESCRIPTION:
	Assumes that unlocking gp has had the domain long
	enough and forces switch if possible
*/
/* INVOCATION:
    int gsc_unlock_domain (pd, parg)
	struct phys_displays     *pd;
	unlock_domain	    *parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_unlock_domain (pd, parg)

    struct phys_displays *pd;
    unlock_domain     *parg;

{
    unlock_domain   a;
    rcmProcPtr	    pproc;
    int 	    domain;
    gscDevPtr	    pdev;
    devDomainPtr    pdom;
    ulong	    old_int;

    BUGLPR(dbg_rcmlock,BUGNFO,("\n==== Enter unlock_domain\n"));
    gsctrace (UNLOCK_DOMAIN, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### unlock_domain ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmlock,BUGACT,
	   ("====== unlock_domain... found proc=0x%x\n", pproc));

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### unlock_domain ERROR copyin arg \n"));
	return (EFAULT);
    }
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== unlock_domain... domain=%d\n", a.domain));

    /* check parms */
    if (a.domain < 0 || a.domain > (pdev->devHead.num_domains - 1)) {
	BUGPR(("###### unlock_domain ERROR, bad domain \n"));
	return (EINVAL);
    }
    pdom = &pdev->domain[a.domain];
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== unlock_domain... pdev=0x%x, pdom=0x%x\n", pdev, pdom));
	
    /* if domain not locked by gp */
    if (pdom->pLockProc != pproc) {
	/* return */
	return (EINVAL);
    }

    RCM_TRACE(0x241,getpid(),pdom,pdom->flags);

    /* unlock domain */
    unguard_dom (pdom, UNGUARD_UNLOCK);

    /* return */
    gsctrace (UNLOCK_DOMAIN, PTID_EXIT);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_give_up_timeslice
*/
/* PURPOSE: Informs that a domain has been unlocked
*/
/* DESCRIPTION:
	Assumes that unlocking gp does not have the domain locked
	and forces switch if possible
*/
/* INVOCATION:
    int gsc_give_up_timeslice (pd, parg)
	struct phys_displays     *pd;
	give_up_timeslice    *parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_give_up_timeslice (pd, parg)

    struct phys_displays *pd;
    give_up_timeslice     *parg;

{
    give_up_timeslice   a;
    rcmProcPtr	    pproc;
    int 	    domain;
    gscDevPtr	    pdev;
    devDomainPtr    pdom;
    ulong	    old_int;

    BUGLPR(dbg_rcmlock,BUGNFO,("\n==== Enter give_up_timeslice\n"));
    gsctrace (GIVE_UP_TIMESLICE, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### give_up_timeslice ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmlock,BUGACT,
	   ("====== give_up_timeslice... found proc=0x%x\n", pproc));

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### give_up_timeslice ERROR copyin arg \n"));
	return (EFAULT);
    }
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== give_up_timeslice... domain=%d\n", a.domain));

    /* check parms */
    if (a.domain < 0 || a.domain > (pdev->devHead.num_domains - 1)) {
	BUGPR(("###### give_up_timeslice ERROR, bad domain \n"));
	return (EINVAL);
    }
    pdom = &pdev->domain[a.domain];
    BUGLPR(dbg_rcmlock,BUGNFO,
	   ("====== give_up_timeslice... pdev=0x%x, pdom=0x%x\n", pdev, pdom));
	
    RCM_TRACE(0x261,getpid(),pdom,pdom->flags);

    /* give up timeslice */
    old_int = i_disable (INTMAX);

    /* give up timeslice and dispatch the domain */
    gp_give_up_time_slice (pdom, pproc);

    /*
     *  If a wakeup on the guardlist has been issued, but none of the
     *  waiters has blocked the domain, then make this process fault
     *  when it next tries to touch the domain.
     *
     *  DOMAIN_GUARD_WAKED and DOMAIN_GUARD_PENDING will both be reset
     *  if the domain gets guarded before the fault from this process.
     *  In that case, only the DOMAIN_SUSPENDED flag will be left on.
     *  In the simple case the fault handler will just reconnect the process
     *  to its domain.
     *
     *  Since it appears that a context switch can occur during the
     *  time that the app makes the gsc_give_up_timeslice system call,
     *  we must check thoroughly that our context is still on the domain
     *  before we manipulate bus access or flag settings.  If our context
     *  is no longer on the domain, then we don't have to do anything more.
     */
    if ((pdom->flags & DOMAIN_GUARD_WAKED) &&
	pdom->pCurProc == pproc &&
	!(pdom->flags & (DOMAIN_SWITCHING | DOMAIN_SUSPENDED)) )
    {
	fix_mstsave (pproc, pdom, FIX_MSTSAVE_SUB);
	pdom->flags |= (DOMAIN_SUSPENDED | DOMAIN_GUARD_PENDING);
    }

    i_enable (old_int);

    /* return */
    gsctrace (GIVE_UP_TIMESLICE, PTID_EXIT);
    return (0);
}

/* =============================================================

		INTERNAL FUNCTIONS

   ============================================================= */

/* -------------------------------------------------------------

    guard_dom
	
	    This is a locking mechanism for the domain.  When called,
	    it waits for any domain switching to stop, any other
	    process domain blocks to go away, and any other process
	    time slice to expire, then grants the requested domain block
	    to the current process.

	    See Domain States description above.

	    Processes sleep on the domain "guard list." The guard list
	    is awakened in dispatch_dom.

	    Uses:

	    Guards the domain during system calls to the device
	    specific functions which may force a context switch.

	    Can be used by rcm_lock_pdev to lock the adapter.
*/

void guard_dom (pdom, pproc, in_int, flags)

    devDomainPtr    pdom;	/* domain to guard */
    rcmProcPtr	    pproc;	/* process to guard for */
    ulong	    in_int;	/* input interrupt mask, if enter disabled */
    ulong	    flags;	/* flags to indicate alternative action */

{
    ulong   old_int;

    /* if entering function with interrupts disabled */
    if (flags & (GUARD_DISABLED | GUARD_NO_ENABLE) ) {
	/* set up interrupt mask */
	old_int = in_int;
    } else {
	/* disable interrupts */
	old_int = i_disable(INTMAX);
    }

    RCM_TRACE(0x200,getpid(),pdom,pdom->flags);

    /*
     *  Sleep until the domain gets to a state where we can set our
     *  domain block.
     */
    while (1)
    {
	int  must_sleep;

#ifdef RCM_CHECK
	rcm_check_lists(pdom);
#endif

	must_sleep = guard_dom_sleep_test (pdom, pproc);

	/*
	 *  If we can block the domain without sleeping, then do it!
	 */
	if (!must_sleep)
	    break;

	/*
	 *  Lock feedback protocol:
	 *
	 *  Send necessary feedback signals before sleeping, provided the
	 *  domain timer is off.  This means that each user of the domain
	 *  gets to finish his time slice before receiving signals to get off.
	 */
	if (!(pdom->flags & DOMAIN_TIMER_ON))
	{
	    if (must_sleep < 0)			/* fast domain lock */
		sig_fast_dom_lock (pdom->pCur, 2);
	    else if (pdom->flags & DOMAIN_LOCKED) /* std domain lock */
		pidsig (pdom->pLockProc->procHead.pid, SIGURG);
	}

	e_sleep (&pdom->guardlist, EVENT_SHORT);

	/*
	 *  When waked, check all the conditions again.
	 */
    }

    /*
     *  Clear flags that make dispatcher take note of pending executors
     *  that may block the domain.  We are going to set the block.
     *  This is all to prevent fast domain lock monopoly.
     */
    pdom->flags &= ~DOMAIN_GUARD_WAKED;
    pdom->flags &= ~DOMAIN_GUARD_PENDING;

    /*
     *  If the guard'er doesn't own the context, suspend the context that
     *  is on it.  This context will have to fault to get access back, even
     *  if it never gets switched off.
     */
    if (pdom->pCurProc != NULL && pproc != pdom->pCurProc)
    {
	fix_mstsave (pdom->pCurProc, pdom, FIX_MSTSAVE_SUB);
	pdom->flags |= DOMAIN_SUSPENDED;
    }

    /*
     *  Set the appropriate blocking bit, and block owner.
     */
    if (flags & GUARD_DEV_LOCK)			/* adapter lock */
	pdom->flags |= DOMAIN_DEV_LOCKED;
    else if (flags & GUARD_LOCK)		/* syscall domain lock */
	pdom->flags |= DOMAIN_LOCKED;
    else					/* guard only */
	pdom->flags |= DOMAIN_GUARDED;

    pdom->pLockProc = pproc;			/* owner of block */

    RCM_TRACE(0x201,getpid(),pdom,pdom->flags);

    /* enable interrupts */
    if (!(flags & GUARD_NO_ENABLE))
	i_enable (old_int);
}

/*
 *  guard_dom_sleep_test - Check for sleep requirement, without causing
 *			   effects.
 *
 *  Return +/-1 (TRUE) or 0 (FALSE).  -1 means fast-domain-lock is set by
 *  another process.
 *
 *  This must be called with interrupts disabled.
 */
int guard_dom_sleep_test (
	devDomainPtr  pdom,
	rcmProcPtr  pproc)
{
	int  must_sleep;

	must_sleep = 0;				/* init, no sleep required */

	/*
	 *  If the domain is switching, we can't do anything but sleep.
	 *  It is true that we can know if we are switching to one of our
	 *  contexts, but guard_dom is used to pause execution until the
	 *  switching is done.  We don't want callers of guard_dom to
	 *  have to check to see if switching is still going on.  To do
	 *  so would involve changing other aspects of the rcm.
	 */
	if (pdom->flags & DOMAIN_SWITCHING)
	{
	    RCM_TRACE (0x202, getpid (), pdom->flags, 0);
	    must_sleep = 1;
	}
	else
	/*
	 *  The domain is not switching.
	 *
	 *  If the context on the domain has the domain fast-locked, AND
	 *  if the context is not suspended, AND
	 *  if the context is not ours, then sleep.
	 *
	 *  Fast domain-lock takes precedence over other locks.
	 *
	 *  Anyone with a fast domain lock set while they are on the
	 *  adapter AND are suspended is considered to have called
	 *  give-up-time-slice (and been conditionally suspended) and
	 *  to not yet have faulted back onto the domain.
	 *
	 *  There are only two places in the rcm where anyone MAY get
	 *  suspended:  1)  guard_dom, and 2) gsc_give_up_timeslice.
	 */
	if (pproc != pdom->pCurProc &&			/* not ours */
	    !(pdom->flags & DOMAIN_SUSPENDED) &&
	    ck_fast_dom_lock (pdom->pCur) != 0)		/* fast locked? */
	{
	    RCM_TRACE (0x203, getpid (), pproc->procHead.pid,
					pdom->pCurProc->procHead.pid);

	    must_sleep = -1;			/* sleep + fast dom lock */

	    RCM_ASSERT (!(pdom->flags & DOMAIN_BLOCKED) ||
		    pproc == pdom->pLockProc, 0, 0, 0, 0, 0);
	}
	else
	/*
	 *  The domain is not switching.  Also, there is no fast domain
	 *  lock set (or, if there is, it's ours!).
	 *
	 *  Handle domain blocks.  If DOMAIN_LOCK or DOMAIN_DEV_LOCK,
	 *  and it's not our block, then sleep.  Always sleep on simple
	 *  guard, since it (currently) isn't nestable.  However, it IS
	 *  nestable with the two other domain blocking bits.
	 *
	 *  WARNING:  DOMAIN_GUARDED is not nestable by the same (or any)
	 *            process!
	 */
	if (((pdom->flags & DOMAIN_GUARDED                     )        ) ||
	    ((pdom->flags & (DOMAIN_LOCKED | DOMAIN_DEV_LOCKED)) &&
						pproc != pdom->pLockProc)    )
	{
	    RCM_TRACE (0x204, getpid (), pdom->flags, pdom->pLockProc);
	    must_sleep = 1;
	}
	else
	/*
	 *  The domain is not switching.  Also, there is no fast domain
	 *  lock set (or, if there is, it's ours!).  And, there are no
	 *  domain blocks set.
	 *
	 *  If the domain timer is on and the context on the domain isn't ours,
	 *  then sleep.
	 */
	if ((pdom->flags & DOMAIN_TIMER_ON) && pproc != pdom->pCurProc)
	{
	    RCM_TRACE (0x205, getpid (), pdom->flags, pdom->pCurProc);
	    must_sleep = 1;
	}

	RCM_TRACE (0x206, getpid (), must_sleep, 0);

	return (must_sleep);
}

/* -------------------------------------------------------------

    unguard_dom
	
	    Resumes activity on a domain after it has been
	    guarded or locked.

	    Uses:

	    Unguards the domain after system calls to the device
	    specific functions which may force a context switch.

	    Also unlocks a domain.

    NOTES:
	    The current design favors processes on the guard list,
	    i.e., if the guard list is not empty, unguard will wake
	    those on the list rather than try to dispatch from the
	    fault list.

	    A process can lock (domain or device) lock a domain, and
	    then guard the domain. The process can then unguard the
	    domain and keep it locked. However, since guarding is
	    a temporary thing, it is not correct to guard, lock,
	    unlock, and unguard.
*/

void unguard_dom (pdom, flags)

    devDomainPtr    pdom;
    ulong	    flags;

{
    ulong   old_int;

    /* disable interrupts */
    old_int = i_disable(INTMAX);

    RCM_TRACE(0x210,getpid(),pdom,pdom->flags);

    /*
     *  Release the various locks available.  Note that we allow
     *  simple guard to be nested within either one of the other two.
     *  This nesting is by the same locking process.
     */
    if (flags & UNGUARD_DEV_UNLOCK)		/* lock adapter flag */
    {
	pdom->flags &= ~DOMAIN_DEV_LOCKED;
	pdom->pLockProc = NULL;
    }
    else if (flags & UNGUARD_UNLOCK)		/* lock single domain flag */
    {
	pdom->flags &= ~DOMAIN_LOCKED;
	pdom->pLockProc = NULL;
    }
    else					/* simple unguard */
    {
	pdom->flags &= ~DOMAIN_GUARDED;

	/* Don't release pLockProc if real lock remains */
	if (!(pdom->flags & (DOMAIN_LOCKED | DOMAIN_DEV_LOCKED)))
	    pdom->pLockProc = NULL;
    }

    /*
     *  Find the next user of the domain, if any.
     */
    dispatch_dom (pdom);

    i_enable (old_int);

#ifdef RCM_CHECK
    rcm_check_lists(pdom);
#endif
}

#ifdef RCM_CHECK
rcm_check_lists(pdom)
struct _devDomain *pdom;
{
	int i;
	struct _gscDev	*pdev = pdom->pDev;
	struct _rcmProc *pproc;
	struct _rcmWG	*pwg;
	struct _rcmWG	*pwgt;
	struct _rcmWA	*pwa;
	struct _rcmWA	*pwat;
	struct _rcx	*prcx;
	struct _rcx	*prcxt;

	RCM_ASSERT(pdev, 0, 0, 0, 0, 0);
	if(pdev->devHead.pProc) {
		for(pproc = pdev->devHead.pProc;
				pproc;
				pproc = pproc->procHead.pNext) {
			for(prcx = pproc->procHead.pRcx;
					prcx;
					prcx = prcx->pNext) {
				if(prcx->pWG) {
					FIND_WG(pdev, prcx->pWG, pwg);
					if (!pwg) BUGPR(
					("wg 0x%x for rcx 0x%x not in pdev 0x%x list\n",
						prcx->pWG, prcx, pdev));
					RCM_ASSERT(pwg, 0, 0, 0, 0, 0);
					for(prcxt = pwg->pHead;
							prcxt;
							prcxt = prcxt->pLinkWG) {
						if(prcxt == prcx)
							break;
					}
					RCM_ASSERT(prcxt == prcx, 0, 0, 0, 0, 0);
				}
				if(prcx->pWA) {
					FIND_WA(pproc, prcx->pWA, pwa);
					if (!pwa) BUGPR(
					("wa 0x%x for rcx 0x%x not in pproc 0x%x list\n",
						prcx->pWA, prcx, pproc));
					RCM_ASSERT(pwa, 0, 0, 0, 0, 0);
					for(prcxt = pwa->pHead;
							prcxt;
							prcxt = prcxt->pLinkWA) {
						if(prcxt == prcx)
							break;
					}
					RCM_ASSERT(prcxt == prcx, 0, 0, 0, 0, 0);
				}
			}
		}
	}
}
#endif

gp_guard_domain(pdom, pproc)
devDomainPtr	pdom;	/* domain to guard */
rcmProcPtr	pproc;	/* process to guard for */
{
	guard_dom(pdom, pproc, 0, GUARD_ONLY);
}

gp_unguard_domain(pdom)
devDomainPtr	pdom;	/* domain to guard */
{
	unguard_dom(pdom, UNGUARD_ONLY);
}

/*
 *  rcm_lock_pdev - Lock the device structure.  Also, optionally guard all
 *		    domains with the "device lock" guard mode.  This
 *		    combination is also called "adapter lock".
 *
 *  The following flags enable special actions.
 *
 *	PDEV_COND  - Return error code if device is already adapter-locked
 *		     and not by this process.  Do not honor this code if the
 *		     device is not adapter locked, even though this lock
 *		     operation may then sleep for simple pdev lock or domain
 *		     guards to clear up.
 *
 *		     Currently, the "error code" is the pid of the process
 *		     which has the device adapter-locked.
 *
 *		     This pid may not always be accurate after interrupts are
 *		     allowed.
 *
 *	PDEV_GUARD - Guard all domains with the "device lock" after locking
 *		     the device structure.  This only applies at the first
 *		     lock of a potential nest.  This causes "adapter lock".
 *
 *  Nesting is allowed and accounted for.
 */
int rcm_lock_pdev (pdev, pproc, flags)
gscDevPtr	pdev;
rcmProcPtr	pproc;
int		flags;
{
	int	old_int, domain;

	/*
	 *  Go under interrupt protection to check/set flags.
	 */
	old_int = i_disable (INTMAX);

	RCM_TRACE(0x220, getpid(), flags, pdev->devHead.flags);
	RCM_TRACE(0x221, getpid(), pdev->devHead.locknest, 0);

	/*
	 *  See if we can perform the device lock (and any optional dev-
	 *  lock guards), but not as a single unitary operation.
	 *
	 *  This consists of two phases:
	 *
	 *  1)  Lock the pdev structure.  This is the same lock whether
	 *      for simple pdev lock or all-inclusive adapter lock.
	 *
	 *  2)  Optionally (for adapter lock),
	 *      guard all the device domains successfully, using the
	 *      GUARD_DEV_LOCK guard bit.  This bit is only for the use
	 *      of adapter lock.  Successfully guarding all the domains
	 *      this way constitutes "adapter lock".
	 *
	 *  There is a conditional non-waiting call which tests for
	 *  adapter lock already set.
	 */

	/*
	 *  Process the conditional request.
	 *
	 *  Note that the returned pid may not be accurate once i_enable occurs!
	 */
	if ((flags & PDEV_COND)                               &&
	    (pdev->devHead.flags & DEV_GP_LOCKED_AND_GUARDED) &&
	    pdev->devHead.pLockProc != pproc                     )
	{
	    int    pid;

	    /*  pick up pid while under interrupt protection */
	    pid = pdev->devHead.pLockProc->procHead.pid;

	    RCM_TRACE (0x228, getpid (), pid, pdev->devHead.flags);

	    i_enable (old_int);

	    return (pid);
	}

	/*
	 *  If the device is locked by someone else, then sleep.
	 *
	 *  In the conditional case, the device is not DEV_GP_LOCKED, OR
	 *  the lock (if any), is ours.  This means we won't sleep.
	 */
	while ((pdev->devHead.flags & DEV_GP_LOCKED) &&
	       pdev->devHead.pLockProc != pproc         )
	{
	    RCM_ASSERT (pdev->devHead.pLockProc, 0, 0, 0, 0, 0);

	    RCM_TRACE (0x222, getpid (), pdev->devHead.pLockProc->procHead.pid,
		    							0);

	    e_sleep (&pdev->devHead.locklist, EVENT_SHORT);
	}

	/*
	 *  Now that we have, or can get, the pdev lock, set the lock.
	 */
	if (!(pdev->devHead.flags & DEV_GP_LOCKED))	/* if no nest */
	{
	    RCM_ASSERT (pdev->devHead.locknest == 0, 0, 0, 0, 0, 0);
	    RCM_ASSERT (!(pdev->devHead.flags & DEV_GP_LOCKED_AND_GUARDED),
								0, 0, 0, 0, 0);

	    /* device lock settings */
	    pdev->devHead.flags |= DEV_GP_LOCKED;
	    pdev->devHead.pLockProc = pproc;
	    pdev->devHead.locknest = 1;

	     /* adapter lock, too */
	    if (flags & PDEV_GUARD)
	    {
		/* set before before guard so looks like adapter lock */
		pdev->devHead.flags |= DEV_GP_LOCKED_AND_GUARDED;

		/*
		 *  Guard each domain on the device.
		 *  These may sleep.
		 */
		for (domain=0; domain<pdev->devHead.num_domains; domain++)
		    guard_dom (&pdev->domain[domain], pproc, 0,
					GUARD_DEV_LOCK | GUARD_NO_ENABLE);
	    }
	}
	else				/* is already lock, by us */
	{
	    RCM_ASSERT (pdev->devHead.locknest > 0, 0, 0, 0, 0, 0);
	    RCM_ASSERT (!(flags & PDEV_GUARD) ||
		    (pdev->devHead.flags & DEV_GP_LOCKED_AND_GUARDED),
								0, 0, 0, 0, 0);

	    pdev->devHead.locknest++;
	}

	i_enable (old_int);

	RCM_TRACE (0x229, getpid (),
			pdev->devHead.locknest, pdev->devHead.flags);

	return (0);
}


/*
 *  rcm_unlock_pdev - Unlock the device structure.  Handle complexities
 *		      that occur in case of nesting and adapter lock.
 *
 *  The following flags enable special actions.
 *
 *	PDEV_COND    - Return error code EINVAL if device is not locked
 *		       or is locked by some other process (else assert!).
 *
 *	PDEV_UNNEST  - Pop off all nesting, else just remove one level.
 */
int rcm_unlock_pdev (pdev, pproc, flags)
gscDevPtr	pdev;
rcmProcPtr	pproc;
int		flags;
{
	int	old_int, domain;

	/*
	 *  Go under interrupt protection to check flags.
	 */
	old_int = i_disable (INTMAX);

	RCM_TRACE (0x230, getpid(), flags, pdev->devHead.flags);
	RCM_TRACE (0x231, getpid(), pdev->devHead.locknest, 0);

	/*
	 *  First, process the PDEV_COND flag.
	 *
	 *  If the device is not lock (in any fashion), or if it is not
	 *  locked by us, then return or assert.
	 */
	if (!(pdev->devHead.flags & DEV_GP_LOCKED) ||
	       pdev->devHead.pLockProc != pproc       )
	{
	    RCM_TRACE (0x238, getpid (),
		pdev->devHead.pLockProc ?
		pdev->devHead.pLockProc->procHead.pid : 0,
		0);

	    i_enable (old_int);

	    if ((flags & PDEV_COND))
		return (EINVAL);
	    else
		assert (0);
	}

	/*
	 *  Unguard the domains if they were guarded when we entered
	 *  the nest (for lock adapter), AND if we are exiting the nest.
	 *
	 *  Go out from under interrupt protection to unguard to facilitate
	 *  timely dispatching of processes waiting for unguard.  I don't
	 *  think this is actually necessary.
	 *
	 *  Don't set any pdev locking info to 'unlocked' state, yet.
	 */
	if (pdev->devHead.flags & DEV_GP_LOCKED_AND_GUARDED)
	{
	    /* is it time to unguard? */
	    if ((flags & PDEV_UNNEST) || pdev->devHead.locknest == 1)
	    {
		i_enable (old_int);

		for (domain = 0; domain < pdev->devHead.num_domains; domain++)
		    unguard_dom (&pdev->domain[domain], UNGUARD_DEV_UNLOCK);

		old_int = i_disable (INTMAX);
	    }
	}

	/*
	 *  Recognize the PDEV_UNNEST flag to properly pop nesting level.
	 *
	 *  It would seem simpler to do this above the previous paragraph,
	 *  but would sometimes leave locknest zeroed when we go out of
	 *  interrupt protection to unguard a domain.  Keep life simple.
	 */
	if (flags & PDEV_UNNEST)
	    pdev->devHead.locknest = 0;
	else
	    pdev->devHead.locknest--;

	RCM_ASSERT (pdev->devHead.locknest >= 0, 0, 0, 0, 0, 0);

	/*
	 *  If exiting the outermost of a lock nest, then release the
	 *  lock.
	 */
	if (pdev->devHead.locknest == 0)		/* last nest */
	{
	    pdev->devHead.flags &= ~DEV_GP_LOCKED;
	    pdev->devHead.flags &= ~DEV_GP_LOCKED_AND_GUARDED;
	    pdev->devHead.pLockProc = NULL;

	    if (pdev->devHead.locklist != EVENT_NULL)
		e_wakeup (&pdev->devHead.locklist);
	}

	i_enable (old_int);

	RCM_TRACE (0x239, getpid (),
				pdev->devHead.locknest, pdev->devHead.flags);

	return (0);
}


/*
 *  ck_fast_dom_lock - Return the fast domain lock value.
 *
 *  This must be called with interrupts disabled.  The domain
 *  must not be switching.
 */
int  ck_fast_dom_lock (rcxPtr  prcx)
{
    int  rc = 0;

    /*
     *  If the context on the domain has an extension in user space
     *  in which the context lock (fast-domain-lock) flag exists, then
     *  return the value of that flag.  Else, return zero (not locked).
     */
    if (prcx && prcx->pDomainLock &&
	xmemin (prcx->pDomainLock,
		&prcx->DomLockCopy,
		sizeof(struct DomainLock),
		&prcx->XmemDomainLock) == XMEM_SUCC)
    {
	rc = prcx->DomLockCopy.DomainLocked;
    }

    return  rc;
}


/*
 *  sig_fast_dom_lock - Signal the fast domain lock process.
 *
 *  This must be called with interrupts disabled.  The domain
 *  must not be switching.  The lock structure must have been
 *  read in from the user space via ck_fast_dom_lock.  Furthermore,
 *  no dispatching can have taken place before sig_fast_dom_lock
 *  is called, or there will be a race condition concerning the
 *  lock flag with the app!
 */
int  sig_fast_dom_lock (rcxPtr  prcx, int  val)
{
    /*
     *  Tell the locking process that we need an unlock.
     */
    if (prcx && prcx->pDomainLock)		/* make sure it exists */
    {
	int  rc;

        prcx->DomLockCopy.TimeSliceExpired = val;

        rc = xmemout (&prcx->DomLockCopy,	/* write it out */
		      prcx->pDomainLock, 
		      sizeof(struct DomainLock), 
		      &prcx->XmemDomainLock);

        /* exterminate him on error */
        if (rc != XMEM_SUCC)
        {
	    RCM_TRACE(0x206,getpid(),prcx->pDomain,2);

	    pidsig (prcx->pDomain->pLockProc->procHead.pid, SIGTERM);
        }
    }

    return  0;
}


/*
 *  clr_fast_dom_lock - Clear the fast domain locks in the user process.
 *
 *  This must be called with interrupts disabled.  The domain
 *  must not be switching.
 */
int  clr_fast_dom_lock (rcxPtr  prcx)
{
    /*
     *  Clear out the locking protocol flags in the user space.
     */
    if (prcx && prcx->pDomainLock)		/* make sure it exists */
    {
	int  rc;

        prcx->DomLockCopy.DomainLocked = 0;
        prcx->DomLockCopy.TimeSliceExpired = 0;

        rc = xmemout (&prcx->DomLockCopy,	/* write it out */
		      prcx->pDomainLock, 
		      sizeof(struct DomainLock), 
		      &prcx->XmemDomainLock);

        /* exterminate him on error */
        if (rc != XMEM_SUCC)
        {
	    RCM_TRACE(0x206,getpid(),prcx->pDomain,2);

	    pidsig (prcx->pDomain->pLockProc->procHead.pid, SIGTERM);
        }
    }

    return  0;
}
