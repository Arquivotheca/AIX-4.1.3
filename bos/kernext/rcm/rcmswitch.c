static char sccsid[] = "@(#)41	1.33.3.15  src/bos/kernext/rcm/rcmswitch.c, rcm, bos41J, 9520A_all 5/3/95 11:44:19";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Fault/Exception Handling
 *
 * FUNCTIONS:
 *    gp_fault		- handles graphics faults (data storage faults)
 *    gp_give_up_time_slice - relinquish remaining time in current slice
 *    gp_dispatch	- handles graphics time slice expiration
 *    rcx_switch_done	- finishes any rcx switch
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/user.h>			/* user structure */
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/trchkid.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "rcmhsc.h"
#include "xmalloc_trace.h"

/* external functions without .h files */
extern int rm_rcx_fault (devDomainPtr, rcxPtr);

BUGVDEF(dbg_rcmswitch,99);

#define DEBUG_TIME 0	    /* timer time in seconds */

/******************************************************************/
/******************************************************************
    Domain time slice - good for a fixed time slice
	- the time is in nanoseconds so for milliseconds,
	  multiply by 1000000
 ******************************************************************/
/******************************************************************/



#ifdef DEBUG
#define TRACE_FAULT_LIST(pdom, val)	trace_fault_list(pdom, val)
#else
#define TRACE_FAULT_LIST(pdom, val)
#endif



/******************************************************************/
/******************************************************************
  THIS MODULE CONTAINS FUNCTIONS OPERATE ON AN INTERRUPT LEVEL,
  THEREFORE IT MUST BE PINNED (OR SOME OF IT MUST BE)
 ******************************************************************/
/******************************************************************/

/* ============================================================= */
/* FUNCTION: gp_fault
*/
/* PURPOSE:  handles graphics faults (data storage faults)
*/
/* DESCRIPTION:
	    It determines if the fault is a valid graphics fault.
	    It determines what gp caused the fault, then
	    determines what rcx the gp believes to be
	    current. If it can switch contexts, it switches to the rcx, else
	    it links the rcx into the fault list.
*/
/* INVOCATION:
    int gp_fault (pexcp, type, tid, pmst)
	struct uexcepth     *pexcp;
	int		    type;
	tid_t		    tid;
	struct mstsave	    *pmst;
*/
/* CALLS:
	vddcheck_dev	- to check if address belongs to device
	vddstart_switch - to start a rcx switch
*/
/* DATA:
	RCM common structure and device structures, device process
	    structures
*/
/* RETURNS:
	EXCEPT_HANDLED	    if handled
	EXCEPT_NOT_HANDLED  if not handled
*/
/* OUTPUT:
	changes fault list

	can also change mstsave
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	This must be registered as an exception handler for the
	RCM; it then handles all DWA devices.
	
	The first parameter is also a pointer to the RCM common
	structure.

	Since this guy runs on an interrupt level, it must touch
	only pinned data. It can't sleep. Lists must be intact,
	which means that they must be updated with interrupts
	disabled.

	There are other things that I need to check to see if
	this is an exception I can handle besides the type parm,
	e.g., the CSR15, DSISR, and seg reg value,
	all of which are in the except array in the mstsave area
	in positions 0,1,2 respectively. The DAR is in position 3.

	CSR15 (mstsave.except[0]) contains the status of the error
	and the bus address causing the error (see p.32-36 of Rios
	I/O Arch. v2.1).

	At this time, there does not appear to be any reason to
	check the content of DSISR (in mstsave.except[1]).

	The most significant 4 bits of the bus address must come
	from the segment register (in mstsave.except[2]).

*/


int gp_fault (pexcp, type, tid, pmst)

    struct uexcepth	    *pexcp;
    int 		    type;
    tid_t		    tid;
    struct mstsave	    *pmst;

{

#define RCM_EXCP_LIMIT	3	      /* limit check fault */
#define RCM_EXCP_AUTH	5	      /* authorization fault */
#define RCM_ILLOP	1	      /* K bit fault */
#define ADD_MASK	0x0FFFFFFF    /* address mask */

#define PCOMEX	((struct _gscCom *) pexcp)  /* shorthand for pointer to a */
					     /* RCM common structure */

    int 	    domain;
    int	    	    except_code;
    rcmProcPtr	    pproc;
    gscDevPtr	    pdev;
    devDomainPtr    pdom;
    int		    rc;
    int		    old_int;

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("\n==== Enter gp_fault, pexcp=0x%x, type=0x%x\n", pexcp, type));
    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("====== gp_fault, tid=0x%x, pmst=0x%x\n", tid, pmst));
    gsctrace (GP_FAULT, PTID_ENTRY);

    /*
     *  Make sure we are on the proper processor.
     */
    if (__power_mp () && !am_i_funnelled ())
	return (EXCEPT_NOT_HANDLED);

    /*
     *  We need to be able to distinguish and isolate the graphics faults
     *  with as few tests as possible, so that we don't have to search for
     *  pid/tid match in the RCM tables if the process is not graphical.
     *  This may require just the right kinds of tests on type, and may
     *  also imply the need for new, more appropriate types generated by
     *  the kernel user mode exception handler.
     *
     *  If the type of exception is proper, then we investigate the details
     *  of the fault in order to exclude, if possible, all states that are
     *  not to be legitimately handled by the RCM.  This will involve error
     *  code checking on the status words in the mst.  These checks may be
     *  bus dependent.  If this is true, then the bus type and bus id need to
     *  be extracted in the process, also.
     *
     *  If the type and status of the exception renders it most likely that
     *  it is a legitimate graphics fault, then we must search the RCM tables
     *  in order to proceed further.  The device dependent rcmprocess struc-
     *  tures are scanned looking for a match on pid/tid and bus id.  If one
     *  is found, then the check_dev function for that head is called.  The
     *  check_dev function determines if the fault effective address is within
     *  one of its protection domains.  If so, it returns the domain number.
     *  If not, it returns an error.  In the latter case, the domain search is
     *  continued in the same manner on the other devices.  If no device is
     *  found, the graphical fault is returned as if it were not a graphical
     *  fault.  (If a software error has occurred then this will cause a
     *  BUS_ERROR in the user process).
     */

    /* if fault type not one we can handle, return not handled */
    if (type != EXCEPT_IO_IOCC &&
	type != EXCEPT_IO_SGA  &&
	type != EXCEPT_GRAPHICS_SID)	/* Processor bus fault */
    {
	BUGLPR(dbg_rcmswitch,BUGACT, ("====== gp_fault... not my type\n"));
	return (EXCEPT_NOT_HANDLED);
    }

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("====== gp_fault, ex[0]=0x%x, ex[1]=0x%x\n",
	    pmst->except[0], pmst->except[1]));
    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("====== gp_fault, ex[2]=0x%x, ex[3]=0x%x\n",
	    pmst->except[2], pmst->except[3]));

    /*
     *  Check for IOCC error we handle.  PowerPC has no limits check.
     *
     *  Right now, I think that all faults have a pseudo-IOCC error
     *  vector structure, even if the fault was not generated by an IOCC.
     */
    except_code = pmst->except[0] >> 28;	/* real or pseudo csr15 */

    if (__power_pc ())
    {
	if (type != EXCEPT_GRAPHICS_SID)		/* IOCC gen'd fault */
	{
	    if (except_code != RCM_EXCP_AUTH &&	/* bus mem auth fault */
	        except_code != RCM_ILLOP        )	/* K bit protection */
	    {
		BUGLPR(dbg_rcmswitch,BUGACT,
				("====== gp_fault... not my csr type\n"));
		RCM_ASSERT (0, pmst->except[0], pmst->except[1],
			   pmst->except[2], pmst->except[3], 0);
		return (EXCEPT_NOT_HANDLED);
	    }
	}
	else						/* PPC proc bus fault */
	{
	    /* no known error code checks to make */
	}
    }
    else
    {
	if (except_code != RCM_EXCP_LIMIT &&	/* bus I/O fault, or ... */
	    except_code != RCM_EXCP_AUTH     )	/* bus mem auth fault */
	{
	    BUGLPR(dbg_rcmswitch,BUGACT,
				("====== gp_fault... not my csr type\n"));
	    RCM_ASSERT (0, pmst->except[0], pmst->except[1],
			   pmst->except[2], pmst->except[3], 0);
	    return (EXCEPT_NOT_HANDLED);
	}
    }

    /* ---------------------------------------------------------------------
        The following section rewritten for defect 64846. 

        Ensure this fault comes from a graphics process:  this is a nested
	loop procedure.  The outer loop examines all the graphics devices.
        The inner loop checks all the graphics processes (per g-device).
        Assuming we find a match, we call the device dependent address range
        check routine (vdd_check_dev). 
     * --------------------------------------------------------------------- */

    /* disable interrupts (timer mostly) so lists can't get corrupted */
    old_int = i_disable(INTMAX);

    BUGLPR(dbg_rcmswitch, 2, ("====== gp_fault... address=0x%x\n",
					   pmst->except[3] & ADD_MASK));

    /* --------------------------------------------------------------------- *
       Scan all (pdev, pproc) pairs for matching process id in the rcm process
       structures.  There will be more than one match for multihead scenarios.

       There cannot be more than one rcm process structure for the same process
       attached to any given pdev.  If there is more than one rcm process struc-
       ture for the process, each structure will be attached to a different
       pdev structure.  (There is a pdev structure for each virtual terminal
       which has a registered graphics process.)

       A further (and critical) limitation on multihead scenarios is that the
       screens must all appear on separate physical devices, not just different
       virtual terminals (which could be on the same physical device).  This
       restriction is enforced by the way multihead scenarios are started.  You
       cannot start a multihead scenario with two of the screens set to appear
       on two different virtual terminals on the same physical device.  All
       screens are visible when the multihead scenario first comes up.

       Since each match on process id found in the search of all rcmprocess
       structures can only occur on separate physical devices, and since
       separate physical devices have nonoverlapping bus memory addresses,
       the vddcheck_dev function may be used to resolve which (pdev, pproc) pair
       is correct.  This function also returns the domain.  This completely
       defines the pdev, pproc, and pdom pointers.

       As noted above:
       This procedure can not distinguish two different rcm process structures
       for the same process on two different pdev structures (virtual terminals)
       on the SAME PHYSICAL DEVICE.  But this multihead scenario cannot be
       started.  "Multihead", by definition, means separate physical devices.
     * -------------------------------------------------------------------- */
    for (pdev = PCOMEX->pDevList; pdev != NULL; pdev = pdev->devHead.pNext) 
    {
	/*
	 *  Make sure we are the right bus.  The way we do this now (un-
	 *  fortunately) is to compare fault parameter 'type' with what
	 *  has been set up by make-gp.  There are only two types: EXCEPT_IO,
	 *  and GRAPHICS_SID.  These two types are to distinguish between
	 *  the Microchannel and the processor bus (for Magenta).
	 */
	if (pdev->devHead.fault_type != type)
	    continue;

	/* scan the rcm process structures on this device for a tid match */
    	for (pproc = pdev->devHead.pProc; pproc != NULL;
	     pproc = pproc->procHead.pNext)
	{
	    if (pproc->procHead.tid == tid) 
	    {
		/* ---------------------------------------------------------- *
		   Now check the address range.  The DD routine returns a domain
		   as a return code.  We then check if the returned domain is in
		   the adapter's domain range (i.e., is valid).  The domain will
                   be valid when the correct device is touched.

		   If domain invalid, then this physical device wasn't being
		   addressed by the fault.  In that case, we immediate skip to
		   the next pdev structure (since no more matches on tid will
		   be found on this one).
		* ---------------------------------------------------------- */
		if (pdev->devHead.display->check_dev != NULL)
		{
		    domain = (pdev->devHead.display->check_dev)
	 		(pdev, pmst->except[3] & ADD_MASK, pproc->procHead.pid);

		    if ( (domain >= 0 && domain < pdev->devHead.num_domains) )
			goto FOUND_GP ;
		}

		break;	/* no more tid match on pproc loop.  go next pdev */
	    }  /* end of tid match */
        }    /* end of inner (GP) loop */
    }   /* end of outer (pdev) loop */

    i_enable (old_int);

    RCM_ASSERT (0, pmst->except[0], pmst->except[1],
		   pmst->except[2], pmst->except[3], 0);

    /* If we complete the loop, we have not found a match */
    BUGLPR(dbg_rcmswitch,BUGACT, ("====== gp_fault... no GP (tid) match\n"));
    return (EXCEPT_NOT_HANDLED);


 FOUND_GP:

    /* ---------------------------------------------------------------------
        End of fault checks and end of section rewritten for 64846. 
     * --------------------------------------------------------------------- */


    /* find domain */
    pdom = &pdev->domain[domain];  /* pointer to domain that faulted */

    RCM_TRACE(0x100,tid,pdom,pproc);
    RCM_TRACE(0x101,pmst->except[0],pmst->except[1],pmst->except[3]);

