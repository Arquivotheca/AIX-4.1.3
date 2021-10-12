static char sccsid[] = "@(#)28  1.3  src/bos/kernext/dlpi/driver.c, sysxdlpi, bos41J, 9521B_all 5/25/95 15:54:02";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: EQ
 *		cb_iocroute
 *		dlpiclose
 *		dlpiconfig
 *		dlpictl
 *		dlpiopen
 *		dlpirsrv
 *		dlpitune
 *		dlpiwput
 *		dlpiwsrv
 *		nddget
 *		nddinit
 *		nddrele
 *		nddterm
 *		settune
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * driver.c - dlpi streams driver framework
 *
 * public routines:
 *	dlpiconfig - configuration entry point
 * private routines:
 *	dlpiopen - initialize DLPI stream
 *	dlpiclose - terminate DLPI stream
 *	dlpirsrv - handle upstream flow control; stop interrupt path
 *	dlpiwput - handle downstream messages
 *	dlpiwsrv - handle downstream DLPI messages
 *	dlpictl - handle M_IOCTL and M_CTL messages and send proper replies
 *	dlpitune - alter per-stream tunable parameters
 *	nddinit - associate a CDLI driver with DLPI
 *	nddterm - dissociate a CDLI driver from DLPI
 *	nddget - gain a reference to a provider
 *	nddrele - release a reference to a provider
 */

#include "include.h"

static int dlpiopen(), dlpiclose();
static int dlpiwput(), dlpiwsrv();
static int dlpirsrv();

static void dlpictl(), dlpitune();
static void cb_iocroute();

       int dlpiconfig();
static int nddinit(), nddterm();
static prov_t *nddget();
static void nddrele();

/*
 * driver private data structures
 */

#define	HIWAT	4096
#define	LOWAT	 512

static struct module_info minfo =  { DLPINUM, "dlpi", 0, INFPSZ, HIWAT, LOWAT };
static struct qinit rinit = { 0, dlpirsrv, dlpiopen, dlpiclose, 0, &minfo };
static struct qinit winit = { dlpiwput, dlpiwsrv, 0, 0, 0, &minfo };
static struct streamtab dlpiinfo = { &rinit, &winit };

static prov_t *provh;		/* list of configured CDLI providers */
static DLB *dlbh;		/* list of open DLPI streams */

/*
 * default tunable parameters
 *
 * Note: N1 is variable depending upon the provider; check when tuning.
 */
static llctune_t dl_tune[3] = {
	/*          f,  t1,  t2,  ti,  n1,  n2,  n3,   k */
	/* min */ { 0,   1,   1,   1,   1,   1,   1,   1 },
	/* dft */ { 0,  10,   4, 300,99999, 10,   3,   7 },
	/* max */ { 0, 300, 300, 600,99999, 60, 127, 127 },
};
#define	TMIN	0
#define	TDFT	1
#define	TMAX	2

/*
 * debugging aid: easy to find important variables in a dump
 */

typedef struct dlpivar {
	int	tag;
	prov_t	**provh;	/* ptr to list of configured providers */
	DLB	**dlbh;		/* ptr to list of open streams */
	stats_t	*stats;		/* ptr to global stats */
	struct streamtab *info;	/* streamtab info */
} dlpivar_t;
dlpivar_t dlpivar;
#define	dv dlpivar

/*
 * dlpiopen - initialize DLPI stream
 */

