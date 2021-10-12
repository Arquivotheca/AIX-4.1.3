static char sccsid[] = "@(#)44  1.2  src/bos/kernext/aio/aiopin.c, sysxaio, bos41J, 9520A_all 5/11/95 10:16:52";
 /*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: arl_iodone, fetchbufs, check_suspender, create_suspender,
 *            remove_suspender, arl_suspend, create_knot, untie_knot,
 *            slip_knot, check_knot, destroy_knot, getreq, releasereq,
 *            releasebuf, morebufs
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

/* global data          */
Simple_lock lvm_lock;            /*lock for LVM structures       */

knot_list   knots = {NULL};             /* list of knots */
susp_list   suspenders = {NULL};        /* list of suspenders */

struct hash_req lvmio[NHREQ];    /* hash table for active lvm requests */

/* buf pool for all strategy-directed requests       */
struct buf     *freebufp;               /* buf free list */
int     freebufsize = 0;                /* size of buf free list */

/* request block pool for all strategy-directed requests       */
struct request *freereqp;               /* request free list */
int     freereqsize = 0;                /* size of request free list */

/* knot pool for all lio requests       */
struct lio_knot *freeknotp;             /* knot free list */
int     freeknotsize = 0;               /* size of free knot pool     */

/***************************************************************************
 * NAME: arl_iodone
 *
 * FUNCTION: iodone routine for async LVM requests
 *
 *
 * EXECUTION ENVIRONMENT: INTIODONE - this routine must be pinned and cannot
 *                        cause page faults.
 *
 * RETURNS:
 *
 *
 *
 ***************************************************************************
 */
void
arl_iodone(struct buf *bufp)
{
	int     rc;
	request *rp;            /* request block pointer        */
	int     oldpri;         /* process execution priority           */

	rp = (struct request *) bufp->b_forw;  /* access request block */

	/* unpin the user buffer section   */
	rc = xmemunpin(bufp->b_baddr, bufp->b_bcount, &(bufp->b_xmemd));
	ASSERT (rc == 0);

	/* we must serialize the buffer count in the request on MP
	 * machines since multiple processors could be running a
	 * different buffer for the same request. If the count is not
	 * serialized, we could free a buffer and request twice. This only
	 * happens on MP machines since on UP machines, the iodone()
	 * routines are disabled and so this routine will not be interrupted
	 * for the duration.
	 */
	oldpri = LVM_LOCK();

	rp->buf_cnt -= 1;

	/* Save error data if all user data not transferred, or if
	 * an error has occurred for this section of the I/O request.
	 */
	if ((bufp->b_resid) || (bufp->b_flags & B_ERROR)) {
		/* Save error data if no previous error or this error is
		 * closer to the beginning of the user data buffer
		 */
		if ((rp->kaiocb.aio_errno == 0) ||
		(bufp->b_blkno < rp->b_blkno)) {
		    rp->kaiocb.aio_errno = geterror(bufp);
		    rp->b_bcount = bufp->b_bcount;
		    rp->b_blkno = bufp->b_blkno; /* block offset*/
		    rp->b_resid = bufp->b_resid; /*data not transferred*/
		}
	}

	/* return the buf to the free pool */
	bufp->av_forw = freebufp;
	freebufp = bufp;
	freebufsize += 1;

	/* check for request complete, must have interrupt for every buf */
	if (rp->buf_cnt != 0) {
		LVM_UNLOCK(oldpri);
		return;
	}

	rp->inprog = AIO_DONE;  /* really no reason to do this except for
				 * consistent logic. The request will be
				 * removed before the lvmio lock is
				 * released.
				 */

	/* detach user-space buffer                */
	rc = xmdetach(&(bufp->b_xmemd));
	ASSERT(rc == XMEM_SUCC);

	/* Calculate how much data transferred, or return the error. */
	if (rp->kaiocb.aio_errno) {
		rp->kaiocb.aio_return = -1;
	}
	else {
		rp->kaiocb.aio_return = ((off_t) rp->b_blkno<<UBSHIFT)
		   - rp->kaiocb.aio_offset + rp->b_bcount - rp->b_resid;
	}

	/* update user aiocb            */
	rc = xmemout((char *)&rp->kaiocb, (char *)rp->aiocbp,
				   sizeof(struct aiocb), &rp->aiocbd);
	ASSERT(rc == XMEM_SUCC);

	/* unpin and detach from the user aiocb */
	rc = xmemunpin((caddr_t) rp->aiocbp, sizeof(struct aiocb),
							&(rp->aiocbd));
	ASSERT(rc == 0);

	rc = xmdetach(&(rp->aiocbd));   /* detach from user-space aiocb */
	ASSERT(rc == XMEM_SUCC);

	/* check for a suspend list that contains this request  */
	check_suspender(rp);

	/* requestor may want to be signaled    */
	if (rp->kaiocb.aio_flag & AIO_SIGNAL) {
		pidsig(rp->pid, SIGIO);
	}

	/* check for a 'knot'. If it exists, this means the request
	 * was initiated by lio_listio()
	 */
	check_knot(rp);

	/* remove the request from the hash table */
	remque(rp);

	/* free the request block       */
	rp->next = freereqp;
	freereqp = rp;
	freereqsize += 1;

	LVM_UNLOCK(oldpri);
	return;

} /* end arl_iodone     */