#ifdef DEBUG
    /* if the faulting gp faulted because the domain is suspended */
    if ( (pproc == pdom->pCurProc) && (pdom->flags & DOMAIN_SUSPENDED) ) {
	/* everything is OK */
    } else {
	RCM_ASSERT(pproc != pdom->pCurProc, 0, 0, 0, 0, 0);
	    /* faulting process should not own domain !! */
    }

	trchkgt(HKWD_DISPLAY_RCM_D | (GP_FAULT & 0xff) << 4 | 2,
		0x10,
		pdom->flags & DOMAIN_LOCKED,
		pdom->flags & DOMAIN_DEV_LOCKED,
		pdom->flags & DOMAIN_BLOCKED,
		pdom->flags & DOMAIN_SWITCHING);

	trchkgt(HKWD_DISPLAY_RCM_D | (GP_FAULT & 0xff) << 4 | 2,
		0x11,
		pdom->flags & DOMAIN_TIMER_ON,
		pdom->pLockProc,
		pproc,
		pdom->pFault);
#endif

    /* fault_handler is entered with interrupts OFF! */
    rc = fault_handler (pdev, pproc, pdom, domain, old_int);
    /* fault_handler returns with interrupts restored to old_int! */

    gsctrace (GP_FAULT, PTID_EXIT);

    return (rc);
}


    /************************************************************************** 

                		START of PROLOGUE 

        Function:  gp_put_on_fault_list

        Descriptive name:  Put context back on fault list 

     *----------------------------------------------------------------------*  

        Function:  
           This routine places the context back on the fault list. 

        Notes:     
	   This is currently only called by the ped driver in the interrupt
	   environment.  It is called as a consequence of the adapter having
	   just been switched away from the context being reenqueued.

     *************************************************************************/

gp_put_on_fault_list	(rcxPtr pRCX)
{
	devDomain	*pDom ;
	int 		old_int;
	int 		rc;
	rcmProc		*pProc ; 



    /*-------------------------------------------------------------------------
        Echo the parms 
     *-----------------------------------------------------------------------*/
    	BUGLPR(dbg_rcmswitch, 1, 
		("====== top of gp_put_on_fault_list:  pRCX =  \n", pRCX ));


    /*-------------------------------------------------------------------------
        Disable interrupts 
     *-----------------------------------------------------------------------*/
	old_int = i_disable(INTMAX);

    /*-------------------------------------------------------------------------
        Get the Domain 
     *-----------------------------------------------------------------------*/
	pDom = pRCX -> pDomain ; 
        BUGLPR(dbg_rcmswitch, 2, ("Domain = 0x%X\n", pDom));

    /*-------------------------------------------------------------------------
        Put the context back onto the fault list.  Do not block.
     *-----------------------------------------------------------------------*/

	RCM_TRACE (0x170, getpid (), pDom, pRCX);

	rcx_fault_list (pDom, pRCX, 0) ; 

	/*
	 *  This is a quasi-fault.  Currently, the fault handler does its
	 *  own custom dispatching, and doesn't call dispatch_dom.  We
	 *  may do the same.  That is, we may use fault_handler or dispatch_dom
	 *  as models of things to do at this time.  Notice, however, that those
	 *  functions are most often proactively following user process re-
	 *  quests for action.  This we do not do.  This reenqueue means that
	 *  more work needs to be done by the adapter for this context, but we
	 *  should not force it to be done immediately.
	 */

	/*
	 *  If the reenqueue puts us elsewhere than the top of the fault
	 *  list, then we have changed nothing significant for domain dis-
	 *  patching.  We only need to take special action if our reenqueue
	 *  has put us at the top of the fault list.
	 */
	if (pDom->pFault == pRCX)
	{
		/*
		 *  We depend on antithrashing code elsewhere in the rcm to
		 *  prevent thrashing due to this reenqueued context.  Our task
		 *  here is to make sure that switch-on of this context will
		 *  eventually take place, and not end up being stalled
		 *  indefinitely (however unlikely that might be).
		 *
		 *  Therefore, if no further passes through dispatch_dom seem to
		 *  be scheduled (no timeslice, lock, block, switch, etc. in
		 *  process), then institute a timeslice on the context current
		 *  on the domain in order to force a pass through dispatch_dom
		 *  after a reasonable interval of time.  This timeslice is
		 *  unconditional, whether the context on the domain has ever
		 *  enjoyed one or not.
		 */

    		/*
    		 *  If the domain is switching, a context is already being
		 *  dispatched.  This will cause a pass through dispatch_dom.
		 *
		 *  If the domain is blocked, do nothing.  A pass through
		 *  dispatch_dom will occur at unblock time.
		 *
		 *  If the timer is on, do nothing.  A pass through dispatch_
		 *  dom will occur when the timer pops.
    		 */
   		if (pDom->flags &
			(DOMAIN_SWITCHING | DOMAIN_BLOCKED | DOMAIN_TIMER_ON))
    		{
		    RCM_TRACE (0x171, pProc->procHead.pid,
						pDom->flags, pDom->guardlist);
    		}
		else
		/*
		 *  If the process with the current context has the domain	
		 *  fast locked, then we do nothing.  A pass through dispatch_
		 *  dom will eventually occur.
		 *
		 *  Currently, this is not applicable, since reenqueue and GTO
		 *  are mutually exclusive.
		 */
		if (ck_fast_dom_lock (pDom->pCur) &&
		    !(pDom->flags & DOMAIN_SUSPENDED))
		{
			RCM_TRACE (0x172, pProc->procHead.pid, pDom, 0);
		}
		else
		/*
		 *  Nothing obvious is going to cause a timely pass through
		 *  dispatch_dom.  Start a timeslice on the context currently
		 *  on the domain, WHETHER IT HAS HAD ONE ALREADY OR NOT.
		 */
		{
			rcxPtr   pold;

			pold = pDom->pCur;

			/*
			 *  We know at this point that no locks, blocks, or
			 *  timeslices are in effect, nor is any process waiting
			 *  to guard the domain, therefore we are able to
			 *  institute a timeslice.
			 */
			start_time_slice (pDom, pold);
		}
	}

    /*-------------------------------------------------------------------------
        Enable interrupts 
     *-----------------------------------------------------------------------*/
	i_enable (old_int);

    	BUGLPR(dbg_rcmswitch, 1, ("Bottom of gp_put_on_fault_list \n" ));
	return 0;
}