static int
dlpiopen(q, devp, flag, sflag, credp)
	queue_t *q;
	dev_t *devp;
	int flag, sflag;
	cred_t *credp;
{
	DLB *dlb;
	prov_t *p;
	int err;
	timerq_t *t1, *t2, *ti;
	mblk_t *mp;

	if (sflag != CLONEOPEN)
		return ENXIO;

	if (!(p = nddget(major(*devp))))
		return ENXIO;

	if (!(mp = allocb(1, BPRI_HI))) {
		nddrele(p);
		return ENOSR;
	}
	mp->b_datap->db_type = M_ERROR;
	*mp->b_wptr++ = ENOSR;

	/* pre-allocate timers, flow control block */
	t1 = tq_alloc(WR(q), 0x54310000);
	t2 = tq_alloc(WR(q), 0x54320000);
	ti = tq_alloc(WR(q), 0x54690000);
	if (!(t1 && t2 && ti)) {
		err = ENOSR;
error:
		if (ti) tq_free(ti);
		if (t2) tq_free(t2);
		if (t1) tq_free(t1);
		freeb(mp);
		nddrele(p);
		return err;
	}

	/* convenient open tedium handler */
	err = mi_open_comm(&dlbh, sizeof(DLB), q, devp, flag, sflag, credp);
	if (err)
		goto error;

	/* init per-stream info */
	dlb = (DLB *)q->q_ptr;		/* allocated by mi_open_comm() */
	dlb->dlb_tag = TAG_DLB;		/* incore locator tag */
	dlb->dlb_minor = minor(*devp);	/* helpful for tracing */
	dlb->dlb_rq = q;		/* for intr routines */
	dlb->dlb_wq = WR(q);		/* for convenience */
	dlb->dlb_priv = !drv_priv(credp);/* is this a privileged stream? */
	dlb->dlb_prov = p;		/* provider-specific info */
	dlb->dlb_pkt_format = NS_PROTO;	/* default: 4.1 style addresses */
	dlb->dlb_tune = dl_tune[TDFT];	/* default tunables */
	dlb->dlb_state = DL_UNATTACHED;
	dlb->dlb_tq1 = t1;		/* pre-allocated timers */
	dlb->dlb_tq2 = t2;
	dlb->dlb_tqi = ti;
	dlb->dlb_failmp = mp;

	/* finish timer init */
	t1->dlb = dlb;
	t2->dlb = dlb;
	ti->dlb = dlb;

	TRC(dlb, "dlpiopen: dlb 0x%x rq 0x%x wq 0x%x", dlb, q, WR(q));
	nincstats(0, open);
	return 0;
}

/*
 * dlpiclose - terminate DLPI stream
 */

static int
dlpiclose(q)
	queue_t	*q;
{
	DLB *dlb;
	multi_t *multi;
	int err;

	dlb = (DLB *)q->q_ptr;

	switch (dlb->dlb_state) {
	case DL_DATAXFER:
	case DL_PROV_RESET_PENDING:
	case DL_USER_RESET_PENDING:
	case DL_DISCON11_PENDING:
	case DL_OUTCON_PENDING:
	case DL_INCON_PENDING:
		abort_session(dlb);
		/* fallthrough */

	case DL_IDLE:
		/* cancel pending transmissions blocked on source routes */
		if (dlb->dlb_drd)
			drd_cancel(dlb);

		/* turn off promiscuity */
		if (dlb->dlb_promisc) {
			if (err = promiscoff(dlb, dlb->dlb_promisc))
				ERR(dlb, "dlpiclose: promiscoff err (%d)", err);
		}

		/* remove registered multicasts */
		while (multi = dlb->dlb_multi) {
			int addr = *(int *)(multi->addr+2); /* <ahem> helpful */
			err = delmulti(dlb, multi->addr, dlb->dlb_physlen);
			if (err)
				ERR(dlb, "dlpiclose: delmulti %x err %d",
								addr, err);
		}
		dl_unbind_sap(dlb);
		if (dlb->dlb_pend)
			putmem(dlb->dlb_pend);

		/* toss any unprocessed messages */
		dl_nukem(dlb);
		/* fallthrough */

	case DL_UNBOUND:
		detach(dlb);
		/* fallthrough */

	case DL_UNATTACHED:
		tq_free(dlb->dlb_tq1);
		tq_free(dlb->dlb_tq2);
		tq_free(dlb->dlb_tqi);
		if (dlb->dlb_failmp)
			freeb(dlb->dlb_failmp);
		nddrele(dlb->dlb_prov);
		dlb->dlb_state = -1;	/* assert if somehow reentered */
		break;

	default:
		DB(assert(0););
	}

	nincstats(0, close);
	return mi_close_comm(&dlbh, q);
}