/***************************************************************************
 * NAME: fetchbufs
 *
 * FUNCTION: get unused bufs for a request and add the request to the
 *           global hash table.
 *
 * EXECUTION ENVIRONMENT: INTIODONE - this routine must be pinned and
 *                        can not page fault when locked.
 *
 * RETURNS:  NULL - bufs not available
 *
 *           >0   - address of unused buf chain
 *
 ***************************************************************************
 */
struct buf *
fetchbufs(request *rp)
{
	int     oldpri,         /* process execution priority   */
		i, rc;
	struct hash_req *hrq;
	struct  buf  *bp, *abp, *releasebufp = NULL;

	/* serially access the global buf pool and lvm request list     */
	oldpri = LVM_LOCK();     /* serialize access to freebufp    */

	/* get the bufs, preferably from the freepool  */
	if (rp->buf_cnt > freebufsize) {  /* nope, have to use xmalloc  */
		LVM_UNLOCK(oldpri); /* xmalloc() will page fault      */
		abp = morebufs(rp->buf_cnt);
		DPRINTF(("called morebufs, abp=%x, bufcnt=%d\n", abp, rp->buf_cnt));
		if (abp == NULL)
			return NULL;
		LVM_LOCK();
	}
	else {  /* get from the buf pool */
		abp = freebufp;    /* grab them off the top       */
		for (i=rp->buf_cnt; i>0; --i) {
			if (i == 1)
				bp = freebufp; /* end of chain     */
			/* update buf pool pointer      */
			freebufp = freebufp->av_forw;
		}
		bp->av_forw = NULL; /* mark end of returned chain       */

		freebufsize -= rp->buf_cnt; /* update freepool count    */

		/* lower the buf pool to the high-water mark     */
		if (freebufsize > MAXBUFPOOL) {
			releasebufp = freebufp;
			for (i = freebufsize - MAXBUFPOOL; i>0; --i) {
				if (i == 1)
					bp = freebufp;
				freebufp = freebufp->av_forw;
			}
			bp->av_forw = NULL; /* mark end of chain to delete*/
			freebufsize = MAXBUFPOOL;
		}
	}

	/* add the request to the hash table        */
	hrq = ARLHASH((int)rp->aiocbp ^ (int)rp->pid);
	insque( rp, hrq);

	LVM_UNLOCK(oldpri);

	/* need to free some bufs if the pool was too big */
	while (releasebufp) {
		DPRINTF(("releasing bufs, releasebufp = %x\n", releasebufp));
		bp = releasebufp->av_forw;
		rc = xmfree((char *)releasebufp, pinned_heap);
		ASSERT(rc == 0);
		releasebufp = bp;
	}

	return abp;
}       /* end fetchbufs()        */

