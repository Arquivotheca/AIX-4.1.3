static char sccsid[] = "@(#)71	1.9  src/bos/kernel/ios/uphysio_paged.c, sysios, bos411, 9428A410j 6/22/94 10:38:54";

/*
 * COMPONENT_NAME: (SYSIOS) Raw I/O (uio) services
 *
 * FUNCTIONS:	uphysio
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
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
#ifdef _POWER_MP
#include <sys/atomic_op.h>
#endif /* _POWER_MP */
/*
 * NAME:  uphysio
 *
 * FUNCTION:  This service performs a raw i/o for a block device
 *	driver. It is typically called by the character read or write
 *	routines. It initializes the buffer header, validates access
 *	rights to the buffer, makes the buffer accessable in other
 *	environments, and calls the device's strategy routine to
 *	perform the operation.
 *
 *	This service attempts to call the strat routine with as many
 *	requests as it can, up to the limit specified by the buf_cnt
 *	parameter. This allows for as much concurrency as the device
 *	supports and minimizes the i/o redrive time.
 *
 *	This service attempts to break the i/o requests into smaller
 *	sizes when pinned memory restrictions prevent the initiation
 *	of an i/o request.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine can only be called under a process.
 *
 *	It may page fault.
 *
 * NOTES:
 *	This service may start request after the device driver
 *	has detected and reported an error.
 *
 *	Extended parameters, such as b_work and b_options, can
 *	be passed to the mincnt routine via minparms. This
 *	provides the mincnt routine with the ability to initialize
 *	these fields for each request.
 *
 *	The buf_cnt parameter can be set to 1 to ensure strictly
 *	sequential operation.
 *
 *	The busy processing performed by the System V physio
 *	service and the BSD physio service does not need to be
 *	performed by this service because it allocates the
 *	buffer headers and therefore they can not be in use.
 *
 *	The System V physio option that allows the caller to
 *	supply the buffer header is not supported to simplify
 *	the logic.
 *
 * DATA STRUCTURES: none
 *
 * RETURN VALUE DESCRIPTION:
 *	0		successful completion
 *	EFAULT		invalid access authority
 *	ENOMEM		cannot allocte buffer headers or pin buffer
 *	EAGAIN		pin count exceeded on buffer
 *	b_error		device driver error value
 *
 * EXTERNAL PROCEDURES CALLED:
 *	ASSERT
 *	assert
 *	fetch_and_add_h
 *	xmalloc		allocate buffer headers
 *	mincnt		device driver's request check routine
 *	xmempin		prevent buffer from being paged out
 *	xmattach	attach for cross memory operation
 *	geterror	determine error value
 *	xmfree		free buffer headers
 */