/*****************************************************************************
			START OF PROLOGUE

    Routine:  fault_handler - Graphics Fault handler part 2 


 *-------------------------------------------------------------------------

    Function:
       By the time this routine is called, we have already determined
       that a valid graphics fault has occurred.  This routine determines
       whether we can switch the context onto the domain or if some other
       action must occur.  These various "other actions" include: 
         . putting the context on the fault list or
         . blocking the process 

 *-------------------------------------------------------------------------

    Notes: 
      1. fault_handler is entered with interrupts OFF! 

******************************************************************************/

fault_handler(pdev, pproc, pdom, domain, old_int)
rcmProcPtr	    pproc;
gscDevPtr	    pdev;
devDomainPtr        pdom;
int 		    domain;
int		    old_int;
{
    int 	    can_switch;       /* can switch (=1), or must put on */
				      /* fault list (=0) */
    int  ourrcx, ourselctrcx, ourlock, fastlocked, switching;

    /* fault_handler is entered with interrupts OFF! */
    RCM_TRACE(0x104, pdom->pCur, pproc->pDomainCur[0], pproc->procHead.flags);

    /************************************************************************** 
        The following section checks if the device driver has requested that
        the process be blocked.  If it has, then the process will, in fact,
        be blocked.  

	Note that it is the resposibility of the device driver
        to request the unblock. 

	There is special UNUSED example code in 'unmake-gp' which could be used
	to clear the kind of block which is set here.  That code is not used
	for the reasons detailed in 'unmake-gp'.  However, if that code were
	used, it requires that the code in this place NOT_HANDLE unexpected
	faults from a process which is supposed to be uexblocked because of
	gp_block_gp.  It was observed that this code to NOT_HANDLE such a
	fault is convenient to be left active, here, even though the uexclear
	operation is never done when taking the process down in 'unmake-gp'.
     *************************************************************************/ 

    if (pproc->procHead.flags & PROC_DD_BLOCK_REQUEST)
    {
        RCM_TRACE(0x105,pproc->procHead.pid,pdom,pproc->procHead.flags);

        /*-----------------------------------------------------------------
                Set flag indicating the block was actually done. 
        *---------------------------------------------------------------*/
	/*
	 *  If we get a spurious reentry, then tell the kernel we didn't
	 *  handle the fault.  This will make the process crash.
	 *
	 *  If unmake-gp ever does a uexclear (not currently planned or
	 *  needed), this code here might really be useful.  The process
	 *  receiving the kill signal from unmake-gp (and being uexclear'd by
	 *  that code, if it had been blocked by gp_block_gp + actual fault)
	 *  might erroneously fault again.  In that case we protect ourselves
	 *  against unwanted faults or fault loops by just not handling the
	 *  fault.
	 */
	if (pproc->procHead.flags & PROC_PROCESS_BLOCKED)
	{
	    i_enable (old_int);

	    return (EXCEPT_NOT_HANDLED);
	}

	/* set up the blockage */
        RCM_TRACE(0x106,pproc->procHead.pid,pdom,pproc->procHead.flags);

        pproc->procHead.flags |= PROC_PROCESS_BLOCKED ; 

	uexblock (pproc -> procHead.tid);

	/*
	 *  Begin special code for bos325, only, for handling the case
	 *  where a process has scheduled high-water mark handling on
	 *  the next fault, has been switched off the domain, locks the
	 *  adapter (with someone else's context on it), then tries to
	 *  touch the adapter.  This causes a high-water mark block to
	 *  take effect here.  Since the wrong context is on the domain
	 *  and since the adapter is locked, the proper reenqueued high
	 *  water marked context will never be switched back on unless
	 *  explicit action is taken by the fault handler.  That action
	 *  is taken here.
	 *
	 *  The follow code is a specialization of the general fault
	 *  handler code below.  This was done to avoid too many changes
	 *  to the fault handler late in the bos325 test process.  The
	 *  general solution is available and will be dropped in future
	 *  releases.
	 */
	ourselctrcx = (pdom->pCur == pproc->pDomainCur[domain]);
	ourlock     = (pproc == pdom->pLockProc);

	if (pdom->flags & DOMAIN_BLOCKED && ourlock && !ourselctrcx)
	{
	    rcx_switch (pdom->pCur, pproc->pDomainCur[domain],
						RCX_SWITCH_QUEUE, old_int);
	}
	else
	{
	    i_enable (old_int);
	}

	return (EXCEPT_HANDLED);
    }

    /**************************************************************************
     *
     *  Handle graphics fault when the above roadblock is not called for.
     *
     *************************************************************************/ 

    /*
     *  The order of priority for determining who should own the domain
     *  is as follows:
     *
     *  1)  If the domain is switching, then we must go on the fault list,
     *      unless the context being switched on is our selected context.
     *      In that case we, we block ourselves and set our state so that
     *      rcx_switch_done will unblock us when the switch is done.
     *
     *  2)  Domain blocks (not uexblock) in effect on the domain, or fast-
     *      domain-lock in effect on the context on the domain, imply owner-
     *      ship by the one who performed the block.  Fast domain lock is
     *      looked at first, followed by standard domain blocks.
     *	
     *	    If the one blocking is someone else, we must go on the fault
     *	    list.  If the block is ours, we may have the domain.  We
     *      institute an immediate switch to our selected context, or just
     *      reenable bus access, if the selected context is already on.
     *
     *      NOTE:  A fast domain lock setting on a context is a lock REQUEST,
     *      if the domain is suspended.  The time slice has been given up, and
     *      it is as if no lock were in effect, yet.  The fast lock request
     *      is GRANTED when that context gets switched on the domain.
     *
     *  3)  Domain clock on (time slices unexpired) imply ownership by
     *	    the process owning the context on the domain.
     *
     *	    If the owner is someone else, we must go on the fault list.
     *      If the owner is us, we should ignore the time slice, since we
     *      are single threaded.  In the latter case we institute an immediate
     *      switch, or reenable domain access, according to the selected rcx.
     *
     *  4)  Check to see if processes wishing to guard the domain have been
     *      made ready to execute (and haven't acquired the domain lock), AND
     *      the gsc_give_up_timeslice call has been made.  This is to avoid
     *      monopoly by a single process.  If such a condition exists, regard
     *      the processes waiting to guard the domain as being higher priority.
     *      Go on the fault list.
     *
     *  5)  If no domain guarders are waiting to guard, then check the fault
     *      list.  If there are contexts on the list, then line up in priority
     *      order on the fault list.
     *
     *  6)  We have the domain to ourselves.  Institute an immediate switch,
     *      or reenable domain access, according to the selected rcx.
     */

    /*
     *  We set up local variables by which we ascertain the domain state.
     *  If the domain is switching, the pCur and pCurProc members pertain
     *  to the new context being switched on.
     *
     *  ourrcx      - Owned by us.
     *  ourselctrcx - Our selected context.
     *  ourlock     - Is the domain locked by us?
     *  fastlocked  - Is the fast domain lock set (been granted)?
     *  switching   - Is the domain switching?
     *
     *  NOTE:  A fast domain lock setting is a request only, if the domain
     *  is suspended.  If the time slice has been given up (or never started),
     *  and if the domain is suspended (or the context with fast-lock request
     *  has never been switched on the domain), then the fast domain lock is
     *  a request only.  It only becomes an actual lock when the context with
     *  the fast lock bit set is switched onto the domain and is NOT suspended.
     */
    switching   = (pdom->flags & DOMAIN_SWITCHING);
    ourrcx      = (pproc == pdom->pCurProc);
    ourselctrcx = (pdom->pCur == pproc->pDomainCur[domain]);
    ourlock     = (pproc == pdom->pLockProc);
    fastlocked  = (!(pdom->flags & DOMAIN_SUSPENDED) &&
			ck_fast_dom_lock (pdom->pCur));

    /*
     *  If the domain is switching, then we normally just go on the fault list.
     *  However, if the context being switching on is our selected context, then
     *  we don't want to make a new entry on the fault list.  We must block
     *  ourselves to wait for the switch to finish, and arrange for rcx_switch_
     *  done to unblock us.
     */
    if (switching)
    {
	RCM_TRACE (0x710, pproc->procHead.pid, pdom, pdom->flags);

	if (ourselctrcx)
	{
	    RCM_TRACE (0x711, pproc->procHead.pid, pdom, pdom->flags);

	    pproc->pDomainCur[domain]->flags |= RCX_BLOCKED;

	    uexblock (pproc->procHead.tid);

	    i_enable (old_int);

	    RCM_TRACE (0x761, pproc->procHead.pid, pdom, pdom->flags);

	    return (EXCEPT_HANDLED);
	}

	can_switch = 0;
    }
    else
    /*
     *  The domain is not switching.
     *
     *  If the user space context lock is set, and the context is not
     *  suspended, then the lock is in force.  It can't be ours.
     *  We must go on the fault list.
     */
    if (fastlocked)
    {
	RCM_TRACE (0x712, pproc->procHead.pid, pdom, pdom->flags);

	RCM_ASSERT (!ourrcx, 0, 0, 0, 0, 0);	/* can't be ours */

	can_switch = 0;			/* fault list */
    }
    else
    /*
     *  The domain is not switching and the context is not fast-locked onto
     *  the domain with a 'granted' fast lock request.
     *
     *  If the domain is blocked, act according to the ownership of block.
     */
    if (pdom->flags & DOMAIN_BLOCKED)
    {
	RCM_TRACE (0x713, pproc->procHead.pid, pdom, pdom->flags);

	RCM_ASSERT (pdom->pLockProc, 0, 0, 0, 0, 0);

	can_switch = ourlock;
    }
    else
    /*
     *  The domain is not switching, and is not fast-locked, AND
     *  is not blocked.
     *
     *  If the domain time slice has not expired, act according
     *  to ownership.
     */
    if (pdom->flags & DOMAIN_TIMER_ON)
    {
	RCM_TRACE (0x714, pproc->procHead.pid, pdom, pdom->flags);

	RCM_ASSERT (pdom->pCurProc, 0, 0, 0, 0, 0);

	can_switch = ourrcx;
    }
    else
    /*
     *  The domain is not switching, and is not blocked, AND the domain
     *  timer is off.
     *
     *  Give precedence to those processes that are waiting to
     *  try to guard the domain.
     */
    if (pdom->flags & DOMAIN_GUARD_PENDING)
    {
	RCM_TRACE (0x715, pproc->procHead.pid, pdom, pdom->flags);

	can_switch = 0;				/* give them a chance */
    }
    else
    /*
     *  If there are processes on the fault list, line up behind
     *  them.
     */
    if (pdom->pFault)
    {
	RCM_TRACE (0x716, pproc->procHead.pid, pdom, pdom->flags);

	can_switch = 0;
    }
    else
    /*
     *  We are at liberty to use the domain.
     */
    {
	RCM_TRACE (0x717, pproc->procHead.pid, pdom, pdom->flags);

	can_switch = 1;			/* switch or reconnect bus */
    }

    /*
     *  Now branch on the can_switch flag.
     *
     *  Case 1:  can_switch == 0 -> go on fault list.
     *  Case 2:  can_switch != 0 -> switch to new context, or reenable bus
     */
    if (can_switch)				/* start switch */
    {
	if (!ourselctrcx)			/* may not even be ours! */
	{
	    rcxPtr  pnew;

	    RCM_TRACE (0x730, pproc->procHead.pid, pdom, pdom->flags);

	    /*
	     *  Set up to start the switch.  Mark the process to
	     *  be blocked when it returns to user space.
	     *  If this turns out to be a light switch to a context for
	     *  the same process, then rcx_switch_done (which will be
	     *  executed before returning here) will perform the uexclear
	     *  so that the process will never be held up!
	     */
	    pnew = pproc->pDomainCur[domain];

/**//* this uexblock/uexclear could be optimized for light switches */
	    pnew->flags |= RCX_BLOCKED;
	    uexblock (pproc->procHead.tid);

	    /*
	     *  Switch to the new context.  Finish if it is a light switch.
	     *  Queue any heavy switch to the heavy switch controller.  In
	     *  that case, rcx_switch_done will be done by the heavy switch
	     *  controller.
	     *
	     *  Rcx_switch/rcx_switch_done will not touch the domain timer
	     *  unless we are switching between contexts belonging to different
	     *  processes, AND we are under no domain blocks.  In that case, the
	     *  timer will be stopped before the switch, and a new timeslice
	     *  will be started by rcx_switch_done.  Else, the domain timer
	     *  may be running, and may expire at any time.
	     *
	     *  Fast domain lock does not apply here, since we do not switch
	     *  while that lock is in effect.
	     *
	     *  Reenable interrupts.
	     */
	    rcx_switch (pdom->pCur, pnew, RCX_SWITCH_QUEUE, old_int);
	}
	else				/* reenable bus access */
	{
	    RCM_TRACE (0x740, pproc->procHead.pid, pdom, pdom->flags);

	    /*
	     *  Reestablish process access to the domain.
	     *  Reenable interrupts.
	     *
	     *  NOTE:  We can come here because the fault was triggered
	     *  by high-water-mark, but the path from user to fault handler
	     *  was interrupted by the low-water-mark interrupt, thus
	     *  making the state of the rcm look like there was no visible
	     *  cause for the fault having occured.  I.e., there will be
	     *  no state flags which indicate that the rcm suspended the
	     *  domain.
	     */
	    pdom->flags &= ~DOMAIN_SUSPENDED;
	    fix_mstsave (pproc, pdom, FIX_MSTSAVE_ADD);

	    i_enable (old_int);
	}
    }
    /*
     *  We will defer this fault to the fault list.
     */
    else
    {
	RCM_TRACE (0x750, pproc->procHead.pid, pdom, pdom->flags);

	/*
	 *  If the domain is blocked, AND if the clock is off, then
	 *  send feedback to the processes which have the locks.
	 */
	if (!(pdom->flags & DOMAIN_TIMER_ON))
	{
	    /*
	     *  If the timer is off, then feedback a request for
	     *  fast domain unlock/give-up-timeslice call from app.
	     */
	    if (fastlocked)
	    {
	        RCM_TRACE (0x712, pproc->procHead.pid, pdom, pdom->flags);

	        sig_fast_dom_lock (pdom->pCur, 1);
	    }

	    /*
	     *  If this is a syscall domain block, and if the timer is off,
	     *  then signal a request for domain unlock.
	     */
	    if (pdom->flags & DOMAIN_LOCKED)
	    {
	        RCM_TRACE (0x714, pproc->procHead.pid, pdom, pdom->flags);

	        pidsig (pdom->pLockProc->procHead.pid, SIGURG);
	    }
	}

	/*
	 *  Put this context on the fault list.
	 *
	 *  This process will be blocked when it exits the fault handler.
	 */
	rcx_fault_list (pdom, pproc->pDomainCur[domain], RCX_FAULT_BLOCK);

	i_enable (old_int);
    }

    /* interrupts should be on at this point */
    RCM_ASSERT (i_disable (old_int) == old_int, 0, 0, 0, 0, 0);

    RCM_TRACE (0x767, pproc->procHead.pid, pdom, pdom->flags);

    return (EXCEPT_HANDLED);
}

	
/* ============================================================= */

