static char sccsid[] = "@(#)30        1.12  src/bos/kernext/pse/str_subr.c, sysxpse, bos411, 9439C411a 9/27/94 09:06:16";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      sth_alloc
 *                 sth_free
 *                 q_alloc
 *                 q_free
 *                 sth_set_queue
 *                 sth_muxid_lookup
 *                 sth_iocblk_init
 *                 sth_iocblk_term
 *                 sth_iocblk 
 *                 sth_link_alloc
 *                 sth_read_reset
 *                 sth_read_seek
 *                 close_wrapper
 *                 open_wrapper
 *                 sth_uiomove
 *                 sth_uiodone
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

#include <sys/errno.h>

#define SQH_LOCK_COUNT
#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <sys/stropts.h>

STHP
sth_alloc()
{
	STHP	sth;
static	STH	sth_template;

	NET_MALLOC(sth, STHP, sizeof *sth, M_STRHEAD, M_WAITOK);
	*sth = sth_template;
	sth->sth_pollq.next = sth->sth_pollq.prev = (POLLQP)&sth->sth_pollq;
	sth->sth_sigsq.next = sth->sth_sigsq.prev = (SIGSQ *)&sth->sth_sigsq;
	return sth;
}

void
sth_free (sth)
	STHP	sth;
{
#if	MACH_ASSERT
	if (sth->sth_ext_flags)
		panic("sth_free sth_ext_flags");
#endif
	NET_FREE(sth, M_STRHEAD);
}

queue_t *
q_alloc ()
{
reg	queue_t	* q;
static	queue_t	q_template;
static  short q_qlock_count = 0;
DISABLE_LOCK_DECL
extern simple_lock_data_t streams_open_lock;

	NET_MALLOC(q, queue_t *, sizeof(queue_t) * 2, M_STRQUEUE, M_WAITOK);
	q[0] = q_template;
	q[1] = q_template;
	q[0].q_other = &q[1];
	q[1].q_other = &q[0];

	/* Allocate the synch queues */
	NET_MALLOC(q[0].q_runq_sq, SQP, 2 * sizeof (SQ), M_STRSQ, M_WAITOK);
	q[1].q_runq_sq = &(q[0].q_runq_sq)[1];

	/* Initialize the Read queue */
	q->q_flag = QUSE | QREADR | QWANTR;
	runq_sq_init(q);
	DISABLE_LOCK(&streams_open_lock);
	sqh_init(&q->q_sqh);
        lock_alloc((&q->q_qlock), LOCK_ALLOC_PIN, PSE_Q_LOCK, q_qlock_count++);
	DISABLE_UNLOCK(&streams_open_lock);
	simple_lock_init(&q->q_qlock);

	/* Initialize the Write queue */
	q = WR(q);
	q->q_flag = QUSE | QWANTR;
	runq_sq_init(q);
	DISABLE_LOCK(&streams_open_lock);
	sqh_init(&q->q_sqh);
        lock_alloc((&q->q_qlock), LOCK_ALLOC_PIN, PSE_Q_LOCK, q_qlock_count++);
	DISABLE_UNLOCK(&streams_open_lock);
	simple_lock_init(&q->q_qlock);

	return RD(q);
}

int
q_free (q)
	queue_t	* q;
{
	int band;

	for (band = q->q_nband; band >= 0; --band)
		flushband(q, band, FLUSHALL|FLUSH_CAN_CLOSE);
	for (band = WR(q)->q_nband; band >= 0; --band)
		flushband(WR(q), band, FLUSHALL);

	if (q[0].q_bandp)
		NET_FREE(q[0].q_bandp, M_STRQBAND);
	if (q[1].q_bandp)
		NET_FREE(q[1].q_bandp, M_STRQBAND);
#if	MACH_ASSERT
	if (q[0].q_runq_sq->sq_flags || q[1].q_runq_sq->sq_flags)
		panic("q_free q_runq_sq");
#endif

	NET_FREE(q[0].q_runq_sq, M_STRSQ);

	/* we assume that q is the read queue first */
	sqh_term(&q->q_sqh);
	lock_free(&q->q_qlock);

	sqh_term(&(WR(q))->q_sqh);	
	lock_free(&(WR(q))->q_qlock);

	NET_FREE(q, M_STRQUEUE);
	return 0;
}

void
sth_set_queue (q, rinit, winit)
reg	queue_t		* q;
	struct qinit	* rinit;
	struct qinit	* winit;
{
reg	struct module_info * mi;