int
uphysio(
register struct	uio *uio,		/* description of buffer	*/
register int	rw,			/* access mode:  read or write	*/
register uint	buf_cnt,		/* degree of concurrency	*/
register dev_t	dev,			/* device to read from/write to	*/
register int	(*strat)(),		/* ptr to I/O strategy routine	*/
register int	(*mincnt)(),		/* device request check routine	*/
register void	*minparms)		/* parameters for mincnt	*/
{
	register struct buf *mbp;	/* start of buffer header alloc	*/
	struct buf	*fbp;		/* list of buffer headers	*/
	struct buf	*abp;		/* list of active requests	*/
	struct buf	*sbp;		/* list of pending requests	*/
	struct buf	*ebp;		/* earliest error		*/
	struct buf	*nbp;		/* pointer to next buf allocated*/
	register struct buf *bp;	/* current buffer		*/
	register struct buf *lbp;	/* last buffer			*/
	register struct	iovec *uio_iov;	/* current iov struct		*/
	register struct xmem *uio_xmem;	/* current xmem struct		*/
	register int	uio_iovcnt;	/* #iovec structs left		*/
	register int	uio_iovdcnt;	/* #iovec structs completed	*/
	register offset_t   uio_offset;	/* current device offset - byte	*/
	register caddr_t iov_base;	/* current iov buffer address	*/
	register int	iov_len;	/* remaining iov byte count	*/
	register int	min_error;	/* mincnt return value		*/
	extern int 	geterror();
	register int	rc;		/* temporary return value	*/
	register int	done;		/* bytes transfered up to error	*/
	register int	error = 0;	/* uphysio return value		*/
	register int	buf_snt_cnt = 0;/* number buffers sent		*/
	extern void uphysdone();	/* routine to wakeup processes
					   waiting for this raw I/O	*/
	
	/*
	 * Validate the request.
	 */
	ASSERT(CSA->prev == NULL);
	assert((buf_cnt != 0) && (buf_cnt < 65));

	/*
	 * Allocate the requested number of buffer headers. The buffer
	 * headers are pinned by xmalloc because they are allocated out
	 * of the pinned heap.
	 */
	mbp = (struct buf *)xmalloc(buf_cnt * sizeof(struct buf), 0,
				    pinned_heap);
	if (mbp == (struct buf *)NULL)
		return(ENOMEM);

	/*
	 * Initialize anchor for the free list.
	 */
	nbp = mbp + buf_cnt;
	fbp = (struct buf *)NULL;

	/*
	 * Initialize the various internal variables for the start of
	 * the request.
	 */
	abp = (struct buf *)NULL;
	sbp = (struct buf *)NULL;
	ebp = (struct buf *)NULL;
	bp = (struct buf *)NULL;
	lbp = (struct buf *)NULL;
	uio_offset = uio->uio_offset;
	uio_iovcnt = uio->uio_iovcnt;
	uio_iov = uio->uio_iov;
	uio_iovdcnt = uio->uio_iovdcnt;
	uio_xmem = uio->uio_xmem;
	iov_len = 0;
        
        /*
         * Increment the appropriate counter for the physical read or
         * write.
         */
        if (rw == B_READ)
#ifdef _POWER_MP
		fetch_and_add(&sysinfo.phread, 1);	/* sysinfo.phread++ */
#else /* _POWER_MP */
		sysinfo.phread++;
#endif /* _POWER_MP */
        else
#ifdef _POWER_MP
		fetch_and_add(&sysinfo.phwrite, 1);	/* sysinfo.phwrite++ */
#else /* _POWER_MP */
		sysinfo.phwrite++;
#endif /* _POWER_MP */
        
	/*
	 * This loop is performed until the entire i/o is
	 * processed or an error occurs.
	 */
	while ((uio_iovcnt > 0) || (iov_len > 0))
	{

		/*
		 * Setup the parameters for the next iov.
		 */
		if (iov_len <= 0)
		{
			iov_len = uio_iov->iov_len;
			iov_base = uio_iov->iov_base;
			uio_iovcnt--;
			uio_iov++;
			uio_iovdcnt++;
			uio_xmem++;
			if (iov_len <= 0)
				continue;
		}

		/*
		 * Allocate a buffer header and initialize it if this
		 * is not a retry due to pin memory limitations.
		 */
		if (bp == (struct buf *)NULL)
		{
			/*
			 * Get a buffer header.  First check if any from the initial
			 * allocate are available.  If so, use one of them.  If not,
			 * check if there are any that have been returned to the
			 * free list.  Assert if there are none.
			 */
			if (nbp == mbp)
			{
				/*
		 		 * Start the i/o if no more free buffer 
				 * headers are available.
		 		*/
				if (fbp == (struct buf *)NULL)
				{
					sbp->av_back = (struct buf *)NULL;
					uphystart(strat, sbp, &abp);
					sbp = (struct buf *)NULL;
					uphyswait(&abp, &fbp, &ebp, uio->uio_segflg);
					if (ebp != (struct buf *)NULL)
						break;
				}
				ASSERT(fbp != (struct buf *)NULL);
				bp = fbp;
				fbp = fbp->av_forw;
			}
			else
			{
				bp = nbp - 1;
				nbp = nbp - 1;
			}
						
			/*
			 * Initialize the buffer header.
			 */
			bp->b_flags = B_BUSY | B_MPSAFE_INITIAL | rw;
			bp->av_forw = (struct buf *)NULL;
			bp->av_back = (struct buf *)NULL;
			bp->b_iodone = uphysdone;
			bp->b_vp = (struct vnode *)NULL;
			bp->b_dev = dev;
			bp->b_blkno = uio_offset >> UBSHIFT;
			bp->b_baddr = iov_base;
			bp->b_bcount = iov_len;
			bp->b_error = 0;
			bp->b_resid = 0;
			bp->b_work = 0;
			bp->b_options = 0;
			bp->b_event = EVENT_NULL;
			bp->b_start.tv_sec = 0;
			bp->b_start.tv_nsec = 0;
			bp->b_xmemd.aspace_id = XMEM_INVAL;
		}

		/*
		 * Allow the transfer to be broken apart as per any
		 * device dependent requirements.
		 */
		min_error = (*mincnt)(bp, minparms);
		if (min_error != 0)
		{
			if (ebp == (struct buf *)NULL)
			{
				ebp = bp;
				ebp->b_flags |= B_ERROR;
				ebp->b_error = min_error;
			}
			break;
		}

		/*
		 * Make the buffer accessable to the
		 * device driver's interrupt handler.
		 */
		if (uio->uio_segflg == UIO_XMEM)
		{
			bp->b_xmemd = *uio_xmem;
		}
		else
		{
			rc = xmattach(bp->b_baddr, bp->b_bcount, &(bp->b_xmemd), uio->uio_segflg);
			if (rc != XMEM_SUCC)
			{
				if (ebp == (struct buf *)NULL)
				{
					ebp = bp;
					ebp->b_flags |= B_ERROR;
					ebp->b_error = EFAULT;
				}
				break;
			}
		}

		/*
		 * Pin the buffer. Pinning the buffer prevents the pager from
		 * altering the access rights to a page to any value other
		 * than that expected by the application.
		 *
		 * The rest of this raw i/o operation will be performed one
		 * request at a time when the request can not be pinned due
		 * to insufficient memory. Note that this is achieved by
		 * ensuring bp is the only buffer header used for the rest
		 * of the raw i/o operation. The rest of the free list is lost,
		 * but still recoverable by the call to xmfree at the end.
		 */
		rc = xmempin(bp->b_baddr, bp->b_bcount, &(bp->b_xmemd));
		if (rc != 0)
		{
			(void) xmdetach(&(bp->b_xmemd));
			if (rc == ENOMEM)
			{
				bp->b_bcount = ((bp->b_bcount + (UBSIZE - 1)) >> 1) & ~(UBSIZE - 1);
				while (abp != (struct buf *)NULL)
				{
					fbp = (struct buf *)NULL;
					uphyswait(&abp, &fbp, &ebp, uio->uio_segflg);
				}
				fbp = (struct buf *)NULL;
				if (bp->b_bcount != 0)
					continue;
			}
			if (ebp == (struct buf *)NULL)
			{
				if (rc == ERANGE)
					rc = EAGAIN;
				ebp = bp;
				ebp->b_flags |= B_ERROR;
				ebp->b_error = rc;
			}
			break;
		}

		/*
		 * Update the uio structure based on the amount just
		 * processed.
		 */
		iov_len -= bp->b_bcount;
		iov_base += bp->b_bcount;
		uio_offset += bp->b_bcount;

		/*
		 * Put this request at the end of the list of pending
		 * requests.
		 */
		if (sbp == (struct buf *)NULL)
		{
			sbp = bp;
			sbp->av_back = sbp;
		}
		else
		{
			sbp->av_back->av_forw = bp;
			sbp->av_back = bp;
		}
		lbp = bp;
		bp = (struct buf *)NULL;
		buf_snt_cnt++;

	}

	/*
	 * Start any pending requests. This is typically the way the last
	 * request is initiated. It also is the way any pending requests
	 * are started when an error is detected in the above loop. These
	 * should be processed because all i/o up to the error is expected
	 * to be performed.
	 */
	if (sbp != (struct buf *)NULL)
	{
		sbp->av_back = (struct buf *)NULL;
		uphystart(strat, sbp, &abp);
		sbp = (struct buf *)NULL;
	}

	/*
	 * Wait for all of the requests to complete processing.
	 */
	while (abp != (struct buf *)NULL)
	{
		fbp = (struct buf *)NULL;
		uphyswait(&abp, &fbp, &ebp, uio->uio_segflg);
	}

	/*
	 * Indicate how much of the operation was performed and
	 * if an error occured.
	 */
	if (ebp != (struct buf *)NULL)
	{
		if (buf_snt_cnt != 1)
			done = (((offset_t) ebp->b_blkno << UBSHIFT) - 
				uio->uio_offset) +
				(ebp->b_bcount - ebp->b_resid);
		else
			done = ebp->b_bcount - ebp->b_resid;
		uio->uio_offset += done;
		uio->uio_resid -= done;
		error = geterror(ebp);
	}
	else if (lbp != (struct buf *)NULL)
	{
		if (buf_snt_cnt != 1)
			done = (((offset_t) lbp->b_blkno << UBSHIFT) - 
				uio->uio_offset) +
				(lbp->b_bcount - lbp->b_resid);
		else
			done = lbp->b_bcount - lbp->b_resid;
		uio->uio_offset += done;
		uio->uio_resid -= done;
	}

	/*
	 * Free up the buffer headers.
	 */
	rc = xmfree((void *)mbp, pinned_heap);
	ASSERT(rc == 0);

	/*
	 * Return to the caller.
	 */
	return(error);

}   /* end uphysio */
