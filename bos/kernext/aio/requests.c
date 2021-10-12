static char sccsid[] = "@(#)05  1.10  src/bos/kernext/aio/requests.c, sysxaio, bos41J, 9521A_all 5/23/95 10:30:24";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: add_request, get_request, delete_request, find_request
 *	      cancel_request, cancel_fd
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

/*
 * add_request -- add a request to the appropriate queue
 *
 * add the request to the (tail of) the queue
 */
void
add_request(request *rp)
{
	queue  *qp;
	tid_t	s_tid = -1;
	int i;

	qp = find_queue(rp->fp);

	/*
	 * we need to serialize with the kproc to make sure that we
	 * don't enqueue work after it has checked the queue but
	 * before it makes itself available again--this can make it
	 * hang if there are no other servers available, since we
	 * won't post to it.
	 */
	AIOQ_LOCK();

	QUEUE_LOCK(qp);

	/*
	 * add the request to the tail of the queue
	 */
	if (qp->head) {
		qp->tail->next = rp;
		qp->tail = rp;
	} else {
		/* empty queue */
		ASSERT(qp->req_count == 0);
		qp->head = qp->tail = rp;
	}

	/*
	 * note that the queue has another request in it
	 */
	qp->req_count++;

	/*
	 * add another server if we can
	 */
	s_tid = get_free_server(qp);

	/* we can release the queue lock once we've incremented
	   the req_count */
	QUEUE_UNLOCK(qp);

	/*
	 * we can allow the kproc to check the queues now.
	 */
	AIOQ_UNLOCK();

	/*
	 * if we got a server, give it a nudge
	 * we hope it can go right to work since we don't have the
	 * queue locked
	 */
	if (s_tid != -1)
		et_post(EVENT_AQUEUE, s_tid);
}

/*
 * get_request -- return the first free request from the head of the queue
 */
request *
get_request(queue *qp)
{
	request	*rp;
	
	for (rp = qp->head; rp; rp = rp->next)
		if (rp->inprog == QUEUED)
			return rp;
	return NULL;
}

/*
 * delete_request -- remove a request from the queue
 *
 * This routine is only called by the kproc once the request
 * was completed successfully, or by cancel_request().
 */
void
delete_request(request *rp, queue *qp)
{
	request *crp;	/* current request */
	request *prp;	/* previous request */

	/* detach the buf and aiocb which we attached
	 * to in ardwr() when the request was initially
	 * submitted to the request queue.
	 */
	xmdetach(&(rp->bufd));
	xmdetach(&(rp->aiocbd));

	/* decrement the global request counter.
	 */
	fetch_and_add(&requestcount, -1);

	if (qp->head == qp->tail) {
		qp->head = qp->tail = NULL;
		qp->req_count = 0;
		return;
	}

	/* there's more than one request on the queue */
	ASSERT(qp->req_count > 1);
	for (prp = NULL, crp = qp->head; crp; prp = crp, crp = crp->next)
		if (crp == rp) {
			/* we can't be both first and last */
			if (!crp->next) {
				/* last request on the queue */
				ASSERT(crp == qp->tail);
				ASSERT(prp);
				prp->next = NULL;
				qp->tail = prp;
			} else if (!prp)
				/* first request on the queue */
				qp->head = crp->next;
			else
				/* in the middle somewhere */
				prp->next = crp->next;
			qp->req_count--;
			return;
		}
	PANIC("delete_request: couldn't find request");
}

/* find_request -- locate a request
 *
 * return a pointer to the request matching the parameters
 */
request *
find_request(queue *qp, struct aiocb *cbp, pid_t pid)
{
	request	*rp;

	for (rp = qp->head; rp; rp = rp->next)
		if (rp->aiocbp == cbp && rp->pid == pid)
			return rp;

	return NULL;
}