	if ((q->q_qinfo = rinit) &&  (mi = rinit->qi_minfo)) {
		q->q_minpsz = mi->mi_minpsz;
		q->q_maxpsz = mi->mi_maxpsz;
		q->q_hiwat = mi->mi_hiwat;
		q->q_lowat = mi->mi_lowat;
	}
	q = WR(q);
	if ((q->q_qinfo = winit) &&  (mi = winit->qi_minfo)) {
		q->q_minpsz = mi->mi_minpsz;
		q->q_maxpsz = mi->mi_maxpsz;
		q->q_hiwat = mi->mi_hiwat;
		q->q_lowat = mi->mi_lowat;
	}
}

STHPP
sth_muxid_lookup (sth, muxid, is_persistent)
reg	STHP	sth;
	int	muxid;
	int	is_persistent;
{
reg	STHPP	sthpp;

	sthpp = is_persistent ? &sth->sth_pmux_top : &sth->sth_mux_top;
	if (muxid != MUXID_ALL) {
		for ( ; sth = *sthpp; sthpp = &sth->sth_mux_link) {
			if (sth->sth_muxid == muxid)
				break;
		}
	}
	return sthpp;
}

static	int			iocblk_id;
static simple_lock_data_t iocblk_id_lock = {SIMPLE_LOCK_AVAIL};

void
sth_iocblk_init()
{
	lock_alloc((&iocblk_id_lock), LOCK_ALLOC_PIN, PSE_IOCBLK_LOCK, -1);
	simple_lock_init(&iocblk_id_lock);
}

void
sth_iocblk_term()
{
	lock_free(&iocblk_id_lock);
}

int
sth_iocblk()
{
	int id;

	SIMPLE_LOCK(&iocblk_id_lock);
	if ((id = ++iocblk_id) == 0)
		id = ++iocblk_id;
	SIMPLE_UNLOCK(&iocblk_id_lock);
	return id;
}

MBLKP
sth_link_alloc (osr, cmd, muxid, qtop, qbot)
	OSRP	osr;
	int	cmd;
	int	muxid;
	queue_t	* qtop;
	queue_t	* qbot;
{
	MBLKP	mp;
	struct iocblk	* iocp;
	struct linkblk	* linkp;

	if (!(mp = allocb(sizeof(struct iocblk), BPRI_MED)))
		return nil(MBLKP);
	if (!(mp->b_cont = allocb(sizeof(struct linkblk), BPRI_MED))) {
		freeb(mp);
		return nil(MBLKP);
	}
	mp->b_datap->db_type = M_IOCTL;
	iocp = (struct iocblk *)mp->b_rptr;
	mp->b_wptr += sizeof(struct iocblk);
	iocp->ioc_cmd = cmd;
	iocp->ioc_cr = osr->osr_creds;
	iocp->ioc_id = sth_iocblk();
	iocp->ioc_count = sizeof(struct linkblk);
	iocp->ioc_error = 0;
	iocp->ioc_rval = 0;
	linkp = (struct linkblk *)mp->b_cont->b_rptr;
	mp->b_cont->b_wptr += sizeof(struct linkblk);
	linkp->l_index = muxid;
	linkp->l_qtop = qtop;
	linkp->l_qbot = qbot;
	return mp;
}

int
sth_read_reset (osr)
	OSRP	osr;
{
	osr->osr_rw_count  = osr->osr_rw_uio->uio_resid;
	osr->osr_rw_offset = 0;
	osr->osr_rw_total  = 0;
	return 0;
}

int
sth_read_seek (osr, whence, offset)
	OSRP	osr;
	int	whence;
	long	offset;
{
	long newoffset;

	switch (whence) {
	case 0:
		newoffset = offset;
		break;
	case 1:
		newoffset = osr->osr_rw_offset + offset;
		break;
	case 2:
		newoffset = (osr->osr_rw_offset + osr->osr_rw_uio->uio_resid) + offset;
		break;
	default:
		return EINVAL;
	}
	if (newoffset >= 0 && newoffset < osr->osr_rw_uio->uio_resid) {
		osr->osr_rw_offset = newoffset;
		return 0;
	}
	return EINVAL;
}

int
close_wrapper (args)
	struct open_args *args;
{
	int error;


