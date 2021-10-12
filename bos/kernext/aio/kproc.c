static char sccsid[] = "@(#)03  1.1.1.10  src/bos/kernext/aio/kproc.c, sysxaio, bos41J, 9521A_all 5/23/95 10:30:14";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: server_main, do_requests, rw_request
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"
#include <fcntl.h>
#include <sys/uio.h>

static int do_requests(server *sp);
static void rw_request(request *rp, int *kfp);

/* server_main -- main entry point for the kproc
 */
void
server_main(int data, server **spp, int plen)
{
	DBGPRINTF(DBG_SERVER, ("%d SERVER STARTING\n", (int) thread_self()));
	
	(void)setpinit();

	if (setsid() == -1)
		DPRINTF(("%d server setsid failed\n", thread_self()));

	/*
	 * get/set the thread id of the kproc
	 */
	(*spp)->tid = thread_self();

	if (spp && *spp) {
		
		/*
		 *  Continually wait on the queue becoming non-empty.
		 *  Call do_requests() when the queue is non-empty.
		 *  Exit if do_requests dequeues a terminate request.
		 */
		
		do
			(void)et_wait(EVENT_AQUEUE, EVENT_AQUEUE,
				     EVENT_SHORT);
		while (do_requests(*spp));
		
		free(*spp);
	}
}

static
int
do_requests(server *sp)
{
	int		killflag;
	request	       *rp;
	queue	       *qp = sp->qp;
	queue	       *new_queue;
	int		i;
	suspender       *susp;

	ASSERT(qp->s_count > 0);

	QUEUE_LOCK(qp);
      top:
	ASSERT(qp->s_count > 0);

	while (rp = get_request(qp)) {
		ASSERT(rp->inprog == QUEUED);
		assert(rp->fp->f_count > 0);
		DEBUG_DISP_TOP(rp->reqtype);

		/* Clear any pending signals */
		while(sig_chk());

		killflag = 0;
		rp->inprog = STARTED;

		QUEUE_UNLOCK(qp);

		switch (rp->reqtype) {
		      case QREADREQ:
		      case QWRITEREQ:
			rw_request(rp, &killflag);
			break;
		      default:
			/* blow up */
			DPRINTF(("do_requests -- bad type: %d\n", rp->reqtype));
			PANIC("do_requests: bad type");
			break;
		}

		/*
		 * The originator may be e_waiting on this request,
		 * either because of a call of iosuspend, or a call
		 * of close, exit, or exec.
		 */
		QUEUE_LOCK(qp);

		/* update suspend block if necessary    */
		check_suspender(rp);

		/*
		 * Set request state to DONE while we're serialized
		 * on the queue lock.
		 */
		rp->inprog = AIO_DONE;

		QUEUE_UNLOCK(qp);

		/* 
		 * if we just posted an event we shouldn't
		 * also send a signal
		 */
		if (killflag) {
			pidsig(rp->pid, SIGIO);
		}

		/*
		 * if this was the last request of a list
		 *	alert anyone waiting on it
		 *	and deallocate the knot
		 */
		check_knot(rp);

		QUEUE_LOCK(qp);
		delete_request(rp, qp);
		releasereq(rp); /* free the request block */

	} /* while (req = top_request()) */

	ASSERT(qp->s_count > 0);
	qp->s_count--;

	/* 
	 * We've completed all outstanding requests for
	 * this queue. We unlock the queue and look for
	 * any other work we might be able to do.
	 */
	QUEUE_UNLOCK(qp);

	/*
	 * Check to see if any queue is understaffed.
	 * We get no benefit from servers that merely
	 * stand and e_wait.
	 * We must serialize with user procs that may
	 * be trying to add requests to the queues,
	 * since if we miss their request before we
	 * make ourselves available again we may not
	 * get posted to, and we may hang.
	 */
	AIOQ_LOCK();
	if (new_queue = find_work()) {
		if (new_queue != qp) {
			qp = new_queue;
		}
		QUEUE_LOCK(qp);
		AIOQ_UNLOCK();
		ASSERT(qp->s_count >= 0);
		qp->s_count++;
		goto top;
	}

	/*
	 * Nothing to do.
	 * put ourselves back on the avail list
	 * hold the queues locked while we do so someone won't
	 *   add a request to this queue (which we have left)
	 *   check the free list for servers (we haven't arrived yet)
	 *   and notify no one
	 */
	SERVERS_LOCK();
	add_server(sp);
	SERVERS_UNLOCK();
	AIOQ_UNLOCK();

	return 1;
}

static
void
rw_request(request *rp, int *kfp)
{
	struct iovec	iov;
	struct uio	auio;
	enum uio_rw	rw;		/* dir:  UIO_READ or UIO_WRITE	*/
	char	       *buf;
	int		count;
	int             err, rc;

	if (!(buf = (char *)vm_att(rp->bufd.subspace_id,
				   (caddr_t) rp->kaiocb.aio_buf))) {
		DPRINTF(("vm_att(rp->bufd) -> NULL\n"));
		err = -1;
		goto error;
	}
	
	iov.iov_base	= (caddr_t)buf;
	iov.iov_len     = rp->kaiocb.aio_nbytes;
	
	auio.uio_iov	= &iov;
	auio.uio_iovcnt	= 1;
	auio.uio_segflg	= SYS_ADSPACE;
	auio.uio_fmode	= rp->fp->f_flag;
	auio.uio_offset = rp->kaiocb.aio_offset;
	auio.uio_resid  = count = rp->kaiocb.aio_nbytes;
	
	if (rp->reqtype == QREADREQ)
		rw = UIO_READ_NO_MOVE;
	else
		rw = UIO_WRITE_NO_MOVE;
	
	assert(rp->fp->f_count > 0);
	err = fp_rwuio(rp->fp, rw, &auio, 0);

	if (err)
		DPRINTF(("fp_rwuio returned %d\n", err));
	else {
		/*
		 * The number of bytes transferred is
		 * the number requested less the residual.
		 */
		count -= auio.uio_resid;
		
		DBGPRINTF(DBG_IO,
			   ("< %s of %d b rtn=%d\n",
			    rp->reqtype == QREADREQ ? "r" : "w", count, err));
	}
	
      error:
	
	if (buf) {
		vm_det(buf);
	}

	rp->kaiocb.aio_return = err ? -1 : count;
	rp->kaiocb.aio_errno = err;
	
	rc = xmemout((char *)&rp->kaiocb, (char *)rp->aiocbp,
			sizeof(struct aiocb), &rp->aiocbd);
	ASSERT(rc == XMEM_SUCC);

	*kfp = (rp->kaiocb.aio_flag & AIO_SIGNAL);
}