/*
 * dlpirsrv - handle upstream flow control; keep interrupts from perculating up
 */

static int
dlpirsrv(q)
	queue_t	*q;
{
	mblk_t *mp;
	DLB *dlb;

	dlb = (DLB *)q->q_ptr;
	dlpiinput(dlb);		/* process incoming frames */

	while (mp = getq(q)) {
		if (mp->b_datap->db_type > QPCTL)
			putnext(q, mp);
		else if (canput(q->q_next))
			putnext(q, mp);
		else {
			putbq(q, mp);
			break;
		}
	}

	/* release local busy, tell remote */
	if (canput(q->q_next))
		local_okay(dlb);
	return 0;
}

/*
 * dlpiwput - handle downstream messages
 *
 * Unitdata messages are handled immediately, in a bid for lower
 * latency.  Raw messages are also sent immediately, since there is
 * nothing else for this driver to do with them.
 *
 * All other messages are deferred to the service proc, where we are
 * guaranteed to be in a non-interrupt environment.  This is critical
 * for routines like DL_ENABMULTI_REQ which must make an ioctl to the
 * ndd, which asserts if called at other than INTBASE.
 */

static int
dlpiwput(q, mp)
	queue_t	*q;
	mblk_t *mp;
{
	DLB *dlb = (DLB *)q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		/* XXX flush retransmit queue also? */
		TRC(dlb, "dlpiwput: flush %d", *mp->b_rptr);
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (*mp->b_rptr & FLUSHR) {
			flushq(RD(q), FLUSHDATA);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			break;
		}
		freemsg(mp);
		break;
	case M_DATA:
		if (dlb->dlb_pkt_format == NS_INCLUDE_MAC)
			tx_raw(dlb, mp);
		else if (!(dlb->dlb_mode & DL_CODLS))
			eproto(dlb, mp);
		else
			putq(q, mp);
		break;
	case M_PROTO:
	case M_PCPROTO:
		if (mp->b_wptr - mp->b_rptr >= sizeof(ulong)) {
			if (*(ulong *)mp->b_rptr == DL_UNITDATA_REQ) {
				/* binary compatibility only */
				if (dlb->dlb_pkt_format == NS_INCLUDE_MAC) {
					if (mp->b_cont)
						tx_raw(dlb, mp->b_cont);
					freeb(mp);
				} else
					dl_udatareq(dlb, mp);
				break;
			}
		}
		/* fallthrough */
	default:
		putq(q, mp);
		break;
	}

	return 0;
}

/*
 * dlpiwsrv - handle downstream DLPI messages
 */

static int
dlpiwsrv(q)
	queue_t *q;
{
	DLB *dlb;
	mblk_t *mp;
	ulong prim;
	timerq_t *tp;
	drdreq_t *rp;
	int type;

	dlb = (DLB *)q->q_ptr;

	dlb->dlb_flags &= ~FD_QENABLE;
	if (dlb->dlb_flags & FD_REXMIT) {
		dl_rexmit(dlb);
		dlb->dlb_flags &= ~FD_REXMIT;
	}

	while (mp = getq(q)) {
		switch (type = mp->b_datap->db_type) {
		case M_PCSIG:
			/* internal timer fired */
			tq_fire(mp);
			break;
		case M_START:
			/* DRD callback mechanism */
			rp = (drdreq_t *)mp->b_rptr;
			if (rp->tag == TAG_DRD)
				(*rp->func)(dlb, rp->arg, rp->addr,
							rp->seg, rp->seglen);
			freemsg(mp);
			break;
		case M_DATA:
			if (dl_datareq(dlb, mp)) {
				putbq(q, mp);
				dlb->dlb_flags |= FD_QENABLE;
				TRC(dlb, "dlpiwsrv: datareq deferred");
				return;
			}
			break;
		case M_PROTO:
		case M_PCPROTO:
			if (mp->b_wptr - mp->b_rptr < sizeof(ulong))
				return eproto(dlb, mp);
			prim = *(ulong *)mp->b_rptr;
			if (prim > DL_LAST_PRIM)
				return eproto(dlb, mp);
			(*dl_funcs[prim])(dlb, mp);
			break;
		case M_CTL:
		case M_IOCTL:
			dlpictl(dlb, mp);
			break;
		default:
			incstats(dlb, unknown_msgs);
			freemsg(mp);
			TRC(dlb, "dlpiwsrv: dropped msg type %x", type);
		}
	}
}