/* FUNCTION:  gp_give_up_time_slice
*/
/* PURPOSE:   Allow device driver to indicate process is sleeping and
	      can relinquish its remaining time.
*/
/* DESCRIPTION: When the device driver puts a graphic process to sleep,
	      it uses this function to tell the RCM that it can dispatch
	      any waiting process.
*/
/* INVOCATION:
	      void gp_give_up_time_slice(rcxPtr prcx)
*/
/* CALLS:
	      tstop - to stop the timer (if one is running).
	      rcx_dispatch - to dispatch waiting process.
*/
/* DATA:
	      RCM structure
*/
/* RETURNS:
	      none
*/
/* OUTPUT:
	      none
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	      Must be called with interrupts disabled.
		It is assumed the domain isn't guarded.
		This function can be called in the interrupt environment.
*/
void 
gp_give_up_time_slice(devDomainPtr pdom, rcmProcPtr pproc)
{
	RCM_ASSERT (i_disable (INTMAX) == INTMAX, 0, 0, 0, 0, 0);

	RCM_TRACE(0x263,getpid(),pdom,pproc);

	BUGLPR(dbg_rcmswitch,BUGNFO,
		("==== Enter gp_give_up_time_slice, pdom=0x%x pproc=0x%x\n",
		pdom, pproc));
	gsctrace (GP_GIVEUP_TS, PTID_ENTRY);
	
	if ((pdom->flags & DOMAIN_SWITCHING) ||
	    pdom->pCurProc != pproc)
	{
		gsctrace (GP_GIVEUP_TS, PTID_EXIT);
		return; 	/* not currently owner of domain */
	}
	
	/*
	 *  GEMINI paradigm was:  If the fast domain lock is clear, clear any
	 *  feedback request flag.  The code was simplified, since the fast
	 *  domain lock is ALWAYS clear in the GEMINI case.  Why even the
	 *  feedback bit needs clearing is unknown.  Was this a kloodge to
	 *  cover some protocol error on the part of the app/library?
	 *
	 *  We will leave the sig_fast_dom_lock to clear any feedback
	 *  request (even though the app should clear this) so that we
	 *  won't have premature recall of 'give-up-time-slice'.
	 */
	RCM_ASSERT (ck_fast_dom_lock (pdom->pCur) == 0, 0, 0, 0, 0, 0);

	/*  Must read the area before you can write it back */
	ck_fast_dom_lock (pdom->pCur);
	sig_fast_dom_lock (pdom->pCur, 0);

	/*
	 *  Turn off any timer to end timeslice.
	 */
	if (pdom->flags & DOMAIN_TIMER_ON)
	{
		tstop(pdom->pDevTimer);
		pdom->flags &= ~DOMAIN_TIMER_ON;
	}
	
	/*
	 *  Find the next user of the domain, if any.
	 */
	dispatch_dom (pdom);
	
	BUGLPR(dbg_rcmswitch,BUGNFO,
	("==== Exit gp_give_up_time_slice... rcx for domain now=0x%x\n",
		pdom->pCur));

	gsctrace (GP_GIVEUP_TS, PTID_EXIT);
}


/* ============================================================= */
/* FUNCTION: gp_make_cur_and_guard_dom
*/
/* PURPOSE:   switches on requested rcx and keeps it there
*/
/* INVOCATION:
    void gp_dispatch (prcx)
	rcxPtr prcx;
*/