/*
 * check_suspender -- locate and update a suspender on this request, if
 *                    necessary.
 *
 * Look through the suspend list for a suspender matching this
 * request (limit one per customer).
 *
 * If we find one, note that we did by putting the address of the
 * aiocb in the suspender.  We use this so we won't post another
 * event to the same suspender later.  The suspender uses it to
 * know which aio completed.
 *
 * This routine must always execute at priority INTIODONE, to serialize
 * access with the iodone routine.
 *
 *  RETURNS: nothing
 *
 */
void
check_suspender(request *rp)
{
	int oldpri;     /* caller's execution priority  */
	suspender *susp;

	oldpri = SUSP_LOCK();
	if (rp->susp_gen) {
		for (susp = suspenders.list; susp; susp = susp->next) {
		/* if this suspender is the first to match*/
			if (susp->pid == rp->pid &&
			    susp->gen == rp->susp_gen &&
			    susp->aiocbp == NULL) {
				/* stash the completed aiocbp so
				 * we won't do this again, and
				 * the iosuspender will know who completed
				 */
				susp->aiocbp = rp->aiocbp;
				et_post(EVENT_ASUSPEND, susp->tid);
				break;
			}
		}
	}
	/*
	 * Clear the request->aiocbp so that another thread will not wait
	 * in aio_suspend() for this request to complete. The request is
	 * not deleted until after the suspend lock is released.
	 */
	rp->aiocbp = NULL;

	SUSP_UNLOCK(oldpri);
	return;
}       /* end check_suspender */

/* create_suspender -- allocate a suspender, initialize it
 *                     and place it on the suspender list
 *
 * RETURNS: NULL - no memory
 *          > 0  - address of suspender
 */
suspender *
create_suspender(pid_t pid, tid_t tid, ulong gen)
{
	suspender *susp;
	int     oldpri;     /*    caller's execution priority     */

	/* allocate the new record */
	if (!(susp = (suspender *) xmalloc(sizeof(suspender), 0,pinned_heap)))
		return NULL;

	/* initialize it */
	susp->tid = tid;
	susp->pid = pid;
	susp->aiocbp = NULL;
	susp->gen = gen;

	oldpri = SUSP_LOCK();

	/* put it on the list of suspenders */
	susp->next = suspenders.list;
	susp->prev = NULL;
	suspenders.list = susp;
	/* if the list was not empty, adjust the back pointer */
	if (susp->next) {
		susp->next->prev = susp;
	}
	SUSP_UNLOCK(oldpri);
	DPRINTF(("suspend block inserted top of chain, susp %x\n", susp));
	return susp;
}

/* remove_suspender -- remove a suspender from the suspender list
 *
 * This routine must not cause page faults when executing.
 */
void
remove_suspender(suspender *susp)
{

       int oldpri;

	/*
	 * take the suspenders lock, to serialize removal of the
	 * suspender from the list.
	 */
	oldpri = SUSP_LOCK();

	if (!susp->next && !susp->prev)         /* only item on list */
		suspenders.list = NULL;
	else if (!susp->next)                   /* last item on list */
		susp->prev->next = NULL;
	else if (!susp->prev) {                 /* first item on list */
		susp->next->prev = NULL;
		suspenders.list = susp->next;
	} else {                                /* somewhere in middle */
		susp->next->prev = susp->prev;
		susp->prev->next = susp->next;
	}
	SUSP_UNLOCK(oldpri);
	DPRINTF(("suspend removed from chain, chain head = %x\n", susp->prev));
	return;
} /* end remove_suspender       */

/* This routine searches for an active request on the list of LVM requests.
 * For consistency, it must use the same criteria as 'find_request' does for
 * requests on the kproc path. Currently, find_request checks that the
 * request was submitted by the same process as the thread that calls this
 * routine.A more general design would allow the request to be submitted
 * by any process.
 */