/*
 * dlpictl - handle M_IOCTL and M_CTL messages and send proper replies
 */

/* binary compatibility only - do not use in new code */
#define	ODL_PKT_FORMAT		0x01
#define	ODL_OUTPUT_RESOLVE	0x02
#define	ODL_INPUT_RESOLVE	0x04

static void
dlpictl(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	struct iocblk *iocp;
	mblk_t *mpc;
	int len;
	int arg, err;
	pfv_t func;

	iocp = (struct iocblk *)mp->b_rptr;
	if (!(mpc = mp->b_cont))
		goto einval;
	len = mpc->b_wptr - mpc->b_rptr;

	switch (iocp->ioc_cmd) {
	case ODL_OUTPUT_RESOLVE:	/* replace the tx address resolver */
	case DL_OUTPUT_RESOLVE:
		/*
		 * binary compatibility:
		 * if the b_rptr points directly to a function,
		 * this is old style behavior
		 */
		if (mpc->b_rptr <= mpc->b_datap->db_base ||
		    mpc->b_rptr >= mpc->b_datap->db_lim) {
			dlb->dlb_output = (pfi_t)mpc->b_rptr;
			iocp->ioc_count = 0;
			break;
		}
		if (len != sizeof(pfv_t *))
			goto einval;
		func = *(pfv_t *)mpc->b_rptr;
		if (!func || mp->b_datap->db_type != M_CTL)
			goto einval;
		dlb->dlb_output = (pfi_t)func;
		iocp->ioc_count = 0;
		break;
	case ODL_INPUT_RESOLVE:		/* replace the rx address resolver */
	case DL_INPUT_RESOLVE:
		/*
		 * binary compatibility:
		 * if the b_rptr points directly to a function,
		 * this is old style behavior
		 */
		if (mpc->b_rptr <= mpc->b_datap->db_base ||
		    mpc->b_rptr >= mpc->b_datap->db_lim) {
			dlb->dlb_input = (pfv_t)mpc->b_rptr;
			iocp->ioc_count = 0;
			break;
		}
		if (len != sizeof(pfv_t *))
			goto einval;
		func = *(pfv_t *)mpc->b_rptr;
		if (func && mp->b_datap->db_type != M_CTL)
			goto einval;
		dlb->dlb_input = func;
		iocp->ioc_count = 0;
		break;
	case ODL_PKT_FORMAT:		/* change the address format */
	case DL_PKT_FORMAT:
		if (BOUND(dlb)) {
			iocp->ioc_error = EBUSY;
			goto iocnak;
		}
		if (len != sizeof(int))
			goto einval;
		arg = *(int *)mpc->b_rptr;
		switch (arg) {
		case NS_PROTO:
		case NS_PROTO_SNAP:
		case NS_INCLUDE_LLC:
		case NS_INCLUDE_MAC:
		case NS_PROTO_DL_COMPAT:
		case NS_PROTO_DL_DONTCARE:
			dlb->dlb_pkt_format = arg;
			break;
		default:
			goto einval;
		}
		iocp->ioc_count = 0;
		break;
	case DL_ROUTE:			/* alter source routes */
		if (dlb->dlb_state != DL_UNBOUND &&
		    dlb->dlb_state != DL_IDLE) {
			iocp->ioc_error = EBUSY;
			goto iocnak;
		}
		if (len == 0) {
			dlb->dlb_drd = 0;	/* no drd, please */
			dlb->dlb_seglen = 0;	/* no route */
			iocp->ioc_count = 0;
			break;
		} else if (len > dlb->dlb_physlen) {
			if (len & 1)
				goto einval;
			dlb->dlb_drd = 0;	/* no further drd needed */
			dlb->dlb_seglen = len - dlb->dlb_physlen;
			bcopy(mpc->b_rptr + dlb->dlb_physlen,
				dlb->dlb_seg, dlb->dlb_seglen);
			iocp->ioc_count = 0;
			break;
		} else {
			drd(dlb, (struct mbuf *)mp, mpc->b_rptr, cb_iocroute);
			return;	/* cb_iocroute will handle msg */
		}
		break;
	case DL_TUNE_LLC:		/* alter LLC tunables */
		if (dlb->dlb_state != DL_UNBOUND) {
			iocp->ioc_error = EBUSY;
			goto iocnak;
		}
		if (len != sizeof(llctune_t))
			goto einval;
		dlpitune(dlb, mpc->b_rptr);
		*(llctune_t *)mpc->b_rptr = dlb->dlb_tune;
		break;
	case DL_TUNE_TBL:		/* replace the LLC tunable table */
		if (!dlb->dlb_priv) {
			iocp->ioc_error = EACCES;
			goto iocnak;
		}
		if (len != (sizeof(llctune_t) * 3)) {
			iocp->ioc_error = EINVAL;
			goto iocnak;
		}
		bcopy(mpc->b_rptr, dl_tune, len);
		iocp->ioc_count = 0;
		break;
	case DL_ZERO_STATS:		/* restart statistics counters */
		if (*(int *)mpc->b_rptr) {
			if (!dlb->dlb_priv) {
				iocp->ioc_error = EACCES;
				goto iocnak;
			}
			bzero(&dl_stats, sizeof(dl_stats));
		} else {
			bzero(&dlb->dlb_stats, sizeof(dlb->dlb_stats));
		}
		iocp->ioc_count = 0;
		break;
	case DL_SET_REMADDR:		/* set remote address before connect */
		/*
		 * This ioctl allows the remote address to be specified
		 * before a connection is established, thus allowing the
		 * exchange of XID (and TEST and UI) frames on DL_CODLS.
		 * See dlpillc() for details.
		 */
		if (!(dlb->dlb_mode & DL_CODLS))
			goto einval;
		if (dlb->dlb_state != DL_IDLE) {
			iocp->ioc_error = EBUSY;
			goto iocnak;
		}
		if (len < PHYSLEN || len > MAXADDR_LEN) {
			iocp->ioc_error = EINVAL;
			goto iocnak;
		}
		if (len > PHYSLEN)
			dlb->dlb_dsap = mpc->b_rptr[PHYSLEN];
		if (setconn(dlb,dlb->dlb_ssap,mpc->b_rptr,dlb->dlb_dsap,0)) {
			iocp->ioc_error = EBUSY;
			goto iocnak;
		}
		iocp->ioc_count = 0;
		break;
	default:
einval:
		iocp->ioc_error = EINVAL;
iocnak:
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		mp->b_datap->db_type = M_IOCNAK;
		putq(dlb->dlb_rq, mp);
		return;
	}

	iocp->ioc_error = 0;
	iocp->ioc_rval = 0;
	mp->b_datap->db_type = M_IOCACK;
	putq(dlb->dlb_rq, mp);
}