	if (args->a_func == 0)
		return 0;
	if (args->a_queue->q_flag & QOLD)
		error = (*(strclose_V3)(args->a_func))(args->a_queue);
	else
		error = (*(strclose_V4)(args->a_func))
			(args->a_queue, args->a_fflag, args->a_creds);
	/* sanity check */
	switch (error) {
	default:
		if (error >= 0 && error < 128)
			break;
	case ERESTART:
STR_DEBUG(printf("Streams close: would have returned %d\n", error));
		/* Nothing to restart */
		error = (error == ERESTART) ? EINTR : 0;
	}
	return error;
}

int
open_wrapper (args)
	struct open_args *args;
{
	int error;

	 ENTER_FUNC(open_wrapper, args, 0, 0, 0, 0, 0);

	if (args->a_func == 0) {
		LEAVE_FUNC(open_wrapper, ENXIO);
		return ENXIO;
	}
	if (args->a_queue->q_flag & QOLD) {
		int dev;
		/*
		 * V3.2 u.u_error compatibility hack. See sys/stream.h.
		 */
		setuerror(0);
		dev = (*(stropen_V3)(args->a_func))
			(args->a_queue, args->a_devp ? *args->a_devp : NODEV,
				args->a_fflag, args->a_sflag);
		if (dev == OPENFAIL) {
			if (!(error = getuerror())) error = ENXIO;
		} else {
			error = 0;
			if (args->a_devp)
				*args->a_devp = dev;
		}
	} else
		error = (*(stropen_V4)(args->a_func))
				(args->a_queue, args->a_devp,
				args->a_fflag, args->a_sflag, args->a_creds);
	/* sanity check */
	switch (error) {
	default:
		if (error >= 0 && error < 128)
			break;
	case ERESTART:
STR_DEBUG(printf("Streams open: would have returned %d\n", error));
		/* Too dangerous to restart with a pathological driver */
		error = (error == ERESTART) ? EINTR : ENXIO;
	}
	LEAVE_FUNC(open_wrapper, error);
	return error;
}

/*
 * sth_uiomove() and sth_uiodone().
 *
 * Like uiomove, except they defer altering the iov list until
 * the copies are complete. This is needed for sth_read_seek
 * and sth_read_reset, and used only in osr_read() and osr_write().
 */
int
sth_uiomove (cp, n, osr)
	register caddr_t cp;
	register int n;
	OSRP osr;
{
	register struct iovec *iov;
	register struct uio *uio;
	u_int cnt, resid, offset;
	int error = 0;

	uio = osr->osr_rw_uio;
	if (osr->osr_rw_rw != UIO_READ && osr->osr_rw_rw != UIO_WRITE)
		panic("sth_uiomove: mode");
	resid = osr->osr_rw_count;
	offset = osr->osr_rw_offset;
	iov = uio->uio_iov;
	while (n > 0 && resid > 0) {
		while (offset >= iov->iov_len) {
			offset -= iov->iov_len;
			iov++;
		}
		cnt = iov->iov_len - offset;
		if (cnt > n)
			cnt = n;
		switch (uio->uio_segflg) {

		case UIO_USERSPACE:
		case UIO_USERISPACE:
			if (osr->osr_rw_rw == UIO_READ)
				error = copyout(cp, iov->iov_base + offset, cnt);
			else
				error = copyin(iov->iov_base + offset, cp, cnt);
			if (error)
				return (error);
			break;

		case UIO_SYSSPACE:
			if (osr->osr_rw_rw == UIO_READ)
				bcopy((caddr_t)cp, iov->iov_base + offset, cnt);
			else
				bcopy(iov->iov_base + offset, (caddr_t)cp, cnt);
			break;
		}
		iov++;
		osr->osr_rw_count -= cnt;
		resid -= cnt;
		osr->osr_rw_offset += cnt;
		offset = 0;
		osr->osr_rw_total += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

void
sth_uiodone (osr)
	OSRP	osr;
{
	struct uio *uio = osr->osr_rw_uio;
	struct iovec *iov = uio->uio_iov;

	uio->uio_resid -= osr->osr_rw_total;
	uio->uio_offset += osr->osr_rw_total;
	while (osr->osr_rw_total > iov->iov_len) {
		osr->osr_rw_total -= iov->iov_len;
		iov->iov_len = 0;
		iov++;
	}
	iov->iov_len -= osr->osr_rw_total;
	iov->iov_base += osr->osr_rw_total;
}
