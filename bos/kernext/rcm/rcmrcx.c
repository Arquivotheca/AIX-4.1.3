static char sccsid[] = "@(#)40	1.37.5.1  src/bos/kernext/rcm/rcmrcx.c, rcm, bos41J, 9520A_all 5/3/95 11:45:14";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Context Management
 *
 * FUNCTIONS:
 *    create_rcx	- creates a rendering context
 *    delete_rcx	- deletes a rendering context
 *    bind_window	- binds a window and a rendering context
 *    set_rcx		- sets the current rendering context for a graphics
 *			  process
 *    create_rcxp	- creates a rendering context part
 *    delete_rcxp	- deletes a rendering context part
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
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/uio.h>
#include <sys/ioacc.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "xmalloc_trace.h"

/* functions defined in this module, declared extern so can be void */
extern void unlink_rcx_wg();
extern void unlink_rcx_wa();

BUGVDEF(dbg_rcmrcx,99);		/* for system calls */
BUGVDEF(dbg_rcmrcx_int,0);	/* for fault/interrupt calls */

int  rm_rcx_fault (devDomainPtr, rcxPtr);
rcxPtr  find_null_context (rcmProcPtr, int);

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
/* FUNCTION: gsc_create_rcx
*/
/* PURPOSE: creates a rendering context for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_create_rcx (pd, parg)
	struct phys_displays 	*pd;
	create_rcx		*parg;
*/
/* CALLS:
	vddcreate_rcx	- to do device dependent stuff for creating
			  rendering context
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to rendering context
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_create_rcx (pd, parg)

    struct phys_displays	*pd;
    create_rcx		*parg;
{
    create_rcx	a;
    rcmProcPtr	pproc;
    rcxPtr	prcx;
    char	*pdd_data;	/* device dependent data */
    gscDevPtr	pdev;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter create_rcx\n"));
    gsctrace (CREATE_RCX, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### create_rcx ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== create_rcx... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### create_rcx ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("====== create_rcx... domain=%d, pData=0x%x, len=%d\n",
	    a.domain, a.pData, a.length));

    /* check parms */
    if (a.domain < 0 || a.domain > (pdev->devHead.num_domains - 1)) {
	BUGPR(("###### create_rcx ERROR, bad domain \n"));
	return (EINVAL);
    }

    /* allocate the rcx and initialize */
    if((prcx = xmalloc (sizeof (struct _rcx), 3, pinned_heap)) == NULL) {
	BUGPR(("###### create_rcx ERROR xmalloc rcx \n"));
	return (ENOMEM);
    }
    prcx->pNextFlt = NULL;
    prcx->pProc = pproc;
    prcx->priority = pproc->procHead.priority;
    prcx->pData = NULL;
    prcx->domain = a.domain;
    prcx->pDomain = &pdev->domain[a.domain];
    prcx->pWG = NULL;
    prcx->pWA = NULL;
    prcx->pLinkWG = NULL;
    prcx->pLinkWA = NULL;
    prcx->pRcxph = NULL;
    prcx->flags = RCX_ALLOCATED;
    prcx->timeslice = RCM_DEFAULT_TIMESLICE;
    prcx->pDomainLock = NULL;

    /* return handle */
    if (suword (&parg->rcx_handle, prcx)) {
	BUGPR(("###### create_rcx ERROR suword returning handle \n"));
	xmfree ((caddr_t) prcx, pinned_heap);
	return (EFAULT);
    }

    /* get dd data */
    if (a.pData == NULL) {
	pdd_data = NULL;
    } else {
	if ((pdd_data = xmalloc (a.length, 3, kernel_heap)) == NULL)  {
	    BUGPR(("###### create_rcx ERROR xmalloc for dd data\n"));
	    xmfree ((caddr_t) prcx, pinned_heap);
	    return (ENOMEM);
	}
	if (copyin (a.pData, pdd_data, a.length)) {
	    BUGPR(("###### create_rcx ERROR	copyin for dd data\n"));
	    xmfree ((caddr_t) pdd_data, kernel_heap);
	    xmfree ((caddr_t) prcx, pinned_heap);
	    return (EFAULT);
	}
    }

    /*
     * Attach to the area for coordinating domain locks with the user 
     * process.
     */
    if (a.pDomainLock) {
	int rc;
	prcx->pDomainLock = (caddr_t) a.pDomainLock;
	prcx->XmemDomainLock.aspace_id = XMEM_INVAL;
	if(xmattach((char *) a.pDomainLock, sizeof(struct DomainLock),
		&prcx->XmemDomainLock, USER_ADSPACE) != XMEM_SUCC) {
		xmfree((caddr_t) pdd_data, kernel_heap);
		xmfree((caddr_t) prcx, pinned_heap);
		return(ENOMEM);
	}
	rc = pinu((caddr_t) a.pDomainLock, sizeof(struct DomainLock),
		UIO_USERSPACE);
	if(rc != 0) {
		xmdetach(&prcx->XmemDomainLock);
		xmfree((caddr_t) pdd_data, kernel_heap);
		xmfree((caddr_t) prcx, pinned_heap);
		return(ENOMEM);
	}
    }

    RCM_TRACE(0x300,getpid(),prcx,0);

    /* call vddcreate_rcx */
    /************** WARNING ***********************************/
    /* it is assumed that device specific create functions do */
    /* not have to be guarded, because they will not cause a  */
    /* context switch					      */
    /************** WARNING ***********************************/
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    a.error = (pd->create_rcx) (pdev, prcx, &a, pdd_data);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (a.error) {
	if (a.pDomainLock)
	{
	    unpinu ((caddr_t) a.pDomainLock, sizeof(struct DomainLock),
								UIO_USERSPACE);
	    xmdetach (&prcx->XmemDomainLock);
	}
	xmfree ((caddr_t) prcx, pinned_heap);
        xmfree ((caddr_t) pdd_data, kernel_heap);
	suword (&parg->error, a.error);
	BUGPR(("###### create_rcx ERROR dd create_rcx error, %d \n", a.error));
	return (EIO);
    }

    /* free the old data */
    if (pdd_data != NULL)
        xmfree ((caddr_t) pdd_data, kernel_heap);

    /* link rcx into process list */
    prcx->pNext = pproc->procHead.pRcx;
    pproc->procHead.pRcx = prcx;

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== Exit create_rcx... prcx=0x%x\n\n", prcx));

    gsctrace (CREATE_RCX, PTID_EXIT);
    return 0;
}


/* ============================================================= */
/* FUNCTION: gsc_delete_rcx
 */
/* PURPOSE: deletes a rendering context for a graphics process
 */