/*
 * cb_iocroute - DRD callback for DL_ROUTE queries
 */

static void
cb_iocroute(dlb, mp, daddr, segp, seglen)
	DLB *dlb;
	mblk_t *mp;
	uchar *daddr, *segp;
	int seglen;
{
	struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
	mblk_t *mpc = mp->b_cont;

	/* if no route found, fail the ioctl */
	if (seglen < 0) {
		freeb(mpc);
		mp->b_cont = 0;
		iocp->ioc_error = EHOSTUNREACH;
error:
		iocp->ioc_rval = 0;
		iocp->ioc_count = 0;
		mp->b_datap->db_type = M_IOCNAK;
		putq(dlb->dlb_rq, mp);
		return;
	}

	/* ensure there is enough room for reply */
	if (mpc->b_datap->db_lim - mpc->b_datap->db_base < seglen) {
		freeb(mpc);
		mpc = mp->b_cont = allocb(seglen, BPRI_HI);
		if (!mpc) {
			iocp->ioc_error = ENOSR;
			goto error;
		}
	}

	/* response is source_route */
	mpc->b_rptr = mpc->b_datap->db_base;
	mpc->b_wptr = mpc->b_rptr + seglen;
	bcopy(segp, mpc->b_rptr, seglen);
	iocp->ioc_count = seglen;

	/* ack it */
	iocp->ioc_rval = 0;
	iocp->ioc_error = 0;
	mp->b_datap->db_type = M_IOCACK;
	putq(dlb->dlb_rq, mp);
}