gp_make_cur_and_guard_dom (prcx)
rcxPtr prcx;
{
    rcmProcPtr pproc = prcx->pProc;
    devDomainPtr pdom;
    int old_int;

    RCM_TRACE(0x350,getpid(),prcx,0);

    /* disable interrupts */
    old_int = i_disable (INTMAX);

    /* set domain */
    pdom = prcx->pDomain;

    /*
     *  We used to kill the domain timer here if the rcx current on
     *  the domain is owned by our process (or no one).  This was
     *  to avoid having the guard_dom call possibly hang for timer
     *  expiration.  However, inspection of the guard_dom code
     *  shows that the guard operation would never wait in this
     *  case.  Therefore, the timer kill has been removed.  Note:
     *  This code is very similar to the gsc_set_rcx code.
     */

    /* guard domain to make sure that don't try to start a switch
	   while one is going on */
    guard_dom (pdom, pproc, 0, GUARD_ONLY | GUARD_NO_ENABLE);

    /*
     *  Always force the desired context onto the domain, unless
     *  it is already there.
     */
    if (pdom->pCur != prcx)
    {
	/*
	 *  Switch to the new context.  Perform any heavy switch in-line.
	 *  Do not queue it to the Heavy Switch Controller.
	 *
	 *  Rcx_switch/rcx_switch_done will not touch the domain timer
	 *  under these circumstances, since we are switching under a
	 *  domain block.  However, the timer may be running, and may
	 *  expire at any time.
	 *
	 *  Fast domain lock does not apply here, since we do not
	 *  switch while that lock is in effect.
	 *
	 *  Reenable interrupts.
	 */
        rcx_switch (pdom->pCur, prcx, RCX_SWITCH_DOIT, old_int);
    }
    else
	i_enable (old_int);

    /* leave domain guarded */
    return 0;
}


/* ============================================================= */
/* FUNCTION: gp_dispatch
*/
/* PURPOSE:   handles graphics time slice expiration
*/
/* DESCRIPTION:
	    It determines if a rcx can be switched and if so,
	    starts the switch activity.
*/
/* INVOCATION:
    void gp_dispatch (ptimer)
	struct	trb	*ptimer;
*/
/* CALLS:
	vddstart_switch - to start a rcx switch
*/
/* DATA:
	RCM/DDH structure

	process table entry and ublock (mstsave)
*/
/* RETURNS:
	none
*/
/* OUTPUT:
	changes fault list
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Assumes there is a timer for each domain. Each timer
	structure must be initialized at the time of device
	creation.
	
	Assumes that cannot get preempted by process and
	therefore does not need to disable interrupts.

	Assumes that cannot get called if a domain is
	switching because the timer would be expired
	before the start of the switch and not restarted
	until after the switch. HOWEVER, it is possible to
	be called when the domain is guarded; under those
	circumstances, must not dispatch a rcx.

	Assumes that cannot get called if a domain is
	locked because the timer would not be started
	for a locked device or domain.

	There are 2 ways to get on fault list:
	1) real fault - write to domain without
	   authorization; put on list by fault handler
	2) logical fault - take some action that forces
	   a switch, e.g., set_rcx

	The amount of time a process has really run may be
	something that is hard to determine, and the precision
	may not be good enough from existing kernel functions.
	It is not clear what field(s) we can really check at
	this time; something in the accounting and profiling
	data in the ublock (user.h) might work.
*/

void gp_dispatch (ptimer)

    struct	trb	*ptimer;

{
    devDomainPtr    pdom;
    rcmProcPtr	    pproc;
    int             pid = -1;
    int		    old_int;

    pdom = (struct _devDomain *) ptimer->t_func_addr;
    pproc = pdom->pCurProc;
    if (pproc)					/* is it real? */
	pid = pproc->procHead.pid;

    gsctrace (GP_DISPATCH, PTID_ENTRY);

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("\n==== Enter gp_dispatch, ptimer=0x%x domain=0x%x\n",
	    ptimer, pdom));

    /*
     *  There are no cases now where we wish to extend a time slice
     *  that has expired, but if such cases arise, the code could be
     *  put here.
     */
    old_int = i_disable (INTMAX);

    RCM_TRACE(0x110, pid, pdom,ptimer);

    pdom->flags &= ~DOMAIN_TIMER_ON;	/* expired */

    /*
     *  Find the next user of the domain, if any.
     */
    dispatch_dom (pdom);

    i_enable (old_int);

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("==== Exit gp_dispatch... rcx for domain now=0x%x\n\n",
	    pdom->pCur));

    gsctrace (GP_DISPATCH, PTID_EXIT);
}


/* ============================================================= */
/* FUNCTION: rcx_switch_done
*/
/* PURPOSE:  finishes rcx switches for domain
*/
/* DESCRIPTION:
     It performs the necessary actions to finish a switch.
     It may be started by the Heavy Switch Controller.
     It consists of ensuring domain authorization and waking
     up the graphics process.
*/
/* INVOCATION:
    void rcx_switch_done (pdom, prcx, switches)
	devDomainPtr	pdom;
	rcxPtr		prcx;
        ulong		switches;

    This function must be called with interrupts disabled.
*/
/* CALLS:
*/
/* DATA:
	RCM/DDH structure
*/
/* RETURNS:
	none
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/



void rcx_switch_done (pdom, prcx, switches)

    devDomainPtr	pdom;
    rcxPtr		prcx;
    ulong		switches;

{
    rcmProcPtr	pproc = pdom->pCurProc;
    int         domain = prcx->domain;
    rcxPtr	psel;
    int         old_int;

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("\n==== Enter rcx_switch_done... domain=0x%x, rcx=0x%x\n",
	    pdom, prcx));

    gsctrace (RCX_SWITCH_DONE, PTID_ENTRY);

    if (!(switches & RCX_SWITCH_NO_ENABLE))
	old_int = i_disable (INTMAX);
    else
    {
	RCM_ASSERT (i_disable (INTMAX) == INTMAX, 0, 0, 0, 0, 0);
    }

    RCM_ASSERT (prcx->pProc == pdom->pCurProc, 0, 0, 0, 0, 0);

    RCM_TRACE(0x120,getpid(),prcx,0);

    /* clear domain switching flag */
    pdom->flags &= ~DOMAIN_SWITCHING;

     /*---------------------------------------------------------------
     Check if this context is the one that the process wants active.
     If so, we will grant the process adapter access.
     *---------------------------------------------------------------*/

    psel = pproc->pDomainCur[domain];

    if (prcx == psel)
    {
	/*---------------------------------------------------------------
 	   This context is the one that the process wants active.
	   Turn on domain authorization in mstsave, provided the
	   gp_block_gp callback mode isn't in effect.  Always make
	   sure the DOMAIN_SUSPENDED bit is cleared, even when bus
	   access isn't granted.

	   If fast domain lock is set on the context, this will grant
	   the lock, but highwater mark may be in effect.

	 *---------------------------------------------------------------*/
	pdom->flags &= ~DOMAIN_SUSPENDED;

	if (!(pproc->procHead.flags & PROC_DD_BLOCK_REQUEST))
		fix_mstsave (pproc, pdom, FIX_MSTSAVE_ADD);
    }
    else
	pdom->flags |= DOMAIN_SUSPENDED;

    /*
     *  Optionally start a timer.  This is to prevent thrashing by
     *  allowing sufficient time for rendering between context switches.
     *
     *  Even though the timer is on the domain, it is operated as a
     *  PROCESS timer.  When a context for a new process goes onto
     *  the domain, the timer is started.  Any switch to another context
     *  does NOT start a new timer if the owning process does not change.
     *  This prevents monopoly by indefinite extension of time slice,
     *  which a process could otherwise do (by switching contexts).
     *  Exception:  Details are given below of the requirements necessary
     *  to avoid thrashing when one process owns the old and new contexts.
     *  The ped driver reenqueues the contexts being switched off.
     *
     *  The timer is not started if any kind of domain lock is in effect.
     *  This is a necessary optimization, since a switch under a lock may
     *  often be a temporary change necessary to perform a control operation
     *  on the adapter.  (This is particularly true for the Ped family).
     *  The domain is then free to switch immediately when the block is removed,
     *  unless a PREEXISTING time slice is still in effect.
     *
     *  The timeslice size is defined in the context structure member
     *  'timeslice', which is primed by the RCM, and may be altered by
     *  the DD level.  Timeslice values of zero are not currently supported.
     *
     *  Rcx_switch didn't turn the timer off if it was on and the process
     *  ownership of old/new contexts didn't change (or if the switch was
     *  taking place under a block).  Therefore, the timer may be on at this
     *  point.  However, it could also have expired before reaching this point.
     *
     *  The RCX_TIME_SLICE flag bit means that this instance of the context
     *  on the domain has been covered to some extent by a timeslice.  This
     *  helps in the diagnosis and avoidance of thrashing.
     */
    if (pdom->flags & DOMAIN_TIMER_ON)
	prcx->flags |=  RCX_TIME_SLICE;
    else
	prcx->flags &= ~RCX_TIME_SLICE;

    /*
     *  Time slices are NOT started if already on, or if domain is blocked.
     *  Else, a timeslice is started if the process ownership has changed.
     *  Timeslice is NOT started just because a context is reenqueued.
     */
    if (!(pdom->flags & (DOMAIN_BLOCKED | DOMAIN_TIMER_ON)) &&
	pdom->pOldCurProc != pdom->pCurProc)
    {
	start_time_slice (pdom, prcx);
    }

    /* if gp blocked */
    if (prcx->flags & RCX_BLOCKED) {
	BUGLPR(dbg_rcmswitch,BUGACT,
	       ("====== ... rcx_switch_done - blocked pid=0x%x \n",
		prcx->pProc->procHead.pid));
	/* unblock the gp and clear rcx flag */
	prcx->flags &= ~RCX_BLOCKED;

	RCM_TRACE(0x121,getpid(),prcx->pProc->procHead.pid,prcx->pProc);

	uexclear (prcx->pProc->procHead.tid);

    }

    /*
     *  Call the dispatch_dom function for necessary operations as follows:
     *
     *  1) The time slice is allowed to cover all serial context switches which
     *     belong to the same process.  That is, when the process owning the
     *     context on the domain changes, then a timeslice may be started (if
     *     no block is in effect).  The timeslice runs throughout all sub-
     *     switches instituted by the same process (including reenqueues)
     *     until the timeslice expires.  The timeslice may have just expired
     *     DURING THE LAST CONTEXT SWITCH WE ARE PROCESSING HERE.  If any other
     *     process is on the fault list, we do not allow the current process to
     *     hog the adapter by giving it any more time.  It has already used up
     *     one timeslice.  We immediately switch to the faulted process.
     *
     *     Another similar scenario would be continuous serial switches without
     *     timeslices (only possible without reenqueue), interrupted by a fault
     *     from another graphics process DURING THE LAST CONTEXT SWITCH WE ARE
     *     PROCESSING HERE.  In this case, we likewise give control to the
     *     faulting process.
     *
     *     Entertaining the possibility of dispatching now avoids a possible
     *     stalled fault list.
     *
     *  2) It is also necessary to wake up the guard-domain sleepers, since
     *     one of them may be waiting on the switch to finish.  Since all
     *     domain dispatching code has been gathered into 'dispatch_dom', it
     *     is necessary to call that.
     *
     *  3) Other things may enter dispatch_dom, also.
     */
    dispatch_dom (pdom);

    if (!(switches & RCX_SWITCH_NO_ENABLE))
	i_enable (old_int);
    else
    {
	RCM_ASSERT (i_disable (INTMAX) == INTMAX, 0, 0, 0, 0, 0);
    }

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("==== Exit rcx_switch_done... rcx for domain now=0x%x\n\n",
	    pdom->pCur));
    gsctrace (RCX_SWITCH_DONE, PTID_EXIT);
}


