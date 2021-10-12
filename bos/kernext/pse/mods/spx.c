static char sccsid[] = "@(#)31        1.6  src/bos/kernext/pse/mods/spx.c, sysxpse, bos412, 9447C 11/22/94 11:42:22";
/*
 * COMPONENT_NAME: Streams framework
 * 
 * FUNCTIONS:
 * 
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

/*
 * spx - STREAMS Pipe Multiplexor
 *
 * Open this puppy in pairs, then I_FDINSERT one into the other.  Writes
 * to one side will then end up on the other.  Closing either side will
 * send a M_HANGUP to the other side.
 *
 * The raison d'etere for spx is to support Stevens' code in
 * _UNIX Network Programming_.  It is expected to be used only
 * for passing file descriptors around via STREAMS, as true STREAMS
 * pipe support might possibly appear in a later release.
 *
 * history:
 *	910822 dgb	my best guess as to SVR32 RFS's spx functionality
 *	920220 dgb	fixed spoof hole; fixed push/pop bug
 *
 * XXX M_PROTO ambiguity:
 * There is no way to guarantee that a particular M_[PC]PROTO message
 * is an I_FDINSERT generated message, so SPX asserts that the first
 * M_[PC]PROTO message is an I_FDINSERT request, and passes all others.
 */

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/lockname.h>
#include <sys/lock_alloc.h>

static int spxopen(), spxclose(), spxrput(), spxwput();
static int spxrsrv(), spxwsrv();

static struct module_info minfo = { 0, "spx", 0, INFPSZ, 0, 0 };
static struct qinit rinit = { spxrput,spxrsrv,spxopen,spxclose,0,&minfo,0 };
static struct qinit winit = { spxwput,spxwsrv,0,      0,       0,&minfo,0 };
struct streamtab spxinfo = { &rinit, &winit, 0, 0 };

#define MAXSPX	 128*2		/* usually opened in pairs */
static queue_t *spx_spx[MAXSPX];
static Simple_lock spx_lock;

/* ARGSUSED */
static int
spxopen(q, devp, flag, sflag, credp)
	queue_t *q;
	dev_t *devp;
	int flag;
	int sflag;
	cred_t *credp;
{
	int index, _ssavpri;
	

	if (sflag == CLONEOPEN) {
		_ssavpri = disable_lock(INT_MAX, &spx_lock);
		for (index = MAXSPX-1; index >= 0; index--) {
			if (!spx_spx[index])
				break;
		}
		unlock_enable(_ssavpri, &spx_lock);
		if (index < 0) {
			return EBUSY;
		}
		*devp = makedev(major(*devp), index);
	} else {
		index = minor(*devp);
		if (index < 0 || index >= MAXSPX) {
			return ENXIO;
		}
	}

	noenable(WR(q));	/* XXX disallow back-enables */
	_ssavpri = disable_lock(INT_MAX, &spx_lock);
	spx_spx[index] = q;
	unlock_enable(_ssavpri, &spx_lock);
	q->q_ptr = (char *)&spx_spx[index];
	
	return 0;
}

static int
spxrput(q, mp)
	queue_t *q;
	mblk_t *mp;
{

	
	/* only possible flush is flushr */
	if (mp->b_datap->db_type == M_FLUSH)
		flushq(q, FLUSHALL);

	if (mp->b_datap->db_type >= QPCTL || canput(q->q_next))
		putnext(q, mp);
	else
		putq(q, mp);

}

static int
spxrsrv(q)
	queue_t *q;
{
	mblk_t *mp;


	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			return 0;
		}
		putnext(q, mp);
	}

}

static int
spxerr(q, mp, err)
	queue_t *q;
	mblk_t *mp;
	int err;
{


	mp->b_datap->db_type = M_ERROR;
	*mp->b_rptr = err;
	mp->b_wptr = mp->b_rptr + 1;
	qreply(q, mp);
	return 0;
}

