static char sccsid[] = "@(#)70	1.6  src/bos/kernel/ios/uphysio_pin.c, sysios, bos411, 9435A411a 8/26/94 10:42:52";

/*
 * COMPONENT_NAME: (SYSIOS) Raw I/O (uio) services
 *
 * FUNCTIONS:	uphystart	uphyswait	uphysdone	
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	vm_swp.c	7.1 (Berkeley) 6/5/86
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/xmem.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/thread.h>
#include <sys/lock_def.h>
#ifdef _POWER_MP
#include <sys/atomic_op.h>
#endif /* _POWER_MP */

Simple_lock uphysio_lock;	/* alloc and init is done in uphysinit() */
				/* under #ifdef _POWER_MP */


/*
 * NAME:  uphystart
 *
 * FUNCTION:  This routine initiates the raw i/o for the list of
 *	requests. The requests are put on the list of active requests
 *	so that completion processing can later be performed on them
 *	by uphyswait
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is only called by uphysio.
 *
 *	It may page fault.
 *
 * NOTES:
 *	The initial version for AIX 4.1 made use of the devstrat() routine
 *	to call the device driver's strategy routine in an ensured funnelised
 *	environment. However, this did not ensure that uphystart() calls the
 *	routine passed as argument.
 *	With the same reasoning that lead to the conclusion that the mincnt()
 *	routine does not require funnelling, the device driver's strategy
 *	routine should not require additional precaution, i.e., execution is
 *	performed on the unix master.
 *	This does make the assumption that the strategy function and the
 *	top half calling it are of the same type.  i.e. top half for driver
 *	A(MP-Safe) is not using strategy function of driver B(funnelled).
 *	LVM has muliple devswitch table entries, but the top and bottom
 *	halves are matched.
 *	As for syncing data changes, any MP-Safe strategy routine will either
 *	have disable locks in it and therefore be protected without the need
 *	of an explicit sync here or, if required, will perform the necessary
 *	sync operation itself.
 *
 * DATA STRUCTURES: none
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_lock	serialize with uphysdone
 *	unlock_enable	end of uphysdone critical section
 *	strat		device driver's strategy routine
 */
void
uphystart(
register int	(*strat)(),		/* ptr to I/O strategy routine	*/
register struct buf *sbp,		/* list of requests to start	*/
register struct buf **abp)		/* list of active requests	*/
{
	register struct buf *nbp;	/* next on request active list	*/
	register int	opri;		/* current interrupt priority	*/
	register int	(*tempstrat)();

	tempstrat = strat;
	/*
	 * Put the buffer headers on the list of active requests.
	 */
	nbp = sbp;
	while (nbp != (struct buf *)NULL)
	{
		opri = disable_lock(INTIODONE, &uphysio_lock);
		if (*abp == (struct buf *)NULL)
		{
			nbp->b_forw = nbp;
			nbp->b_back = nbp;
			*abp = nbp;
		}
		else
		{
			nbp->b_forw = *abp;
			nbp->b_back = (*abp)->b_back;
			nbp->b_back->b_forw = nbp;
			nbp->b_forw->b_back = nbp;
		}
		unlock_enable(opri, &uphysio_lock);
		nbp = nbp->av_forw;
	}

	/*
	 * Initiate the list of requests.
	 */
	(*tempstrat)(sbp);

}   /* end uphystart */

/*
 * NAME:  uphyswait
 *
 * FUNCTION: Wait for at least one raw i/o request to complete and
 *	perform completion processing for it. Other requests that
 *	complete soon enough may be processed at the same time.
 *
 *	This service may not perform completion processing for any
 *	requests if the free list is not empty when it is called.
 *	It will always perform completion processing for at least
 *	one request when the free list is empty. Calling this
 *	routine with the free list not empty is useful for early
 *	detection of completion of requests. It is also used to
 *	wait for the last few requests to complete.
 *
 *	This routine may also return without putting a buffer
 *	header on the free list when the only active request
 *	results in an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is only called by uphysio.
 *
 *	It may page fault.
 *
 * NOTES:
 *	A buffer header may be lost if an error detected earlier is
 *	replaced with a newer one. This is ok since no additional
 *	requests are started and all of the buffers are freed
 *	by one xmfree call of the entire pool.
 *
 * DATA STRUCTURES: none
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	ASSERT
 *	assert
 *	disable_lock	serialize with uphysdone
 *	e_sleep_thread
 *	fetch_and _add_h
 *	unlock_enable	end of uphysdone critical section
 *	xmemunpin	unpin the buffer
 *	xmdetach	cross memory detach
 */