int
cancel_request(pid_t pid, struct file *fp, int fd, struct aiocb *cbp,
	       queue *qp, request *rp)
{
	int		caller_has_lock = (qp != NULL);

	DBGPRINTF(DBG_CANCEL, ("cancel_request: pid %d fd %d", pid, fd));

	/*
	 * if we find the request corresponding to this aiocb
	 * try to cancel it, otherwise assume the aio is
	 * finished, so return AIO_ALLDONE
	 * Of course, don't do any unnecessary work if the
	 * pointers were passed in.
	 */
	if (!qp) {
		ASSERT(!rp);
		qp = find_queue(fp);
		QUEUE_LOCK(qp);
		if (!(rp = find_request(qp, cbp, pid))) {
			QUEUE_UNLOCK(qp);
			DBGPRINTF(DBG_CANCEL, (" alldone\n"));
			return AIO_ALLDONE;
		} else if (rp->fd != fd) {
			QUEUE_UNLOCK(qp);
			/* 
			 * if the file descriptor passed in doesn't 
			 * match the one in the request we found,
			 * we return AIO_NOTCANCELED.
			 */
			return AIO_NOTCANCELED;
		}
	} else {
		ASSERT(rp);
		ASSERT(rp->fd == fd);
	}

	/*
	 * if the kproc's got it we're too late to cancel
	 */
	if (rp->inprog == STARTED) {
		if (!caller_has_lock)
			QUEUE_UNLOCK(qp);
		DBGPRINTF(DBG_CANCEL, (" not canceled\n"));
		return AIO_NOTCANCELED;
	} else if (rp->inprog == AIO_DONE) {
		if (!caller_has_lock)
			QUEUE_UNLOCK(qp);
		DBGPRINTF(DBG_CANCEL, (" alldone\n"));
		return AIO_ALLDONE;
	}
	ASSERT(rp->inprog == QUEUED);

	/*
	 * We're in the originator's context, so we still have the
	 * file opened.
	 */
	ASSERT(rp->fp->f_count > 0);
	
	/* 
	 * if this is the last request in a knot,
	 * we should not send a signal (they can't
	 * be LIO_WAITing, so we needn't worry about posting
	 * an event). we >do< have to deallocate the knot, though
	 */
	delete_request(rp, qp);
	if (!caller_has_lock)
		QUEUE_UNLOCK(qp);

	/*
	 * update the aiocb so they can see it was cancelled
	 */
	rp->kaiocb.aio_return = -1;
	rp->kaiocb.aio_errno = ECANCELED;

	/* at this point we have successfully canceled the request.
	 * we need not check the return code from copyout() because
	 * if it fails, we can't return any other return except
	 * AIO_CANCELLED, returning an error at this point would
	 * cause the aio_close_hook() function to get confused.
	 */
	copyout(&rp->kaiocb, cbp, sizeof(rp->kaiocb));

	/* free the request block
	 */
	releasereq(rp);

	return AIO_CANCELED;
}

int
cancel_fd(pid_t pid, struct file *fp, int fd, int block)
{
	queue	  *qp;
	request	  *rp;
	request	  *rpnext;
	int	   rc = AIO_ALLDONE; /* if we never find anything to cancel */
	int        ret, this_rc;
	ulong	   gen;
	suspender *susp;
	ulong	   evt;
	int	   second_try = 0;
	tid_t           tid = thread_self();    /* tid of this thread */

	DBGPRINTF(DBG_CANCEL, ("cancel_fd: pid %d, fd %d\n", pid, fd));
	qp = find_queue(fp);

      restart:
	QUEUE_LOCK(qp);

      try_again:
	for (rp = qp->head; rp; rp = rpnext)
	{
		rpnext = rp->next;
		if (rp->pid == pid && rp->fp == fp && rp->fd == fd) {
			if (rp->inprog == QUEUED) {
				this_rc = cancel_request(pid, fp, fd, rp->aiocbp,
							 qp, rp);
				ASSERT(this_rc != AIO_NOTCANCELED);
			} else if (rp->inprog == STARTED) {
				this_rc = AIO_NOTCANCELED;
				/*
				 * if we're blocking on failed cancels
				 *	set us up as a suspender
				 *	wait for the kproc to wake us
				 *	if we awoke because of a signal
				 *		give up, but let our caller
				 *		know we failed
				 *	remove and deallocate the suspender
				 *	start over (the queue may have
				 *	changed considerably while we
				 *	were asleep)
				 */
				if (block && second_try) {
					GEN(gen);
					DBGPRINTF(DBG_CANCEL,
						  ("cancel_fd: blocking\n"));
					if (susp = create_suspender(pid, tid, gen)) {
						rp->susp_gen = gen;
						QUEUE_UNLOCK(qp);
						evt = et_wait(EVENT_ASUSPEND,
							     EVENT_ASUSPEND,
							     0);
						remove_suspender(susp);
						ret = xmfree(susp, pinned_heap);
						ASSERT(ret == 0);
						if (evt == EVENT_SIG) {
							DBGPRINTF(DBG_CANCEL,
								  ("cancel_fd: signalled\n"));
							return AIO_NOTCANCELED;
						}
						DBGPRINTF(DBG_CANCEL,
							  ("cancel_fd: restarting\n"));
						goto restart;
					} else
						PANIC("cancel_fd: couldn't suspend\n");
				}
			} else {
				/* pretend the request was already deleted */
				ASSERT(rp->inprog == AIO_DONE);
				continue;
			}
			/*
			 * AIO_NOTCANCELED is "sticky"
			 * otherwise set the return code to the return
			 * for this request (except we don't need to if
			 * the request was already completed)
			 */
			if (rc != AIO_NOTCANCELED)
				rc = this_rc;
		}
	}

	if (rc == AIO_NOTCANCELED && block && !second_try) {
		++second_try;
		DBGPRINTF(DBG_CANCEL, ("cancel_fd: trying again\n"));
		goto try_again;
	}

	QUEUE_UNLOCK(qp);

	return rc;
}