int
arl_suspend(suspender *susp, struct aiocb *aiocbp, pid_t pid)
{
	struct hash_req *hrq;
	struct request  *rp;
	int oldpri;

	oldpri = LVM_LOCK();

	/* get the hash anchor from the hash table      */
	hrq = ARLHASH((int) aiocbp ^ (int) pid);

	/* locate the request   */
	for (rp = hrq->next; rp != (request *)hrq; rp = rp->next) {
		if ((rp->aiocbp == aiocbp) && (rp->pid == pid)) {
			rp->susp_gen = susp->gen;
			LVM_UNLOCK(oldpri);
			return 1;
		}
	}
	LVM_UNLOCK(oldpri);
	return 0;
}

lio_knot *
create_knot(int cmd, int nent)
{
	int      oldpri;
	lio_knot *lkp;

	oldpri = KNOTS_LOCK();
	/* get an lio_knot, preferably from the freepool  */
	if (freeknotsize) {
		lkp = freeknotp;
		freeknotp = lkp->next;
		freeknotsize -=1;
	}
	else {  /* need to get one from the pinned heap */
		KNOTS_UNLOCK(oldpri);
		lkp = (lio_knot *) xmalloc(sizeof(lio_knot),0,pinned_heap);
		DPRINTF(("knot xmalloced, lkp = %x\n", lkp));
		if (lkp == NULL) {
			return NULL;
		}
		oldpri=KNOTS_LOCK();
	}

	/* initialize the new record */
	lkp->cmd = cmd;
	lkp->count = nent;
	lkp->interested = 1;

	/*
	 * Put it on the list of knots.
	 * take the global knots lock to serialize addition and deletion
	 * of knots to the list of knots.
	 */
	lkp->next = knots.list;
	lkp->prev = NULL;
	knots.list = lkp;
	/* if the list was not empty, adjust the back pointer */
	if (lkp->next)
		lkp->next->prev = lkp;
	KNOTS_UNLOCK(oldpri);
	DPRINTF(("knot added to list, lkp = %x\n", lkp));
	return lkp;
}

/*
 * untie_knot -- decrement the number of requests tied in a knot.
 *
 *               This routine executes at priority INTIODONE. Obtain
 *               the knot_list lock before calling.
 *
 *               The thread that decrements the knot count field to
 *               zero must delete the knot.
 *
 * RETURNS: 1 - knot was destroyed
 *          0 - knot exists and still on the list
 */
int
untie_knot(lio_knot *lkp, int num)
{
	int     oldpri;

	oldpri=KNOTS_LOCK(); /* serialize the count field       */
	lkp->count -= num;
	if (lkp->count == 0) {
		destroy_knot(lkp);
		KNOTS_UNLOCK(oldpri);
		return 1;    /* knot was destroyed   */
	}
	KNOTS_UNLOCK(oldpri);
	return 0;     /* knot still there     */
} /* end untie_knot     */

/* slip_knot -- If a knot exists, flag it so the kprocs won't
 * post this thread. This routine does not decrement the count,
 * therefore does not delete the knot.
 */
void
slip_knot(lio_knot *lkp)
{
	lio_knot *kp;
	int      oldpri;

	oldpri=KNOTS_LOCK();
	for (kp = knots.list; kp; kp = kp->next) {
		if (kp == lkp) {
			kp->interested = 0;
			KNOTS_UNLOCK(oldpri);
			return; /* knot was found       */
		}
	}
	KNOTS_UNLOCK(oldpri);
	return;       /* knot was not found     */
}