void
uphyswait(
register struct buf	**abp,		/* list of active requests	*/
register struct buf	**fbp,		/* list of free buffer headers	*/
register struct buf	**ebp,		/* earliest error		*/
register int		flag)		/* UIO_XMEM or not		*/
{
	register struct buf *nbp;	/* next on request active list	*/
	register int	rc;		/* temporary return value	*/
	register int	opri;		/* current interrupt priority	*/

	/*
	 * Wait for at least one request to complete.
	 */
	while (*abp != (struct buf *)NULL)
	{

		/*
		 * Interrupts are disabled to prevent a request from
		 * completing after it is checked and before this
		 * routine waits. This is to prevent this routine
		 * from waiting until two requests complete to start
		 * processing the next.
		 */
		opri = disable_lock(INTIODONE, &uphysio_lock);

		/*
		 * Perform completion processing for all of the active
		 * requests that have completed. The list is processed
		 * in inverse order to simplify determining when done.
		 */
		nbp = *abp;
		do {
			nbp = nbp->b_back;
			if (nbp->b_flags & B_DONE)
			{

				/*
				 * Disconnect from the buffer.
				 */
				rc = xmemunpin(nbp->b_baddr, nbp->b_bcount,
				    &(nbp->b_xmemd));
				assert(rc == 0);

				/*
				 * If the user has already initialized
				 * his cross memory descriptors, then
				 * we don't need to mess with them here.
				 */
				if (flag != UIO_XMEM)
				{
					rc = xmdetach(&(nbp->b_xmemd));
					ASSERT(rc == XMEM_SUCC);
				}

				/*
				 * The error closest to the start of the uio
				 * request is the one returned to the caller.
				 * This may not be the first error found.
				 */
				if (nbp->b_resid || (nbp->b_flags & B_ERROR))
				{
					if ((*ebp == (struct buf *)NULL) ||
					    ((*ebp)->b_blkno > nbp->b_blkno))
					{
						*ebp = nbp;
					}
				}

				/*
				 * Put the buffer header at the front of the
				 * free list so that it can be reused.
				 */
				if (nbp != *ebp)
				{
					nbp->av_forw = *fbp;
					*fbp = nbp;
				}

				/*
				 * Remove this request from the active list.
				 */
				nbp->b_back->b_forw = nbp->b_forw;
				nbp->b_forw->b_back = nbp->b_back;
				if (*abp == nbp)
				{
					if (nbp == nbp->b_forw)
						*abp = (struct buf *)NULL;
					else
						*abp = nbp->b_forw;
					break;
				}

			}
		} while (nbp != *abp);

		/*
		 * Do not need to wait for a buffer header if the free list
		 * is already empty.
		 */
		if (*fbp != (struct buf *)NULL || *abp == NULL ){
			unlock_enable(opri, &uphysio_lock);
			break;
		}

		/*
		 * Wait for any active request to complete. The standard
		 * loop on B_DONE can not be used because the requests may
		 * complete out of order.
		 */
		syswait.physio++;
		rc = e_sleep_thread(&((*abp)->b_event), &uphysio_lock,
						LOCK_HANDLER);
		syswait.physio--;
		ASSERT(rc == THREAD_AWAKENED);

		unlock_enable(opri, &uphysio_lock);

	}

}   /* end uphyswait */

/*
 * NAME:  uphysdone
 *
 * FUNCTION:  Wake up any processes waiting on the completion of this
 *	raw i/o request. Most of the completion processing is performed
 *	by uphyswait under the process performing the raw i/o.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is only called in an interrupt execution environment
 *	by the off-level i/o done scheduler. It may be called under a
 *	process if iodone is called under a process. It still executes
 *	as an interrupt handler since it executes with selected interrupts
 *	disabled and on a pinned stack.
 *
 *	It does not page fault. The input buf structure must be pinned.
 *
 * NOTES:
 *	This routine does not touch any of the anchors maintained by
 *	physio. These anchors are on the caller's stack and may not be
 *	pinned.
 *
 * DATA STRUCTURES: none
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	ASSERT
 *	assert
 *	e_wakeup
 *	geterror
 *	simple_lock
 *	simple_unlock
 */
void
uphysdone(
register struct buf *bp)		/* ptr to buf that completed */
{
	register struct buf *nbp;	/* next active request */

	ASSERT(CSA->intpri == INTIODONE);

#ifdef _POWER_MP
	simple_lock(&uphysio_lock);
#endif /* _POWER_MP */

	/*
	 * Some drivers will return an ESOFT error.  This indicates that
	 * the command completed successfully after some recovery action.
	 * This action may cause a block to be scheduled for reassignment
	 * but does not translate into an error for the caller; therefore, 
	 * the error is cleared here.
	 */
	if (geterror(bp) == ESOFT)
	{
		bp->b_resid = 0;
		bp->b_error = 0;
		bp->b_flags &= ~B_ERROR;
	}
	bp->b_flags |= B_DONE;	/* This is no longer done in iodone() routine */

	/*
	 * Wake up anyone waiting for any buffer on this list
	 * of active buffers. The physwait routine waits on
	 * the first buffer on the active list. This is because
	 * it does not know which one will complete first. Any
	 * one else waiting on a raw i/o must perform the standard
	 * loop on B_DONE to ensure that the request is really
	 * complete when they wake up.
	 */
	nbp = bp;
	do {
		if (nbp->b_event != EVENT_NULL)
			e_wakeup(&(nbp->b_event));
		nbp = nbp->b_forw;
		assert(nbp != (struct buf *)NULL);
	} while (nbp != bp);

#ifdef _POWER_MP
	simple_unlock(&uphysio_lock);
#endif /* _POWER_MP */

}  /* end uphysdone */