/* DESCRIPTION:
	It deletes a rendering context. If the deleted rcx is
	current for the domain, then perform a rcx switch to
	make sure the adapter gets used as quickly as possible.
*/
/* INVOCATION:
   int gsc_delete_rcx (pd, parg)
	struct phys_displays 	*pd;
	delete_rcx		*parg;
*/
/* CALLS:
	rcm_delete_rcx	- to do the real work
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

int gsc_delete_rcx (pd, parg)

    struct phys_displays	*pd;
    delete_rcx		*parg;

{
    delete_rcx	a;
    rcmProcPtr	pproc;
    rcxPtr	prcx;
    gscDevPtr	pdev;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter delete_rcx\n"));
    gsctrace (DELETE_RCX, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### delete_rcx ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== delete_rcx... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### delete_rcx ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("====== delete_rcx... rcx_handle=0x%x\n", a.rcx_handle));

    /* check parms */
    /* ---- find the rcx, note that a null rcx is NOT ok */
    FIND_RCX(pproc,a.rcx_handle,prcx);
    if ((prcx != (struct _rcx *) a.rcx_handle)
	|| (a.rcx_handle == NULL)) return (EINVAL);
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== delete_rcx... found prcx=0x%x\n", prcx));

    RCM_TRACE(0x310,getpid(),prcx,prcx->pDomain->flags);

    /* delete the rcx */
    if (rcm_delete_rcx (pdev, prcx, &a.error)) {
	suword (&parg->error, a.error);
	return (EIO);
    }

    /* return */
    gsctrace (DELETE_RCX, PTID_EXIT);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_bind_window
 */
/* PURPOSE: binds a window and a rendering context
 */
/* DESCRIPTION:
 */
/* INVOCATION:
    int gsc_bind_window (pd, parg)
	struct phys_displays     *pd;
	bind_window	    *parg;
*/
/* CALLS:
	vdd bind_window     - to fix hardware clipping
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
	Need to consider case where bind a new window to the
	current context for domain. This requires that some
	form of switch take place so that the new clipping
	info gets put in place. This is sort of a graphics
	fault, but only clipping is affected. The interesting
	problem is that the old and the new rcx are the same, so
	may need a special start switch call that references
	only the new clipping.
*/

int gsc_bind_window (pd, parg)
	
    struct phys_displays *pd;
    bind_window     *parg;