/*
 * dlpitune - alter per-stream tunable parameters
 */

#define	settune(dlb, t, parm) \
	(t->parm == -1) ? dlb->dlb_tune.parm = dl_tune[TDFT].parm : \
		(dlb->dlb_tune.parm = max(t->parm, dl_tune[TMIN].parm), \
		dlb->dlb_tune.parm = min(t->parm, dl_tune[TMAX].parm))

static void
dlpitune(dlb, t)
	DLB *dlb;
	llctune_t *t;
{
	if (t->flags & F_LLC_T1) settune(dlb, t, t1);
	if (t->flags & F_LLC_T2) settune(dlb, t, t2);
	if (t->flags & F_LLC_TI) settune(dlb, t, ti);
	if (t->flags & F_LLC_N1) settune(dlb, t, n1);
	if (t->flags & F_LLC_N2) settune(dlb, t, n2);
	if (t->flags & F_LLC_N3) settune(dlb, t, n3);
	if (t->flags & F_LLC_K)  settune(dlb, t, k);

	/* special cases */
	if ((dlb->dlb_tune.n1 > dlb->dlb_ndd->ndd_mtu) ||
	    ((t->flags & F_LLC_N1) && t->n1 == -1))
		dlb->dlb_tune.n1 = dlb->dlb_ndd->ndd_mtu;
	if (dlb->dlb_tune.n3 > dlb->dlb_tune.k)
		dlb->dlb_tune.n3 = dlb->dlb_tune.k;

	/* requested to establish as new defaults */
	if (dlb->dlb_priv && (t->flags & F_LLC_SET))
		bcopy(&dlb->dlb_tune, &dl_tune[TDFT], sizeof(llctune_t));
}

#undef settune

/*
 * dlpiconfig - configuration entry point
 */

#include <sys/device.h>
#include <sys/uio.h>
#include <sys/strconf.h>

int
dlpiconfig(dev, cmd, uiop)
	dev_t dev;
	int cmd;
	struct uio *uiop;
{
	static char buf[FMNAMESZ+10];
	static kstrconf_t conf = {
		buf, &dlpiinfo, STR_NEW_OPEN | STR_MPSAFE | STR_Q_NOTTOSPEC, 0, SQLVL_QUEUEPAIR,
	};
	char *cp;
	int drd;

	NONI_LOCK();
	if (!dv.tag) {
		dv.tag = TAG_DLPI;
		dv.provh = &provh;	/* list of configured CDLI providers */
		dv.dlbh = &dlbh;	/* list of opened DLPI streams */
		dv.stats = &dl_stats;	/* global statistics */
		dv.info = &dlpiinfo;	/* stream interfaces */
	}
	NONI_UNLOCK();