start_time_slice (pdom, prcx)
devDomainPtr	pdom;
rcxPtr		prcx;
{
    if (prcx->timeslice != 0)
    {
	/* set timer on flag and start timer  */
	pdom->flags |= DOMAIN_TIMER_ON;
	pdom->pDevTimer->timeout.it_value.tv_sec = DEBUG_TIME;
	pdom->pDevTimer->timeout.it_value.tv_nsec = prcx->timeslice;
	/* make timer incremental */
	pdom->pDevTimer->flags = 0;

	RCM_TRACE(0x122,pdom->pDevTimer->timeout.it_value.tv_sec, pdom->pDevTimer->timeout.it_value.tv_nsec,0);

	tstart (pdom->pDevTimer);

	prcx->flags |= RCX_TIME_SLICE;
    }
}

/* =============================================================

		INTERNAL FUNCTIONS

   ============================================================= */


/* ============================================================= */
/* FUNCTION: rcx_switch
*/
/* PURPOSE:  switches rcx for domain
*/
/* DESCRIPTION:
	    It performs the necessay actions to start a switch.
	    It then at calls the device specific
	    function, which at least starts the switch. If the
	    switch is heavy, and called from the fault handler,
	    it requests the HSC to finish the job; if
	    called from system call, does switch itself.
*/
/* INVOCATION:
    void rcx_switch (pold, pnew, switches, mask)
	rcxPtr	old, new;
	ulong	switches;
	ulong	mask;
*/
/* CALLS:
	vddstart_switch - to start a rcx switch
*/
/* DATA:
	RCM/DDH structure
*/
/* RETURNS:
	none
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	THIS ROUTINE MUST BE REENTRANT. In fact, many gp's could be
	waiting in this routine to get their rcx current.

	It assumes that interrupts are disabled on entry.

	Expected to get called from the fault handler at the time
	of a graphics fault or from aixgsc routines that must
	switch to a new rcx.

	This function has a very simple algorithm for adjusting
	rcx priority. It may need to be more sophisticated.

	This function should only be called to block a function from
	the exception handler for that function.
*/

 

void rcx_switch (pold, pnew, switches, old_int)

    rcxPtr	pold, pnew;
    ulong	switches;	/* block req, intrpt control */
    int	        old_int;

{
    devDomainPtr    pdom;
    int 	    seq;	      /* used to sequence rcx switches */
    int 	    ret;

    gsctrace (RCX_SWITCH, PTID_ENTRY);

    RCM_ASSERT (i_disable (INTMAX) == INTMAX, 0, 0, 0, 0, 0);

    pdom = pnew->pDomain;

    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("\n==== Enter rcx_switch, pold=0x%x, pnew=0x%x, domain=0x%x\n",
	    pold, pnew, pdom));

    /* set domain switching flag and clear domain suspended */
    pdom->flags |= DOMAIN_SWITCHING;

    /*
     *  Timer maintenance:  If we are switching to another process' context
     *  and we are not under a domain block, then stop the timer.  This means
     *  that switching under a domain block doesn't touch the timer.  Neither
     *  does switching to another context for the same process.
     *
     *  Fast domain lock does not apply here, since we do not switch while
     *  fast domain lock is in effect.
     */
    if (pold != NULL &&
	pnew->pProc != pold->pProc && !(pdom->flags & DOMAIN_BLOCKED))
    {
	if (pdom->flags & DOMAIN_TIMER_ON)
	{
	    pdom->flags &= ~DOMAIN_TIMER_ON;
	    tstop (pdom->pDevTimer);
	}
    }

    RCM_TRACE(0x130,getpid(),pold,pnew);

    /*
     *  Save the id (pointer) of the previous owner in the domain
     *  structure.  This is so rcx_switch_done can know whether the
     *  owning process changed.  This fact is used to start the domain
     *  timer.  We don't touch the timer if successive domain manipulations
     *  (including switching) are done by the same process.
     */
    pdom->pOldCurProc = pdom->pCurProc;

    /*
     *  Remove any instance of the context that we are going to switch
     *  on from the fault list.  There are several reasons it could have
     *  been put there, including reenqueue of switched off contexts by
     *  the ped driver.
     */
    (void) rm_rcx_fault (pdom, pnew);

    /*
     *  We don't need any of the rest of this if we are "switching" to the
     *  same context.  What's above is needed to make rcx_switch_done work
     *  right.
     *
     *  If switching to the same context, then the timer is not touched by
     *  rcx_switch or rcx_switch_done.  The timer may be running and may
     *  expire at any time.
     */

    if (pnew == pold)
    {
	rcx_switch_done (pdom, pnew, switches | RCX_SWITCH_NO_ENABLE);
    }
    else
    /*
     *  Contexts differ.
     */
    {
        /*
         *  Remove bus access from process that owns the old context.
         *  Process the old context priority downward.
         */
        if (pold != NULL)
        {
	    fix_mstsave (pold->pProc, pdom, FIX_MSTSAVE_SUB);
	    pdom->flags |= DOMAIN_SUSPENDED;

	    pold->priority += -1;
	    if (pold->priority < 1)
	        pold->priority = pold->pProc->procHead.priority;
        }

        /*---------------------------------------------------------------
           set current rcx and gp owning it 

           The fault handler depends on these parameters being set up
           for the new context.
         *---------------------------------------------------------------*/
        pdom->pCur = pnew;
        pdom->pCurProc = pnew->pProc;

	/*
	 *  Call the DD level to do the switch and possibly reenable
	 *  interrupts.
	 */
        ret = pfrm_switch (pdom, pold, pnew, switches, old_int);
    }

    BUGLPR(dbg_rcmswitch,BUGNFO, ("==== Exit rcx_switch... \n\n"));

    gsctrace (RCX_SWITCH, PTID_EXIT);
}



/* ============================================================= */
/* rcx_fault_list

    It puts an rcx on the fault list at the correct priority

    Expected to get called from the fault handler or from aixgsc routines
    that must put an rcx on the fault list.

    Should only be asked to block a process from the exception
    handler.

    Interrupts MUST BE guaranteed disabled by calling routine.

*--------------------------------------------------------------------*/

void rcx_fault_list (pdom, prcx, switches)

    devDomainPtr    pdom;	  /* domain  */
    rcxPtr	    prcx;	  /* rcx to put on list */
    ulong	    switches;	  /* switches for action */