{
    bind_window     a;
    rcmProcPtr	    pproc;
    rcmWGPtr	    pwg;
    rcmWAPtr	    pwa;
    rcxPtr	    prcx;
    int 	    error, old_int;
    gscDevPtr	    pdev;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter bind_window\n"));
    gsctrace (BIND_WINDOW, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### bind_window ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== bind_window... found proc=0x%x\n", pproc));

    /* get argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### bind_window ERROR copyin arg \n"));
	return (EFAULT);
    }

    /* find the rcx, note that a null rcx is NOT ok */
    FIND_RCX(pproc,a.rcx,prcx);
    if ((prcx != (struct _rcx *) a.rcx) || (a.rcx == NULL))
	return (EINVAL);
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== bind_window... found prcx=0x%x\n", prcx));

    /* if binding the same components, return */
    if ( (prcx->pWG == (struct _rcmWG *) a.wg)
	&& (prcx->pWA == (struct _rcmWA *) a.wa) )
	return (0);

    if ((a.wa && !a.wg) || (!a.wa && a.wg)) {
	BUGPR(("###### bind_window ERROR attr and geom inconsistent \n"));
	return(EINVAL);
    }
    if (a.wa) {
	    /* find the window attr */
	    FIND_WA(pproc,a.wa,pwa);
	    if (pwa != (struct _rcmWA *) a.wa) return (EINVAL);
	    BUGLPR(dbg_rcmrcx,BUGACT,
		   ("====== bind_window... found pwa=0x%x\n", pwa));
    } else {
	pwa = NULL;
    }

    /* rcm lock device so wg list can't change */
    rcm_lock_pdev (pdev, pproc, 0);

    if (a.wg) {
	    /* find the window geom */
	    FIND_WG(pdev,a.wg,pwg);
	    if (pwg != (struct _rcmWG *) a.wg) {
		BUGPR(("###### bind_window ERROR geom bad \n"));
		/* rcm unlock device */
    		rcm_unlock_pdev (pdev, pproc, 0);
		return (EINVAL);
	    }
	    BUGLPR(dbg_rcmrcx,BUGACT,
		   ("====== bind_window... found pwg=0x%x\n", pwg));
    } else {
	pwg = NULL;
	if(pwa == NULL) {
		if(prcx->pDomain->pCur == prcx) {
		BUGPR(("###### bind_window ERROR cant unbind current rcx \n"));
			/* rcm unlock device */
    			rcm_unlock_pdev (pdev, pproc, 0);
			return (EINVAL);
		}
	}
    }

    RCM_TRACE(0x320,getpid(),prcx,pwg);

    /*
     *  Set up this link for the convenience of certain device specific codes.
     *  This allows us to remember the WA that this WG was last bound to, after
     *  the WG becomes unbound.
     *
     *  The existence of this link does not imply that the WA it points to
     *  is still allocated.  Driver writers take heed!
     */
    if(pwg)
	pwg->pLastWA = pwa;

    /* give the device specific code a chance to
       switch the clipping part of the rcx */
    GUARD_DOM (prcx->pDomain, pproc, 0, GUARD_ONLY);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    a.error = (pd->bind_window) (pdev, prcx, pwg, pwa);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    UNGUARD_DOM (prcx->pDomain, UNGUARD_ONLY);
    if (a.error) {
	suword (&parg->error, a.error);
	BUGPR(("###### bind_window ERROR dd bind_window=%d\n", a.error));
	/* rcm unlock device */
        rcm_unlock_pdev (pdev, pproc, 0);
	return (EIO);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== bind_window... return from dd bind\n"));

    /* if the new wg different than old */
    if (pwg != prcx->pWG) {
	/* unlink the rcx from the old geom */
	if(prcx->pWG)
		unlink_rcx_wg (prcx);
	/* bind the geom to the rcx */
	prcx->pWG = pwg;
	/* link the rcx into the list for the geom */
	if (pwg) {
		old_int = i_disable (INTMAX);
		prcx->pLinkWG = pwg->pHead;
		pwg->pHead = prcx;
		i_enable (old_int);
	}
    }

    /* rcm unlock device */
    rcm_unlock_pdev (pdev, pproc, 0);

    /* if the new wa different than old */
    if (pwa != prcx->pWA) {
	/* unlink the rcx from the old attr */
	if(prcx->pWA)
		unlink_rcx_wa (prcx);
	/* bind the attr to the rcx */
	prcx->pWA = pwa;
	/* link the rcx into the list for the attr */
	if (pwa) {
		prcx->pLinkWA = pwa->pHead;
		pwa->pHead = prcx;
	}
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== Exit bind_window... prcx=0x%x\n\n", prcx));

    gsctrace (BIND_WINDOW, PTID_EXIT);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_set_rcx
*/
/* PURPOSE: sets the current rendering context for a graphics process
*/
/* DESCRIPTION:
	If the gp that calls this function owns the rcx
	current on the domain, then this action is the same
	as a graphics fault, i.e., the rcx is put in the
	list of faulting gps and it goes thru the
	scheduler, with perhaps some bias towards the new
	rcx.
*/
/* INVOCATION:
    int gsc_set_rcx (pd, parg)
	struct phys_displays     *pd;
	set_rcx 	    *parg;
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
	This function causes a switch to the set rcx
	if the gp owns the rcx current on the domain. Another
	paradigm would be to switch to it only if the fault
	list is empty or if the rcx current on the domain
	has not run "long enough;" if not, then would place
	the rcx on the fault list, dispatch the first rcx on
	the list and wait.

*/

int gsc_set_rcx (pd, parg)
	
    struct phys_displays *pd;
    set_rcx	    *parg;

{
    set_rcx	    a;
    rcmProcPtr	    pproc;
    rcxPtr	    prcx;
    gscDevPtr	    pdev;
    devDomainPtr    pdom;
    ulong	    old_int;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter set_rcx\n"));
    gsctrace(SET_RCX, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### set_rcx ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT, ("====== set_rcx... found proc=0x%x\n", pproc));


    /*-----------------------------------------------------------------------
       get argument (Copy parameters from application space)
     *-----------------------------------------------------------------------*/
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### set_rcx ERROR copyin arg \n"));
	return (EFAULT);
    }

    /*-----------------------------------------------------------------------
       check parms 
     *-----------------------------------------------------------------------*/
    if(a.rcx != NULL) {
	    /* ---- find the rcx */
	    FIND_RCX(pproc,a.rcx,prcx);
	    if ((prcx != (struct _rcx *) a.rcx)) return (EINVAL);
	    BUGLPR(dbg_rcmrcx,BUGACT,
		   ("====== set_rcx... found prcx=0x%x\n", prcx));
	    if (!prcx->pWA || !prcx->pWG) {
		BUGPR(("###### set_rcx ERROR no geom or attr \n"));
		return(EINVAL);
	    }
    } else {
	for (prcx = pproc->procHead.pRcx; prcx; prcx = prcx->pNext) {
		if ((prcx->flags & RCX_NULL) && a.domain == prcx->domain)
			break;
	}
	if(!prcx) {
		BUGPR(("###### set_rcx ERROR invalid domain for RCX_NULL\n"));
		return(EINVAL);
	}
    }

    RCM_TRACE(0x330,getpid(),prcx,0);

    /*-----------------------------------------------------------------------
       disable interrupts   
     *-----------------------------------------------------------------------*/
    old_int = i_disable (INTMAX);

    /*-----------------------------------------------------------------------
       set domain 
     *-----------------------------------------------------------------------*/
    pdom = prcx->pDomain;


    /*-----------------------------------------------------------------------
        If the rcx current on the domain is owned by our process
        (but is not the one we want), or if the domain is owned by
        no one, then consider putting this context on the domain.
     *-----------------------------------------------------------------------*/
     
    if (pdom->pCur == NULL ||
	(pdom->pCurProc == pproc && prcx != pdom->pCur))
    {
	/* this test allows the rcx for the domain to be switched
	   even if the domain is locked by the calling gp */
	
	/*
	 *  We used to cancel any timer here because we didn't want
	 *  the following guard_dom to get hung up on it.  But, if
	 *  you inspect the logic of guard_dom you will see that the
	 *  process can't wait even if the timer is on as long as
	 *  we own the context that is on the domain.
	 *
	 *  Letting the timer run helps the time-slice expire so that
	 *  the current process won't monopolize the domain if it makes
	 *  continuous gsc_set_rcx calls.
	 */

	/* guard domain to make sure that don't try to start a switch
	   while one is going on */
	guard_dom (pdom, pproc, 0, GUARD_ONLY | GUARD_NO_ENABLE);

	/*
	 *  Since we could have slept in the guard_dom, we must
	 *  reconnoiter our purpose again.
	 *
         *  If the rcx current on the domain is owned by our process
         *  (but is not the one we want), or if the domain is owned by
         *  no one, then put this context on the domain.
         */
        if (pdom->pCur == NULL ||
	    (pdom->pCurProc == pproc && prcx != pdom->pCur))
	{
    	    /*---------------------------------------------------------------
     	      Record the current context selected/activated by this process.  
     	      Note that this is significant only for multiple-context processes.
    	     *---------------------------------------------------------------*/
            pproc->pDomainCur[prcx->domain]    = prcx;

	    /* switch to new rcx now */
	    BUGLPR(dbg_rcmrcx,BUGACT,
	       ("====== set_rcx... switch rcx on domain\n"));

	    /* this function reenables interrupts */
	    rcx_switch (pdom->pCur, prcx, RCX_SWITCH_DOIT, old_int);

	    /* unguard domain */
	    unguard_dom (pdom, UNGUARD_ONLY);
        }
        /*
         *  The rcx on the domain didn't belong to our process, or if it
	 *  did, it's the one we want to stay on.
         */
	else
	{
    	    /*---------------------------------------------------------------
	      The INNER test -- after the guard domain -- for domain owned 
	      by this process failed.  So no context switch is done here.

     	      Record the current context selected/activated by this process.  
     	      Note that this is significant only for multiple-context processes.
    	     *---------------------------------------------------------------*/
            pproc->pDomainCur[prcx->domain]    = prcx;

	    i_enable (old_int);
	    unguard_dom (pdom, UNGUARD_ONLY);
	}
    }
    /*
     *  The rcx on the domain didn't belong to our process, or if it did,
     *  it's the one we want to stay on.
     */
    else
    {
    	/*---------------------------------------------------------------
	  The OUTER test for domain owned by this process failed.
	   (So no context switch is done here.)

     	  Record the current context selected/activated by this process.  
     	  Note that this is significant only for multiple-context processes.
    	 *---------------------------------------------------------------*/
        pproc->pDomainCur[prcx->domain]    = prcx;

	i_enable (old_int);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO, ("==== Exit set_rcx... prcx=0x%x\n\n", prcx));

    gsctrace(SET_RCX, PTID_EXIT);
    return (0);
}

/* ============================================================= */
/* FUNCTION: gsc_create_rcxp
*/
/* PURPOSE: creates a rendering context part for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_create_rcxp (pd, parg)
	struct phys_displays 	*pd;
	create_rcxp		*parg;
*/
/* CALLS:
	vddcreate_rcxp	- to do device dependent stuff for creating
			  rendering context part
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to rendering context part
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_create_rcxp (pd, parg)

    struct phys_displays	*pd;
    create_rcxp 	*parg;
{
    create_rcxp a;
    struct _rcxp *pRcxp;
    char	*pdd_data;	/* device dependent data */
    gscDevPtr	pdev;
    rcmProcPtr	pproc;
    unsigned int old_int;
    int old_part = 0, lock_stat;
    struct _partList *part;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter create_rcxp\n"));
    gsctrace (CREATE_RCXP, PTID_ENTRY);

    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### create_rcxp ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== create_rcx... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### create_rcxp ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
       ("==== create_rcxp...id=%x, priority %d flags %x pData=0x%x len=%d\n",
		    a.id, a.priority,
		    a.flags, a.pData, a.length));

    RCM_LOCK (&comAnc.rlock, lock_stat);

    /* check parms */

    FIND_RCXP(a.id, pRcxp);

    RCM_TRACE (0x350,getpid (), a.id, pRcxp);
    BUGLPR(dbg_rcmrcx,BUGNFO, ("create_rcxp: pRcxp=0x%x\n", pRcxp));

    if (pRcxp != NULL) {
	old_part = 1;
    } else {

	    /* allocate the rcx and initialize */
	    if((pRcxp = xmalloc (sizeof (struct _rcxp), 3, pinned_heap)) == NULL) {
		BUGPR(("###### create_rcxp ERROR xmalloc rcxp \n"));
	        RCM_UNLOCK (&comAnc.rlock, lock_stat);
		return (ENOMEM);
	    }
	    pRcxp->pNext = NULL;
	    pRcxp->priority = a.priority;
	    pRcxp->pData = NULL;
	    pRcxp->users = 0;
	    pRcxp->flags = a.flags;
	    pRcxp->glob_id = a.id;
    }

    /*
     *  Get the space now which we will use to add to the owner list of
     *  the part down below.  This is separated to simplify error paths.
     */
    if ((part = xmalloc (sizeof (struct _partList), 3, pinned_heap)) == NULL)
    {
	BUGPR(("###### create_rcxp ERROR xmalloc part \n"));
	if (!old_part)
	    xmfree ((caddr_t) pRcxp , pinned_heap);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	return (ENOMEM);
    }

    /* return handle */
    if (suword (&parg->rcxp, pRcxp)) {
	BUGPR(("###### create_rcxp ERROR suword returning handle \n"));
	if (!old_part)
	    xmfree ((caddr_t) pRcxp , pinned_heap);
	xmfree ((caddr_t) part, pinned_heap);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	return (EFAULT);
    }

    /* get dd data */
    if (a.pData == NULL) {
	pdd_data = NULL;
    } else {
	if ((pdd_data = xmalloc (a.length, 3, kernel_heap)) == NULL)  {
	    BUGPR(("###### create_rcxp ERROR xmalloc for dd data\n"));
	    if (!old_part)
	        xmfree ((caddr_t) pRcxp, pinned_heap);
	    xmfree ((caddr_t) part, pinned_heap);
	    RCM_UNLOCK (&comAnc.rlock, lock_stat);
	    return (ENOMEM);
	}
	if (copyin (a.pData, pdd_data, a.length)) {
	    BUGPR(("###### create_rcxp ERROR	copyin for dd data\n"));
	    xmfree ((caddr_t) pdd_data, kernel_heap);
	    if (!old_part)
	        xmfree ((caddr_t) pRcxp, pinned_heap);
	    xmfree ((caddr_t) part, pinned_heap);
	    RCM_UNLOCK (&comAnc.rlock, lock_stat);
	    return (EFAULT);
	}
    }

    /* call vddcreate_rcxp */
    /************** WARNING ***********************************/
    /* it is assumed that device specific create functions do */
    /* not have to be guarded, because they will not cause a  */
    /* context switch					      */
    /************** WARNING ***********************************/
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    a.error = (pd->create_rcxp) (pdev, pRcxp, &a, pdd_data);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (a.error) {
	if (pdd_data != NULL)
	    xmfree ((caddr_t) pdd_data, kernel_heap);
	if (!old_part)
	    xmfree ((caddr_t) pRcxp, pinned_heap);
	xmfree ((caddr_t) part, pinned_heap);
	RCM_UNLOCK (&comAnc.rlock, lock_stat);
	suword (&parg->error, a.error);
	BUGPR(("###### create_rcxp ERROR dd create_rcxp error, %d \n", a.error));
	return (EIO);
    }

    /* free the old data */
    if (pdd_data != NULL)
        xmfree ((caddr_t) pdd_data, kernel_heap);

    /*
     *  Add ownership record to rcmproc structure list.  No protection
     *  required, on thread.  There will be multiple records for the same
     *  process for the same part if multiple creates are done.
     */
    part->pRcxp = pRcxp;
    part->pNext = pproc->procHead.pParts;
    pproc->procHead.pParts = part;

    /* update user count, no protection required, common lock */
    pRcxp->users++;

    BUGLPR(dbg_rcmrcx,BUGNFO, ("create_rcxp: pRcxp->users=%d\n", pRcxp->users));

    if(!old_part) {
	    BUGLPR(dbg_rcmrcx,BUGNFO, ("create_rcxp: link\n"));
    /* link rcxp into common list (protect'n for nonlocked list scanners(?)) */
	    old_int = i_disable (INTMAX);
	    pRcxp->pNext = apCom->pRcxParts;
	    apCom->pRcxParts = pRcxp;
	    i_enable (old_int);
    }

    RCM_UNLOCK (&comAnc.rlock, lock_stat);

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== Exit create_rcx... prcxp=0x%x\n\n", pRcxp));

    gsctrace (CREATE_RCXP, PTID_EXIT);
    return 0;
}

/* ============================================================= */
/* FUNCTION: gsc_delete_rcxp
*/
/* PURPOSE: deletes a rendering context part for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_delete_rcxp (pd, parg)
	struct phys_displays 	*pd;
	delete_rcxp		*parg;
*/
/* CALLS:
	vdddelete_rcxp	- to do device dependent stuff for creating
			  rendering context part
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to rendering context part
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_delete_rcxp (pd, parg)

    struct phys_displays	*pd;
    delete_rcxp 	*parg;
{
    delete_rcxp a;
    struct _rcxp *pRcxp;
    rcmProcPtr	pproc;
    gscDevPtr	pdev;
    int rc = 0, lock_stat;
    struct _partList *part, *part_prev;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter delete_rcxp\n"));
    gsctrace (DELETE_RCXP, PTID_ENTRY);

    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### delete_rcxp ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== delete_rcxp... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### delete_rcxp ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== delete_rcxp...rcxp=%x\n", a.rcxp));

    /* check parms */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    FIND_RCXP_BY_HANDLE(a.rcxp, pRcxp);
    if (pRcxp == NULL) {
	BUGPR(("###### delete_rcxp ERROR, part doesn't exist \n"));
        RCM_UNLOCK (&comAnc.rlock, lock_stat);
	return (EEXIST);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
			("delete_rcxp: pRcxp->glob_id=%x\n", pRcxp->glob_id));

    /* see if we did a create for this one */
    part = pproc->procHead.pParts;
    part_prev = NULL;
    while (part)
    {
	struct _partList *part_next;

	part_next = part->pNext;

	BUGLPR(dbg_rcmrcx,BUGNFO,
		("delete_rcxp: check part 0x%x part->pRcxp 0x%x\n",
							part, part->pRcxp));

	if (part->pRcxp == pRcxp)
	{
	    BUGLPR(dbg_rcmrcx,BUGNFO, ("delete_rcxp: found\n"));

	    rc = rcm_delete_rcxp (pdev, part, part_prev, &a.error, pproc);

	    break;
	}

	part_prev = part;
	part = part_next;
    }

    RCM_UNLOCK (&comAnc.rlock, lock_stat);

    /* if we aren't on the list, return error */
    if (!part)
    {
	BUGPR(("###### delete_rcxp ERROR part not found\n"));
	return (ENOENT);
    }

    /* if driver error */
    if (a.error) {
	suword (&parg->error, a.error);
	BUGPR(("###### delete_rcxp ERROR dd delete_rcxp error, %d \n", a.error));
	return (EIO);
    }

    gsctrace (DELETE_RCXP, PTID_EXIT);
    return rc;
}

/* ============================================================= */
/* FUNCTION: gsc_associate_rcxp
*/
/* PURPOSE: associate a rendering context part with a rendering context
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_associate_rcxp (pd, parg)
	struct phys_displays 	*pd;
	associate_rcxp		*parg;
*/
/* CALLS:
	vddassociate_rcxp	- to do device dependent stuff for
			   associating rendering context part
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to rendering context part
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_associate_rcxp (pd, parg)

    struct phys_displays	*pd;
    associate_rcxp	*parg;
{
    associate_rcxp	a;
    associate_rcxp	*ap = &a;
    struct _rcxph **prcxph_arr;
    struct _rcxp *pRcxp;
    struct _rcx *prcx;
    rcmProcPtr	pproc;
    gscDevPtr	pdev;
    int i, length, lock_stat;
    int rc = 0, err;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter associate_rcxp\n"));
    gsctrace (ASSOCIATE_RCXP, PTID_ENTRY);

    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### associate_rcxp ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== associate_rcxp... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### associate_rcxp ERROR copyin arg \n"));
	return (EFAULT);
    }

    /* get actual length */
    length = sizeof (struct _associate_rcxp);
    if (a.length > 5)
	length += sizeof (RCXP_Handle) * (a.length - 5);

    ap = xmalloc (length, 3, kernel_heap);
    if (ap == NULL)
	return (ENOMEM);

    if (copyin (parg, ap, length)) {
	BUGPR(("###### associate_rcxp ERROR copyin arg \n"));
	xmfree ((caddr_t) ap, kernel_heap);
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== associate_rcxp...rcx=%x length %d\n", ap->rcx, ap->length));

    /* check parms */
    FIND_RCX(pproc, ap->rcx, prcx);
    if(!prcx) {
	BUGPR(("###### associate_rcxp ERROR, rcx doesn't exist \n"));
	xmfree ((caddr_t) ap, kernel_heap);
	return (EINVAL);
    }

    prcxph_arr = xmalloc(sizeof (struct _rcxph *) * ap->length, 3, kernel_heap);

    if(!prcxph_arr) {
	BUGPR(("###### associate_rcxp ERROR, allocating rcxh array \n"));
	xmfree((caddr_t) ap, kernel_heap);
	return(ENOMEM);
    }

    /* On error free both ap and prcxph_arr */

    /* rcm lock device so wg list can't change */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    err = 0;
    for(i = 0; i < ap->length; i++)
    {
	FIND_RCXP_BY_HANDLE(ap->asso[i], pRcxp);
	if (pRcxp == NULL)
	{
	    BUGPR(("###### associate_rcxp ERROR, part doesn't exist \n"));
	    err = EEXIST;
	    break;
	}

	prcxph_arr[i] = xmalloc (sizeof(struct _rcxph), 3, pinned_heap);
	if(!prcxph_arr[i])
	{
	    BUGPR(("###### associate_rcxp ERROR,allocating part header\n"));
	    err = ENOMEM;
	    break;
	}
    }

    /* on error, retract all allocations */
    if (err)
    {
        for (i--; i>=0; i--)
	    xmfree ((caddr_t) prcxph_arr[i], pinned_heap);

        xmfree ((caddr_t) prcxph_arr, kernel_heap);
	xmfree ((caddr_t) ap, kernel_heap);

        RCM_UNLOCK (&comAnc.rlock, lock_stat);

	return (err);
    }

    /*  'i' is sacred until we don't need any more error processing */
    /* call vddassociate_rcx to give the device specific code a chance to
       set up  */
    BUGLPR(dbg_rcmrcx,BUGACT, ("====== asso_rcxp... call dd asso_rcxp\n"));
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    ap->error = (pdev->devHead.display->associate_rcxp) (pdev, ap);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (ap->error) {
	    BUGPR(("###### associate_rcxp ERROR dd associate_rcxp=%d \n",
		   ap->error));
	    suword (&parg->error, ap->error);
	    /* rcm unlock device */
	    RCM_UNLOCK (&comAnc.rlock, lock_stat);

            for (i--; i>=0; i--)
	        xmfree ((caddr_t) prcxph_arr[i], pinned_heap);

            xmfree ((caddr_t) prcxph_arr, kernel_heap);
	    xmfree ((caddr_t) ap, kernel_heap);
	    return (EIO);
    }

    /* can't have an error if you clobber 'i' */
    for(i = 0; i < ap->length; i++)
    {
	struct _rcxph *prh;

	prh = prcxph_arr[i];

	BUGLPR(dbg_rcmrcx,BUGACT,
	("====== asso_rcxp... linking in part header 0x%x for part 0x%x\n",
		prh, ap->asso[i]));

	prh->pRcxp = (struct _rcxp *) ap->asso[i];
	((struct _rcxp *)ap->asso[i])->users++;
	prh->flags = 0;
	prh->pNext = prcx->pRcxph;
	if(prcx->pRcxph)
		    prcx->pRcxph->pPrev = prh;
	prcx->pRcxph = prh;
    }

    xmfree ((caddr_t) prcxph_arr, kernel_heap);
    xmfree ((caddr_t) ap, kernel_heap);

    BUGLPR(dbg_rcmrcx,BUGACT, ("====== asso_rcxp... unlocking common structure\n"));
    /* rcm unlock device */
    RCM_UNLOCK (&comAnc.rlock, lock_stat);
    gsctrace (ASSOCIATE_RCXP, PTID_EXIT);
    return rc;
}

/* ============================================================= */
/* FUNCTION: gsc_disassociate_rcxp
*/
/* PURPOSE: disassociate a rendering context part with a rendering context
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_disassociate_rcxp (pd, parg)
	struct phys_displays 	*pd;
	disassociate_rcxp		*parg;
*/
/* CALLS:
	vdddisassociate_rcxp	- to do device dependent stuff for
			   disassociating rendering context part
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to rendering context part
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_disassociate_rcxp (pd, parg)

    struct phys_displays	*pd;
    disassociate_rcxp	*parg;
{
    disassociate_rcxp	a;
    disassociate_rcxp	*ap = &a;
    struct _rcxph *prcxph, *prcxpht;
    struct _rcxp *pRcxp;
    struct _rcx *prcx;
    rcmProcPtr	pproc;
    gscDevPtr	pdev;
    int i, length, lock_stat;
    int rc = 0;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter disassociate_rcxp\n"));
    gsctrace (DISASSOCIATE_RCXP, PTID_ENTRY);

    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### disassociate_rcxp ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== disassociate_rcxp... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### disassociate_rcxp ERROR copyin arg \n"));
	return (EFAULT);
    }

    length = sizeof (struct _disassociate_rcxp);
    if(a.length > 5)
	length += sizeof (RCXP_Handle) * (a.length - 5);

    ap = xmalloc (length, 3, kernel_heap);
    if (ap == NULL)
	return (ENOMEM);

    if (copyin (parg, ap, length)) {
	BUGPR(("###### disassociate_rcxp ERROR copyin arg \n"));
	xmfree((caddr_t) ap, kernel_heap);
	return (EFAULT);
    }

    BUGLPR(dbg_rcmrcx,BUGNFO,
	  ("==== disassociate_rcxp...rcx=%x length %d\n", ap->rcx, ap->length));

    /* check parms */
    FIND_RCX(pproc, a.rcx, prcx);
    if(!prcx) {
	BUGPR(("###### disassociate_rcxp ERROR, rcx doesn't exist \n"));
	xmfree((caddr_t) ap, kernel_heap);
	return (EINVAL);
    }

    /* rcm lock device so wg list can't change */
    RCM_LOCK (&comAnc.rlock, lock_stat);

    /* call vdddisassociate_rcx to give the device specific code a chance to
       set up  */
    BUGLPR(dbg_rcmrcx,BUGACT,("====== disasso_rcxp... call dd disasso_rcxp\n"));

    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    ap->error = (pdev->devHead.display->disassociate_rcxp) (pdev, ap);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (ap->error) {
	    BUGPR(("###### disassociate_rcxp ERROR dd disassociate_rcxp=%d \n",
		   ap->error));
    	    RCM_UNLOCK (&comAnc.rlock, lock_stat);
	    xmfree ((caddr_t) ap, kernel_heap);
	    suword (&parg->error, ap->error);
	    return (EIO);
    }

    for(prcxph = prcx->pRcxph; prcxph; )
    {
	BUGLPR(dbg_rcmrcx,BUGACT,
		("====== disasso_rcxp... checking part header 0x%x part 0x%x\n",
			prcxph, prcxph->pRcxp));

	for(i=0; i < a.length; i++)
	{
	    if((struct _rcxp *)ap->dasso[i] == prcxph->pRcxp)
	    {
		BUGLPR(dbg_rcmrcx,BUGACT,
		("====== disasso_rcxp... deallocating part header 0x%x 0x%x\n",
						prcxph,prcxph->pNext));

		prcxph->pRcxp->users--;

		if(prcxph->pPrev)
		    prcxph->pPrev->pNext = prcxph->pNext;
		else
		    prcx->pRcxph = prcxph->pNext;
		if(prcxph->pNext)
		    prcxph->pNext->pPrev = prcxph->pPrev;
		prcxpht = prcxph;
		prcxph = prcxph->pNext;

		xmfree((caddr_t) prcxpht, pinned_heap);
	    }
	}
    }

    /* rcm unlock device */
    RCM_UNLOCK (&comAnc.rlock, lock_stat);
    gsctrace (DISASSOCIATE_RCXP, PTID_EXIT);
    return rc;
}