void
check_knot(request *rp)
{
	int oldpri;     /* caller's priority    */

	oldpri = KNOTS_LOCK();

	/* Is this the last request in a list?  */
	if (rp->knotp && --rp->knotp->count == 0) {
		/* Check that a signal didn't knock the requestor out of
		the et_wait. */
		if (rp->knotp->interested) {
			if (rp->knotp->cmd == LIO_WAIT)
				et_post(EVENT_ASUSPEND, rp->tid);
			else if (rp->knotp->cmd == LIO_ASIG)
				pidsig(rp->pid, SIGIO);
		}
		destroy_knot(rp->knotp);
	}
	KNOTS_UNLOCK(oldpri);
	return;
}

/* This routine executes at priority INTIODONE. The knots_lock must be
 * obtained before execution.
 */
void
destroy_knot(lio_knot *lkp)
{

	if (!lkp->next && !lkp->prev)           /* only item on list */
		knots.list = NULL;
	else if (!lkp->next)                    /* last item on list */
		lkp->prev->next = NULL;
	else if (!lkp->prev) {                  /* first item on list */
		lkp->next->prev = NULL;
		knots.list = lkp->next;
	} else {                                /* somewhere in middle */
		lkp->next->prev = lkp->prev;
		lkp->prev->next = lkp->next;
	}

	/* free the lio_knot, return it to the knot pool        */
	lkp->next = freeknotp;
	freeknotp = lkp;
	freeknotsize += 1;

	return;
}

/* get a request from the request block pool    */
request *
getreq()
{
	int     i, rc, oldpri;
	struct request *nextrp, *releasereqp = NULL, *rp = NULL;

	/* check the request block pool       */
	oldpri = LVM_LOCK();     /* serialize with iodone routine*/
	if (freereqsize) {
		rp = freereqp;
		freereqp = rp->next;
		freereqsize -= 1;
		/* lower the pool to the high-water mark     */
		if (freereqsize > MAXREQPOOL) {
			releasereqp = freereqp;
			for (i = freereqsize - MAXREQPOOL; i>0; --i) {
				nextrp = freereqp->next;
				if (i == 1) /* mark the end of chain */
					freereqp->next = NULL;
				freereqp = nextrp;
			}
			freereqsize = MAXREQPOOL;
		}
	}
	LVM_UNLOCK(oldpri);
	if (rp == NULL) {
		rp = (request *) xmalloc(sizeof(request), 0, pinned_heap);
	}

	/* need to free some space if the pool was too big */
	while (releasereqp) {
		nextrp = releasereqp->next;
	     DPRINTF(("request block freed, releasereqp = %x\n",releasereqp));
		rc = xmfree((char *)releasereqp, pinned_heap);
		ASSERT(rc == 0);
		releasereqp = nextrp;
	}

	return rp;
}

/* return request block to the request block pool       */
void
releasereq(request *rp)
{
	int oldpri;

	oldpri = LVM_LOCK();
	rp->next = freereqp;
	freereqp = rp;
	freereqsize += 1;
	LVM_UNLOCK(oldpri);
	return;
}

/* return a buf to the buf pool                         */
void
releasebuf(struct buf *bp)
{
	int oldpri;

	oldpri = LVM_LOCK();     /* serialize access to freebufp    */
	bp->av_forw = freebufp;
	freebufp = bp;
	freebufsize += 1;
	LVM_UNLOCK(oldpri);
	return;
}

/* bufs must all be allocated individually, since they will need to
 * be freed individually if the buf pool gets huge.
 */
struct buf *
morebufs(int count)
{
	int     i, rc;
	struct buf   *bp, *bpp = NULL;

	/* allocate bufs separately     */
	for (i = 0; i < count; ++i) {
		bp = (struct buf *) xmalloc(sizeof(struct buf),
							 0, pinned_heap);
		if (bp == NULL) { /* config has failed    */
			/* free allocated bufs  */
			while (bpp) {
				bp = bpp->av_forw;
				rc = xmfree((caddr_t) bpp, pinned_heap);
				ASSERT(rc == 0);
				bpp = bp;
			}
			return NULL;
		}
		else {  /* chain with av_forw */
			bp->av_forw = bpp;
			bpp = bp;
		}
	}
	return bp;
}