static int
spxwput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	queue_t *xrq, **spx;
	int err = 0;
	int _ssavpri;


	switch (mp->b_datap->db_type) {
	case M_PROTO:
	case M_PCPROTO:
		/* after we're connected, ignore all further M_PROTOs */
		if (q->q_next)
			break;

		/*
		 * We are not connected: assume this is fdinsert.
		 *
		 * There is no direct way to determine if the message
		 * is due to an I_FDINSERT ioctl.  Therefore, the
		 * following tests insure a valid pointer ('d' is
		 * the clincher).
		 *	a. proto message is sizeof queue_t * bytes long
		 *	b. the supposed read queue pointer is valid
		 *	c. there is no data portion to this message
		 *	d. lookup xrq in spx_spx[] table (valid if found)
		 */

		if ((mp->b_wptr - mp->b_rptr != sizeof(queue_t *)) ||
		    !(xrq = *(queue_t **)mp->b_rptr) || xrq == RD(q) ||
		    mp->b_cont) {
			err = spxerr(q, mp, EINVAL);
			return err;
		}
	
		_ssavpri = disable_lock(INT_MAX, &spx_lock);	
		for (spx = &spx_spx[MAXSPX-1]; spx >= spx_spx; spx--)
			if (xrq == *spx)
				break;
		unlock_enable(_ssavpri, &spx_lock);
		if (spx < spx_spx) {
			err = spxerr(q, mp, ENXIO);
			return err;
		}

		/* if other side already connected, fail */
		if (WR(xrq)->q_next) {
			err = spxerr(q, mp, EBUSY);
			return err;
		}

		weldq_cnx (q, xrq);
		weldq_cnx (WR(xrq), RD(q));

		enableok(q);
		enableok(WR(xrq));

		freemsg(mp);
		return 0;

	case M_FLUSH:
		/* if connected, flush opposite side of stream */
		if (q->q_next) {
			if (*mp->b_rptr == FLUSHR)
				*mp->b_rptr = FLUSHW;
			else if (*mp->b_rptr == FLUSHW)
				*mp->b_rptr = FLUSHR;
			/* allow FLUSHRW to propogate normally */
			putnext(q, mp);
		} else {
			/* if not connected, then no traffic to flush */
			freemsg(mp);
		}
		return 0;

	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		((struct iocblk *)mp->b_rptr)->ioc_error = EINVAL;
		qreply(q, mp);
		return 0;

	}

	/* must be cross connected first! */
	if (!q->q_next) {
		err = spxerr(q, mp, EPIPE);
		return err;
	}

	if (canput(q->q_next))
		putnext(q, mp);
	else
		putq(q, mp);

	return 0;
}

static int
spxwsrv(q)
	queue_t *q;
{
	mblk_t *mp;

	
	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			return;
		}
		putnext(q, mp);
	}

}

static int
spxclose(q)
	queue_t *q;
{
	queue_t *xrq;


	/*
	 * disconnect if not already disconnected
	 * first side to close disconnects and sends a hangup to the other
	 */
	if (xrq = WR(q)->q_next) {
		unweldq_cnx (WR(q), xrq);
		unweldq_cnx (WR(xrq), q);
		putctl(xrq, M_HANGUP);
	}
	*(char **)q->q_ptr = (char *)0;
	return 0;

}

#include  <sys/device.h>
#include  <sys/strconf.h>

/* ARGSUSED */
int
spx_config(dev, cmd, uiop)
	dev_t dev;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"spx", &spxinfo, STR_NEW_OPEN|STR_MPSAFE,
		0, SQLVL_QUEUEPAIR, 0
	};

	conf.sc_major = major(dev);

	switch (cmd) {
	case CFG_INIT:	
		lock_alloc((&spx_lock), LOCK_ALLOC_PIN,PSE_STH_EXT_LOCK, -1);
		simple_lock_init(&spx_lock);
		return str_install(STR_LOAD_DEV, &conf);
	case CFG_TERM:
		lock_free(&spx_lock);	
		return str_install(STR_UNLOAD_DEV, &conf);
	default:	
		return EINVAL;
	}
}