/* =============================================================

   INTERNAL FUNCTIONS

   ============================================================= */


/* ============================================================= */
/* FUNCTION: rcm_delete_rcx
*/
/* PURPOSE: deletes a rendering context for a graphics process
*/
/* DESCRIPTION:
	It deletes a rendering context. If the deleted rcx is
	current for the domain, then perform a rcx switch to
	make sure the adapter gets used as quickly as possible.
*/
/* INVOCATION:
    int rcm_delete_rcx (pdev, prcx, perror)
	gscDevPtr	pdev;
	rcxPtr		prcx;
	int		*perror;
*/
/* CALLS:
	vdddelete_rcx	- to do device dependent stuff for deleting
			    rendering context
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

int rcm_delete_rcx (pdev, prcx, perror)
	
    gscDevPtr	pdev;
    rcxPtr	prcx;
    int 	*perror;

{
    rcxPtr	    prcxt, prcx_null;
    devDomainPtr    pdom = prcx->pDomain;
    rcmProcPtr	    pproc = prcx->pProc;
    int		    domain = prcx->domain;
    struct _rcxph   *prcxph, *prcxpht;
    int             rc = 0;
    int 	    old_int;
    int		    selected, on_dom_and_locked;

    BUGLPR(dbg_rcmrcx,BUGNFO,("\n==== Enter del_rcx.. prcx=0x%x\n", prcx));

    RCM_TRACE(0x340,getpid(),prcx,0);

    /*
     *  This function is used to remove the NULL contexts during unmake
     *  gp time.  The NULL contexts must be removed last, or there will
     *  be trouble removing any of the others.  Removing a non-NULL context
     *  may require that the NULL context for the domain still exist.
     */

    /*
     *  This can be called with fast-domain lock or adapter lock set, and
     *  must function appropriately.
     *
     *  Since we desire to adapter lock below, we must guard against a
     *  certain deadlock that can occur when the fast-domain lock is set.
     *  (The adapter lock is nestable, and cannot cause deadlock).  This
     *  deadlock occurs when someone (typically X) has the first half of
     *  a full adapter lock accomplished (lock of pdev), but is waiting
     *  trying to dev-lock a domain which has the fast-domain lock set.
     *  If the process with the fast-lock enters unmake-gp and comes here,
     *  then this lock-adapter call (below) will deadlock with X.
     *
     *  There are two ways to handle this problem:
     *
     *  1)  Rewrite rcm_lock_pdev to make the two step lock operation into
     *      a single unitary operation.  This means that X would still sleep
     *      in the above scenario, but the process with the fast-lock which
     *      is unmaking would be able to successfully perform its own adapter
     *      lock.  It's lock of pdev would work because X slept without having
     *      its lock on pdev set yet; and, the dev-lock of the domain would
     *      succeed because the unmaking gp owned the fast-domain lock.
     *      Upside:  It's good to have unitary locks, and it solves this
     *      problem in a systematic way.
     *      Downside:  The wakeup's on the guardlist's must be more complex,
     *      any wakeup on a guardlist must also include a possible wakeup
     *      on the locklist of the device.  This introduces complexity, which
     *      really isn't too hard to handle, and introduces a longer path
     *      length (but only in those cases where a lock-adapter is pending).
     *
     *  2)  Remove this problem nonsystematically, because this is the only
     *      place where it occurs in normal operation.  This can be done by
     *      clearing the fast lock before performing any lock-adapter.
     *      Upside:  The simplicity of rcm_lock_pdev and guardlist wakeups
     *      as independent of each other can be retained.
     *      Downside:  We need a "clever" solution for this spot.
     *      
     *  This version of the code chooses solution #2, above.
     *
     *  The first thing we do is take care of the fast domain lock, if set.
     *  This is done by checking for its presence, and if present, resetting
     *  it and instituting a switch to the NULL context.  To prevent unwanted
     *  interference, THIS MUST ALL BE DONE AS A SINGLE UNITARY OPERATION.
     *  Preventing interrupts will suffice for this.  The switch will probably
     *  sleep, but the DOMAIN_SWITCHING flag will be set by then so that no
     *  interference on the domain can occur.
     *
     *  Once the fast lock and switch has completed, then we reenable
     *  interrupts and lock the adapter normally.
     */
    old_int = i_disable (INTMAX);		/* critical section */

    selected = (pproc->pDomainCur[domain] == prcx);
    on_dom_and_locked = (!(pdom->flags & DOMAIN_SWITCHING)  &&
			 prcx == pdom->pCur                 &&
			 ck_fast_dom_lock (pdom->pCur) != 0 &&
			 !(pdom->flags & DOMAIN_SUSPENDED)     );

    /* look up NULL context if we will need it for any reason */
    if (selected || on_dom_and_locked)
	prcx_null = find_null_context (pproc, domain);	/* new rcx */

    /*
     *  If the context which is being deleted is on the domain, AND
     *  if the context is fast-locked onto the domain, remove the fast
     *  lock and switch on the NULL context.  NOTE:  This is done whether
     *  the context on the domain is the selected one or not, though it
     *  probably always is for gemini.
     *
     *  NOTES:
     *
     *  This code is currently only used by gemini.
     *  This code should be generalized to become truly device independent.
     *
     *  1)  Can the gemini driver find this out without rcm help?
     *  2)  Does he need a callback for this?
     *  3)  Do we need to do the switch for him, or can he handle
     *      it without our device dependent help?
     */
    if (on_dom_and_locked)
    {
        clr_fast_dom_lock (pdom->pCur);		/* release fast dom locks */

        /*
	 *  Switch to the new context.  Perform any heavy switch in-line.
	 *  Do not queue it to the Heavy Switch Controller.
	 *
	 *  Rcx_switch/rcx_switch_done will not touch the domain timer
	 *  under these circumstances, since we are switching between two
	 *  contexts that belong to the same process.  The timer may be
	 *  running, and may expire at any time.
	 *
	 *  Fast domain lock does not apply here, since we do not
	 *  switch while that lock is in effect.
	 *
	 *  Reenable interrupts.
         */
        rcx_switch (pdom->pCur, prcx_null, RCX_SWITCH_DOIT, old_int);
    }
    else
	i_enable (old_int);			/* end critical section */

    /*
     *  Lock the adapter!  
     *
     *  This is really done because we need a long term 'guard' operations
     *  to span most of this function, plus we need to have regular 'guard'
     *  be nestable within it.  The DD level may call the callback
     *  make_cur_and_guard_dom.
     *
     *  Also, unlinking the window geometry and window attribute structures
     *  from the context must be performed atomically.  Any updating of
     *  window geometry must be pended until all this gets done.
     */
    rcm_lock_pdev (pdev, pproc, PDEV_GUARD);  /* lock and guard all domains */

    /*
     *  If the context being deleted is the selected context, change the
     *  selection to the null context.
     */
    if (selected)
	pproc->pDomainCur[domain] = prcx_null;

    prcxt = pproc->pDomainCur[domain];		/* remember selection */

    /* call DD layer */
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    *perror = (pdev->devHead.display->delete_rcx) (pdev, prcx);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (*perror)
	rc = EIO;

    RCM_ASSERT (prcxt == pproc->pDomainCur[domain], 0, 0, 0, 0, 0);
						/* check selection */

    /*
     *  Now continue the cleanup.
     */

    old_int = i_disable(INTMAX);
    RCM_TRACE(0x342,prcx,pdom->pCur,pdom->pFault);

    /**********************************************************************
    	Clean up the following: 
	  . If the context is on the fault list, remove it. 
	  . If the context is on the adapter, remove it.  (If another 
	      context is waiting, then switch to it.)
	  
	Note that at this point, we have the domain guarded, so can we 
	freely check or change the domain structures or the domain itself, 
	to do away with the old gp.
     **********************************************************************/

    /*
     *  Remove rcx from the fault list (if present) to keep it from
     *  possibly being switched on when we dispatch (unguard_dom).
     */
    (void) rm_rcx_fault (pdom, prcx);

    /* if deleting the current rcx for domain */
    if (prcx == pdom->pCur)
    {
	BUGLPR(dbg_rcmrcx,BUGACT,
	       ("====== del_rcx... deleting cur rcx pdom=0x%x\n", pdom));
	
	/*
	 *  We are going to remove the process bus access, so disable
	 *  the timer now.  There is no purpose in letting it run.  It
	 *  might slow down domain dispatching.
	 */
	if (pdom->flags & DOMAIN_TIMER_ON)
	{
		tstop (pdom->pDevTimer);
		pdom->flags &= ~DOMAIN_TIMER_ON;
	}
	
	/* indicate that have no rcx on domain and remove access authority */
	pdom->pCur = NULL;
	pdom->pCurProc = NULL;

        /*-------------------------------------------------------------------
           Take away the gp's adapter access authority.  (If it tries to 
           use the domain again, a fault will be generated.) 

	   this will force a fault to switch on the null context if the
	       gp tries to write to the adapter again 
         *-------------------------------------------------------------------*/
	fix_mstsave (pproc, pdom, FIX_MSTSAVE_SUB);
	pdom->flags |= DOMAIN_SUSPENDED;
    }

    i_enable (old_int);

    /* unlink the rcx from the process list */
    /* no locking necessary since only the caller can
       manipulate the list */
    /* if first in list */
    prcxt = pproc->procHead.pRcx;
    if (prcxt == prcx) {
	pproc->procHead.pRcx = prcx->pNext;
    } else { /* not first in list, search the rest */
	for (; prcxt->pNext != NULL; prcxt = prcxt->pNext)
	    if (prcxt->pNext == prcx) break;
	/* assume valid rcx, unlink it cause checked for it already */
	RCM_ASSERT(prcxt->pNext != NULL, 0, 0, 0, 0, 0);
	prcxt->pNext = prcxt->pNext->pNext;
    }
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== del_rcx... found prcx=0x%x\n", prcx));

    /* unlink the rcx from the old geom */
    /* pdev is adapter locked! */
    unlink_rcx_wg (prcx);
    BUGLPR(dbg_rcmrcx,BUGACT, ("====== del_rcx... unlinked wg\n"));

    /* unlink the rcx from the old attr */
    unlink_rcx_wa (prcx);
    BUGLPR(dbg_rcmrcx,BUGACT, ("====== del_rcx... unlinked wa\n"));

    /* 
     *  Rcm unlock device.  
     *
     *  This will do any necessary dispatching.
     */
    rcm_unlock_pdev (pdev, pproc, 0);    /* unlock and UNGUARD DOMAIN */

    /*
     *  For all parts, decrement the use count.  Unpin and free headers.
     */
    if (prcxph = prcx->pRcxph)
    {
	for( ; prcxph != NULL; )
	{
	    prcxpht = prcxph;
	    prcxph = prcxph->pNext;

	    old_int = i_disable (INTMAX);	/* users-- isn't unitary */
	    prcxpht->pRcxp->users--;
	    i_enable (old_int);

	    xmfree((caddr_t) prcxpht, pinned_heap);
	}
    }

    if(prcx->pDomainLock)
    {
	xmdetach(&prcx->XmemDomainLock);
	unpinu(prcx->pDomainLock, sizeof(struct DomainLock),
		UIO_USERSPACE);
    }

    /* upin and free the rcx */
    BUGLPR(dbg_rcmrcx,BUGACT,
	   ("====== del_rcx... about to unpin and free rcx=0x%x\n", prcx));
    xmfree ((caddr_t) prcx, pinned_heap);

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== Exit del_rcx... \n"));

    /* return */
    return (0);
}