{
    rcxPtr	    prcxt;

    gsctrace (RCX_FAULT_LIST, PTID_ENTRY);

    /*-------------------------------------------------------------------------
        Echo the input parms: 
     *-----------------------------------------------------------------------*/
    BUGLPR(dbg_rcmswitch,BUGNFO,
	   ("==== Enter rcx_fault_list, domain=0x%8X, rcx=0x%8X\n", pdom,prcx));
    BUGLPR(dbg_rcmswitch, 1, ("===========     switches = 0x%4X\n", switches));

    RCM_TRACE(0x150, prcx->pProc->procHead.pid, prcx, switches);
    RCM_TRACE(0x15F, getpid (), pdom->pCur, pdom -> pFault);
    TRACE_FAULT_LIST(pdom, 0x14b);



    /*-------------------------------------------------------------------------
       Put rcx on fault list at priority 
       (First check if it is already there.)
     *-----------------------------------------------------------------------*/

    if ( !(prcx -> flags & RCX_ON_FAULT_LIST) ) 
    {
        /*---------------------------------------------------------------------
           First set the "on fault list flag" 
         *-------------------------------------------------------------------*/
        prcx -> flags |= RCX_ON_FAULT_LIST ;


    	/*---------------------------------------------------------------------
   	   Now, there are three cases: 

   	     .  fault list is currently empty 
   	     .  context's priority is higher than current highest
   	         (actually this is the same case as the first)    
   	     .  Context goes somewhere after the top of the list
    	 *-------------------------------------------------------------------*/


    	if (pdom->pFault == NULL) 
	{
    	    /*-----------------------------------------------------------------
   	       Case 1:  fault list currently empty :  put on top of list
    	     *---------------------------------------------------------------*/
	    pdom->pFault = prcx;
	    prcx->pNextFlt = NULL;

	    RCM_TRACE(0x151, pdom->pFault,
			pdom->pFault->pNextFlt, prcx->pNextFlt);

	    RCM_DEBUG_TRACE (FAULT_LIST, 0, 0, (0), 0, 0);
    	}
	else if (pdom->pFault->priority < prcx->priority) 
	{
    	    /*-----------------------------------------------------------------
   	       Case 2:  Context higher priority than current highest
		         Again, it is put on the top of the list.   
    	     *---------------------------------------------------------------*/
	    prcx->pNextFlt = pdom->pFault;
	    pdom->pFault = prcx;

	    RCM_TRACE(0x152, pdom->pFault,
			pdom->pFault->pNextFlt, prcx->pNextFlt);

	    RCM_DEBUG_TRACE (FAULT_LIST, 0, 0, (1), 0, 0);
    	}
	else 
	{
    	    /*-----------------------------------------------------------------
   	       Case 3:  Context goes in middle of list 
   	       
   	          We must search the list for the proper priority
    	     *---------------------------------------------------------------*/
	    for (prcxt = pdom->pFault; prcxt->pNextFlt != NULL;
	     					prcxt = prcxt->pNextFlt) 
	    {
	    	if (prcxt->pNextFlt->priority < prcx->priority)
		    break;
	    }

	    prcx->pNextFlt = prcxt->pNextFlt;
	    prcxt->pNextFlt = prcx;

	    RCM_TRACE(0x153, pdom->pFault,
			pdom->pFault->pNextFlt, prcx->pNextFlt);

	    RCM_DEBUG_TRACE (FAULT_LIST, 0, 0, (2), 0, 0);
    	}
    }

    /*-------------------------------------------------------------------------
       Now check for the types of process waits requested.

       NOTE:  the e_wait is never requested, so this code has been removed.
     *-----------------------------------------------------------------------*/

    /*-------------------------------------------------------------------------
       Now check for a block request (this can only be done from an 
	exception handler path).  
     *-----------------------------------------------------------------------*/

    if (switches & RCX_FAULT_BLOCK)
    {
	/* set block flag (unblock at dispatch time) */
	prcx->flags |= RCX_BLOCKED;

	/* block gp */
	RCM_TRACE(0x156, prcx->pProc->procHead.pid, prcx->pProc, prcx->flags);
	uexblock (prcx->pProc->procHead.tid);
    }

    TRACE_FAULT_LIST(pdom, 0x14c);

    BUGLPR(dbg_rcmswitch,BUGNFO, ("==== Exit rcx_fault_list...\n"));

    gsctrace (RCX_FAULT_LIST, PTID_EXIT);
}

#ifdef  DEBUG
trace_fault_list(pdom, val)
devDomainPtr	pdom;		/* domain  */
int val;
{
	struct _rcx *prcxt;		/* context */
	int a, b, c;

	prcxt = pdom->pFault;
	a = prcxt;
	b = prcxt->pNextFlt;
	c = prcxt->pNextFlt->pNextFlt;
	RCM_TRACE(val, a, b, c);
}
#endif


    /************************************************************************** 

                		START of PROLOGUE 

        Function:  gp_block_gp

        Descriptive name:  Device Dependent Process Blocking Function 

     *----------------------------------------------------------------------*  

        Function:  
           This routine blocks the graphics process for a specified
           graphics process. 

           Roughly, this entails setting a flag which notifies the
           graphcis fault handler that the process is not to be dispatched
           (until the device dependent code say so).   This is accomplished
           by having the exception handler block the graphics process.
           Note that the block can be performed only from an exception 
           handler. 

           Note also, that the process remains blocked until the device
           driver issues the companion request to "unblock the process".

           The other, other note is that the block is triggered by an
           adapter access.  No access, no actual block. 
           If the graphics process is currenlty rendering on the
           adapter, its access is removed immediately, so the block will
           happen immediately (next adapter access).   


     *************************************************************************/


gp_block_gp	(
		devDomain	*pDom,
		rcmProc		*pProc )
{
	int old_int;

    /*-------------------------------------------------------------------------
        Disable interrupts 
     *-----------------------------------------------------------------------*/
	old_int = i_disable(INTMAX);

        RCM_TRACE(0x180,pProc->procHead.pid, pDom, pProc->procHead.flags);

    /*-------------------------------------------------------------------------
        Set the "process blocked by device driver" flag.  This flag
	makes the fault handler recognize this special case.  If a
	switch is in progress, rcx_switch_done will be informed to set up the 
	domain in the manner that this code does here.  Bus access will
	NOT be granted.

	This sets things up so that the fault handler will block the
	process IF THE PROCESS FAULTS.  Since there may be domain locks
	in place, we may not able to switch the domain, even when one of
	these blocks is in place.  This processing must not interfere with
	the way the fault-handler already works for such processes.
     *-----------------------------------------------------------------------*/
	pProc -> procHead.flags |= PROC_DD_BLOCK_REQUEST ; 

    /*-------------------------------------------------------------------------
        Check if the context on the domain (or being switched on) belongs to
	our process.  If so, the process access to the graphics hardware
	(adapter) is removed immediately.  The fact that the bus access
	may be removed without setting DOMAIN_SUSPENDED means that processes
	which use fast domain lock will still prevent switching if this
	callback is used for high-water-mark.
     *-----------------------------------------------------------------------*/
	if (pDom->pCurProc == pProc) 
	{ 
		if (!(pDom->flags & DOMAIN_SWITCHING))
			fix_mstsave (pProc, pDom, FIX_MSTSAVE_SUB);
	}

    /*-------------------------------------------------------------------------
        Enable interrupts and return
     *-----------------------------------------------------------------------*/
	i_enable(old_int);
	return 0;
}




    /************************************************************************** 

                		START of PROLOGUE 

        Function:  gp_unblock_gp

        Descriptive name:  Device Dependent Process Unblocking Function 

     *----------------------------------------------------------------------*  

        Function:  
           This function is the companion of the (previous) process 
           blocking function. 




     *************************************************************************/


gp_unblock_gp	(
		devDomain	*pDom,
		rcmProc		*pProc )
{
	int old_int;

    /*-------------------------------------------------------------------------
        Disable interrupts 
     *-----------------------------------------------------------------------*/
	old_int = i_disable(INTMAX);

        RCM_TRACE(0x190,pProc->procHead.pid, pDom, pProc->procHead.flags);

	RCM_ASSERT (pProc->procHead.flags & PROC_DD_BLOCK_REQUEST,
							0, 0, 0, 0, 0);

    /*-------------------------------------------------------------------------
	If our process "owns" the context on (or being switched on) the
	domain, then restore conditions.

        If the domain is not switching and is not suspended, then reinstate
	bus access immediately.  If switching, rcx_switch_done will reinstate
	bus access.  If suspended, then the normal guard_dom/fault_handler
	procedures rule.

	Gp_block_gp does NOT set the suspended bit.  If that bit is set,
	then something besides gp_block_gp has occured to cause it.
     *-----------------------------------------------------------------------*/
	if (pDom->pCurProc == pProc) 
	{ 
		if (!(pDom->flags & (DOMAIN_SWITCHING | DOMAIN_SUSPENDED)))
			fix_mstsave (pProc, pDom, FIX_MSTSAVE_ADD);
	}

    /*-------------------------------------------------------------------------
	If the process was actually blocked, the block is now cleared. 
     *-----------------------------------------------------------------------*/
	if (pProc->procHead.flags & PROC_PROCESS_BLOCKED) 
		uexclear(pProc -> procHead.tid);

    /*-------------------------------------------------------------------------
        Reset the flags
     *-----------------------------------------------------------------------*/
	(pProc -> procHead.flags) &= ~(PROC_DD_BLOCK_REQUEST) ; 
	(pProc -> procHead.flags) &= ~(PROC_PROCESS_BLOCKED) ; 

    /*-------------------------------------------------------------------------
        Enable interrupts and return
     *-----------------------------------------------------------------------*/
	i_enable(old_int);
	return 0;
}

/*
 *  dispatch_dom - Find the next user of this domain, if any.
 *
 *  This must be called with interrupts prevented.
 *
 *  This is currently called from unguard_dom, gp_give_up_time_slice,
 *  gp_dispatch (clock interrupt), and rcx_switch_done.
 *
 *  NOTE:  The fault list dispatching near the end of this function will (if
 *  used) call rcx_switch.  This, in turn, will end up calling rcx_switch_done.
 *  Rcx_switch_done will, in turn, call this function dispatch_dom.
 *
 *  Dispatch_dom will be called recursively on the current thread in the
 *  case of a light switch.  For heavy switch, the "recursive" call will be
 *  on the Heavy Switch Controller process thread.
 *
 *  This is intentional, and is supposed to work.
 */