	if (uiomove(buf, sizeof buf, UIO_WRITE, uiop))
		return EFAULT;
	buf[FMNAMESZ+10-1] = 0;

	/* check for options */
	drd = 0;
	for (cp = buf; *cp; ++cp) {
		if (*cp == ',')
			break;
	}
	/* process options */
	if (*cp) {
		*cp++ = 0;	/* null term the driver name, nuke the comma */
		for (; *cp; ++cp) {
			if (*cp == 'r' || *cp == 'R')
				drd = 1;
		}
	}

	conf.sc_major = major(dev);

	switch (cmd) {
	case CFG_INIT:	return nddinit(&conf, drd);
	case CFG_TERM:	return nddterm(&conf, drd);
	default:	return EINVAL;
	}
}

/*
 * nddinit - associate a CDLI driver with DLPI
 */

#define	EQ(s1,s2)	!strcmp(s1,s2)

static int
nddinit(sc, drd)
	kstrconf_t *sc;
	int drd;
{
	prov_t *p;
	int err;

	if (!(p = getmem(prov_t, 1)))
		return ENOMEM;

	bzero(p, sizeof(prov_t));
	p->p_major = sc->sc_major;

	/* attempt to localize en/et hacks */
	p->p_isether = EQ(sc->sc_name, "en");
	if (EQ(sc->sc_name, "et"))
		bcopy("en", p->p_nddname, sizeof("en"));
	else
		bcopy(sc->sc_name, p->p_nddname, FMNAMESZ+1);

	/* str_install weeds out duplicate attempts */
	if (err = str_install(STR_LOAD_DEV, sc)) {
		putmem(p);
		return err;
	}

	p->p_drd = drd;		/* set DRD request (boolean) */

	NONI_LOCK();
	p->p_next = provh;
	provh = p;
	NONI_UNLOCK();

	return 0;
}

/*
 * nddterm - dissociate a CDLI driver from DLPI
 */

static int
nddterm(sc, drd)
	kstrconf_t *sc;
	int drd;
{
	prov_t *p, **pp;
	ppa_t *ppa;
	int rc = ENXIO;

	NONI_LOCK();
	for (pp = &provh; p = *pp; pp = &(*pp)->p_next) {
		if (p->p_major == sc->sc_major) {
			/* do not terminate if any provider's ppa is busy */
			if (p->p_ref) {
				rc = EBUSY;
				break;
			}

			/* release all resources attached to the ppa's */
			for (ppa = p->p_ppa; ppa; ppa = p->p_ppa) {
				if (p->p_drd)
					drd_unbind(ppa->ndd, ppa->ppa);
				p->p_ppa = ppa->next;
				ns_free(ppa->ndd);
				putmem(ppa);
			}

			/* unlink from streams and provh list */
			if (!(rc = str_install(STR_UNLOAD_DEV, sc))) {
				*pp = p->p_next;
				memset(p, 0xff, sizeof(*p));
				putmem(p);
			}
			break;
		}
	}
	NONI_UNLOCK();
	return rc;
}

/*
 * nddget - gain a reference to a provider
 */

static prov_t *
nddget(maj)
	int maj;
{
	prov_t *p;

	NONI_LOCK();
	for (p = provh; p; p = p->p_next) {
		if (maj == p->p_major) {
			++p->p_ref;	/* prevents unloading before attach */
			break;
		}
	}
	NONI_UNLOCK();
	return p;
}

/*
 * nddrele - release a reference to a provider
 */

static void
nddrele(p)
	prov_t *p;
{
	NONI_LOCK();
	if (--p->p_ref < 0) {
		ERR(0, "p_ref underflow");
		p->p_ref = 0;
	}
	NONI_UNLOCK();
}