/*
 *  rm_rcx_fault - Remove a rendering context entry from the fault list
 *		   for a domain.
 *
 *  This must be entered with interrupts prevented!
 */
int  rm_rcx_fault (
devDomainPtr    pdom,
rcxPtr		prcx)
{
    rcxPtr	    pprev, pflt, pflt_n;

    if (prcx->flags & RCX_ON_FAULT_LIST) /* is it on the list? */
    {
	prcx->flags &= ~RCX_ON_FAULT_LIST; /* clear the flag */

	/* unlink from the fault list */
        pflt = pdom->pFault;		/* first entry or NULL */
        pprev = NULL;			/* will be "previous" ptr */

        while (pflt)			/* find entry and unlink it */
        {
	    pflt_n = pflt->pNextFlt;	/* next one or NULL */

	    if (pflt == prcx) 		/* is this the one? */
	    {
		if (!pprev)
		    pdom->pFault = pflt_n;
		else
		    pprev->pNextFlt = pflt_n;

		break;
	    }

	    pprev = pflt;
	    pflt = pflt_n;
	}
    }

    return  0;
}

/* ============================================================= */
/* FUNCTION: rcm_delete_rcxp
*/
/* PURPOSE: deletes a rendering context part for a graphics process
*/
/* DESCRIPTION:
	It deletes a rendering context part. If the deleted rcxp has
	a use count or other than the creating process is calling,
	then return an error.
*/
/* INVOCATION:
    int rcm_delete_rcxp (prcxp, part, part_prev, perror, pproc)
	gscDevPtr	pdev;
	struct _partList *part, *part_prev;
	int		*perror, pproc;
*/
/* CALLS:
	vdddelete_rcxp	- to do device dependent stuff for deleting
			    rendering context part

	This is to be called with the rcm lock in effect!
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

int rcm_delete_rcxp (pdev, part, part_prev, perror, pproc)
	
    gscDevPtr	pdev;
    struct _partList *part, *part_prev;
    int 	*perror;
    rcmProcPtr	pproc;

{
    rcxpPtr	    prcxp = part->pRcxp;
    rcxpPtr	    prcxpt;
    int 	    old_int;

    BUGLPR(dbg_rcmrcx,BUGNFO,
   ("\n==== Enter rcm_del_rcxp pdev 0x%x part 0x%x part_prev 0x%x pproc 0x%x\n",
					 pdev, part, part_prev, pproc));

    RCM_TRACE(0x360,getpid(),prcxp,pproc);

    /*
     *  Remove us from the list.  No protection, on thread.
     */
    if (part_prev == NULL)
    {
	BUGLPR(dbg_rcmrcx,BUGNFO, ("at the head\n"));
	pproc->procHead.pParts = part->pNext;
    }
    else
    {
	BUGLPR(dbg_rcmrcx,BUGNFO, ("mid list\n"));
	part_prev->pNext = part->pNext;
    }

    BUGLPR(dbg_rcmrcx,BUGNFO, ("free part 0x%x\n", part));
    xmfree ((caddr_t) part, pinned_heap);

    /*  no protection, common lock */
    prcxp->users--;
    BUGLPR(dbg_rcmrcx,BUGNFO, ("part user count %d\n", prcxp->users));
    assert (prcxp->users >= 0);

    /* call vdddelete_rcxp to give the device specific code a chance to
       clean up(so that can free any hardware resources used by rcx) */
    BUGLPR(dbg_rcmrcx,BUGACT, ("====== del_rcxp... call dd delete_rcxp\n"));
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    *perror = (pdev->devHead.display->delete_rcxp) (pdev, prcxp);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    if (*perror) {
	    BUGPR(("###### rcm_delete_rcxp ERROR dd delete_rcxp=%d \n",
		   *perror));
    }

    if (prcxp->users != 0)
	return(0);

    /* if first in list */
    /* this function is only called under the common lock */
    /* but also protect list update against any list searchers that
       might not use the common lock */
    old_int = i_disable (INTMAX);
    prcxpt = apCom->pRcxParts;
    if (prcxpt == prcxp) {
	 apCom->pRcxParts = prcxp->pNext;
    } else { /* not first in list, search the rest */
	for (; prcxpt->pNext != NULL; prcxpt = prcxpt->pNext)
	    if (prcxpt->pNext == prcxp) break;
	/* assume valid rcx, unlink it cause checked for it already */
	prcxpt->pNext = prcxpt->pNext->pNext;
    }
    i_enable (old_int);
    xmfree ((caddr_t) prcxp, pinned_heap);

    BUGLPR(dbg_rcmrcx,BUGNFO,
	   ("==== Exit del_rcxp... \n"));

    /* return */
    return (0);
}