int dispatch_dom (devDomainPtr  pdom)
{
    int  fastlocked = -1;	/* "uninitialized" */
    rcmProcPtr  pproc = pdom->pCurProc;

    RCM_ASSERT (i_disable (INTMAX) == INTMAX, 0, 0, 0, 0, 0);

    /*
     *  We can make a check now to see if there is any condition that
     *  inhibits the effect of dispatch_dom, AND which guarantees that
     *  dispatch_dom will be called later.  In these cases, we can optimize
     *  our path length by not calling e_wakeup, etc.
     *
     *  Items currently checked:
     *
     *  1)  Domain switching.  Nothing can be done to dispatch the domain in
     *      this case, and rcx_switch_done will call dispatch_dom when it gets
     *      through.
     *
     *  Items that USED to be checked:
     *
     *  1)  Domain timer on.  There is a case where the domain can be guarded
     *      by someone who owns the timeslice.  Therefore, the timer check
     *      has been distributed over the other cases.
     */

    /*
     *  Always check for the need for feedback to domain lockers.  Guard_dom
     *  and the fault handler do this if the clock is off.  We do it here if
     *  the clock is off.  Since this is called from the clock interrupt code,
     *  we will get feedback service closure.
     */
    if (!(pdom->flags & DOMAIN_TIMER_ON))
    {
	/*
	 *  If the timer is off, then feedback a request for
	 *  fast domain unlock/give-up-timeslice call from app.
	 */
	fastlocked = (!(pdom->flags & (DOMAIN_SUSPENDED | DOMAIN_SWITCHING)) &&
		      ck_fast_dom_lock (pdom->pCur));

	if (fastlocked)
	{
	    RCM_TRACE (0x770, pproc->procHead.pid, pdom, pdom->flags);

	    sig_fast_dom_lock (pdom->pCur, 1);
	}

	/*
	 *  If this is a syscall domain block, and if the timer is off,
	 *  then signal a request for domain unlock.
	 */
	if (pdom->flags & DOMAIN_LOCKED)
	{
	    RCM_TRACE (0x771, pproc->procHead.pid, pdom, pdom->flags);

	    pidsig (pdom->pLockProc->procHead.pid, SIGURG);
	}
    }

    /*
     *  If the domain is switching, a context is already being dispatched.
     *  Do nothing.  This can only apply when called from gp_dispatch (clock
     *  interrupt), since calls from unguard_dom, gp_give_up_time_slice, and
     *  rcx_switch_done cannot occur when the domain is switching.
     */
    if (pdom->flags & DOMAIN_SWITCHING)
    {
	RCM_TRACE (0x772, pproc->procHead.pid, pdom, 0);
    }
    else
    /*
     *  The domain is not switching.
     *
     *  If processes are waiting to guard the domain or lock the adapter,
     *  then signal the waiters.  These people get higher priority since
     *  they are in the kernel (?).
     *
     *  This kind of assumes one of the processes that gets waked will run
     *  and block the domain.  But, there is nothing in the code that checks
     *  that any one of those sleeping processes can block the domain if it
     *  is waked, nor is there any guarantee that conditions will be the same
     *  when the kernel dispatcher finally runs that process.  What we mainly
     *  do in this case is just refrain from starting any switch.
     */
    if (pdom->guardlist != EVENT_NULL)
    {
	RCM_TRACE (0x773, pproc->procHead.pid, pdom, 0); /* wakeups were done */

	e_wakeup (&pdom->guardlist);

	pdom->flags |= DOMAIN_GUARD_WAKED;
    }
    else
    /*
     *  If the process with the current context has the domain fast locked,
     *  then we do nothing.  Note the kloodgey way we avoid calling
     *  ck_fast_dom_lock unless we have to.  The highest overhead is with
     *  gemini, only.
     */
    if (fastlocked > 0 || (fastlocked < 0 && ck_fast_dom_lock (pdom->pCur) &&
	!(pdom->flags & DOMAIN_SUSPENDED)     ))
    {
	RCM_TRACE (0x774, pproc->procHead.pid, pdom, 0);
    }
    else
    /*
     *  If the domain is blocked, do nothing.
     */
    if (pdom->flags & DOMAIN_BLOCKED)
    {
	RCM_TRACE (0x775, pproc->procHead.pid, pdom, 0);
    }
    else
    /*
     *  If the timer is on, do nothing.
     */
    if (pdom->flags & DOMAIN_TIMER_ON)
    {
	RCM_TRACE (0x776, pproc->procHead.pid, pdom, 0);
    }
    else
    /*
     *  Try to dispatch the fault list.
     */
    {
	rcxPtr   pold, pnew;

	pold = pdom->pCur;
	pnew = pdom->pFault;

	RCM_TRACE (0x777, pproc->procHead.pid, pdom, pnew);

	/*
	 *  Remove first rcx from fault list, if any.
	 */
	if (pnew != NULL)
	{
	    /*
	     *  THRASHING TEST:
	     *
	     *  The Ped family uses the gp_put_on_fault_list callback to re-
	     *  enqueue contexts on the fault list.  If we are not careful this
	     *  can cause thrashing.  This is how we avoid this:
	     *
	     *  If we are attempting to switch on a context that belongs to the
	     *  same process as currently owns the context actually on the
	     *  domain, then under certain circumstances, we back up a notch
	     *  in the dispatch_dom logic, institute a timeslice to avoid
	     *  thrashing, and exit.
	     *
	     *  Institution of timeslice depends on whether the context has
	     *  ever had a timeslice active during its current trip through the
	     *  domain.
	     *
	     *  We know at this point that no locks, blocks, or timeslices are
	     *  in effect, nor is any process waiting to guard the domain,
	     *  therefore we are able to institute a timeslice.
	     *
	     *  NOTE:  This does NOT guarantee that a reenqueued context gets
	     *  a timeslice, if the reenqueued context which just got switched
	     *  on was not touched by user process and there was work to do
	     *  for another process on the fault list.  This continues the
	     *  philosophy of blowing switching work just done for any process
	     *  if it has had at least one timeslice, and there is work to do
	     *  for another process.
	     */
	    if (pold != NULL &&
		!(pold->flags & RCX_TIME_SLICE) &&	/* never a timeslice */
		pnew->pProc == pold->pProc &&		/* owner the same */
		pold->timeslice != 0)		/* MUST BE interval given */
	    {
		start_time_slice (pdom, pold);
	    }
	    else
	    {
		pdom->pFault = pnew->pNextFlt;
		pnew->flags &= ~RCX_ON_FAULT_LIST ;

		/*
		 *  Switch to the new context.  If light, the switch will be
		 *  complete upon return from rcx_switch.  If heavy, queue the
		 *  heavy switch to the heavy switch controller.  The controller
		 *  will run rcx_switch_done later.
		 *
		 *  Rcx_switch/rcx_switch_done will not touch the timer unless
		 *  we are switching between contexts owned by different
		 *  processes, AND we are not under a domain block.  In that
		 *  case, the timer will be stopped before the switch, and a new
		 *  timeslice will be started by rcx_switch_done.  Else, the
		 *  timer may be running, and may expire at any time.
		 *
		 *  Fast domain lock does not apply here, since we do
		 *  not switch while fast domain lock is in effect.
		 *
		 *  Rcx_switch successfully handles the case (pold == pnew).
		 *
		 *  Reenable interrupts.
		 */
		rcx_switch (pold, pnew,
			RCX_SWITCH_QUEUE | RCX_SWITCH_NO_ENABLE, 0);
	    }
	}
	else
	    RCM_TRACE (0x780, pproc->procHead.pid, pdom, 0);
    }
}

/*
 *  pfrm_switch - Perform a context switch by calling the DD level.
 *
 *  This must be called with interrupts prevented.  The switches must
 *  be set properly to allow interrupts to be reenabled if a heavy switch
 *  is done.
 */
int pfrm_switch (
devDomainPtr  pdom,
rcxPtr pold,
rcxPtr pnew,
ulong switches,
int mask)
{
    int  rc = 0, seq, switchtype;

    /*
     *  Initiate switch operation.
     */
    switchtype = (*pdom->pDev->devHead.display->start_switch)
					(pdom->pDev, pold, pnew, &seq);

    /*
     *  If a light switch, then finish up now.
     */
    if (switchtype == RCM_LIGHT_SWITCH)
    {
	/*
	 *  Rcx_switch_done will not start a timer unless the contexts
	 *  which were switched belonged to different processes, AND
	 *  the domain is not blocked.  In case no timer is to be started,
	 *  the domain timer may still be running, and may expire at any
	 *  time.
	 */
	rcx_switch_done (pdom, pnew, RCX_SWITCH_NO_ENABLE);
    } 
    else if (switchtype == RCM_HEAVY_SWITCH)
    { 
	/* if to be queued to the heavy switch controller */
	if (switches & RCX_SWITCH_QUEUE) 
	{
	    hscQE	    qe; 	      /* qe for hsc */

	    /*
	     *  Build the HSC queue packet, and submit to the HSC.
	     */
	    qe.command = HSC_COM_SWITCH;
	    qe.pDev = pdom->pDev;
	    qe.pNew = pnew;
	    qe.pOld = pold;
	    qe.seq = seq;

	    rc = hsc_enq (&qe);
	}
	else		/* do it in-line */
	{
	    RCM_ASSERT (!(switches & RCX_SWITCH_NO_ENABLE), 0, 0, 0, 0, 0);

	    i_enable (mask);

	    rc = pfrm_end_switch (pdom, pold, pnew, seq);

	    return (rc);
	}
    }
    else
	rc = EIO;

    if (!(switches & RCX_SWITCH_NO_ENABLE))
	i_enable (mask);

    return (rc);
}


/*
 *  pfrm_end_switch - Perform the last part of a heavy switch by calling the
 *		      DD level end_switch function.
 *
 *  This is called with interrupts allowed.
 */
int pfrm_end_switch (
devDomainPtr  pdom,
rcxPtr pold,
rcxPtr pnew,
int seq)
{
    int  old_int, rc;

    /*
     *  Finish the heavy switch.
     */
    rc = (*pdom->pDev->devHead.display->end_switch)
					(pdom->pDev, pold, pnew, seq);

    /*
     *  Rcx_switch_done will not start a timer unless the contexts
     *  which were switched belonged to different processes, AND
     *  the domain is not blocked.  In case no timer is to be started,
     *  the domain timer may still be running, and may expire at any
     *  time.
     */
    rcx_switch_done (pdom, pnew, 0);
}