/* unlink rcx from list bound to window geometry */

void unlink_rcx_wg (prcx)

    rcxPtr  prcx;

{

    rcxPtr  prcxt;
    int error, old_int;
    rcmWGPtr pwg = prcx->pWG;

    RCM_ASSERT(prcx != NULL, 0, 0, 0, 0, 0);

    old_int = i_disable (INTMAX);

    if (prcx->pWG == NULL)
    {
	i_enable (old_int);
	return;
    }
    if ((prcxt = prcx->pWG->pHead) == prcx) { /* first */
	prcx->pWG->pHead = prcxt->pLinkWG;
    } else { /* not first, search rest */
	for (; prcxt->pLinkWG != NULL; prcxt = prcxt->pLinkWG)
	    if (prcxt->pLinkWG == prcx) break;
	if(prcxt->pLinkWG != NULL)
		prcxt->pLinkWG = prcxt->pLinkWG->pLinkWG;
    }

    i_enable (old_int);

    if (prcx->pWG->pHead == NULL && pwg->flags & WG_DELETED)
        rcm_delete_win_geom (prcx->pProc->procHead.pGSC, pwg, &error);
}

/* unlink rcx from list bound to window attribute */

void unlink_rcx_wa (prcx)

    rcxPtr  prcx;

{

    rcxPtr  prcxt;
    int error;
    rcmWAPtr pwa = prcx->pWA;

    RCM_ASSERT(prcx != NULL, 0, 0, 0, 0, 0);

    if (prcx->pWA == NULL) return;
    if ((prcxt = prcx->pWA->pHead) == prcx) { /* first */
	prcx->pWA->pHead = prcxt->pLinkWA;
    } else { /* not first, search rest */
	for (; prcxt->pLinkWA != NULL; prcxt = prcxt->pLinkWA)
	    if (prcxt->pLinkWA == prcx) break;
	prcxt->pLinkWA = prcxt->pLinkWA->pLinkWA;
    }

    if (prcx->pWA->pHead == NULL && pwa->flags & WA_DELETED)
        rcm_delete_win_attr (prcx->pProc->procHead.pGSC, prcx->pProc,
								pwa, &error);
}


/*
 *  find_null_context - Locate the NULL context for this domain on the
 *			process' context list.
 */
rcxPtr  find_null_context (
    rcmProcPtr	    pproc,
    int 	    domain)
{
    rcxPtr	    prcxt;

    /*
     *  Find the null context for this domain.
     */
    for (prcxt = pproc->procHead.pRcx; prcxt != NULL; prcxt = prcxt->pNext)
    {
	if (prcxt->domain == domain && (prcxt->flags & RCX_NULL))
	    break;
    }

    assert (prcxt != NULL);		  /* null rcx must be in list */

    return (prcxt);
}
