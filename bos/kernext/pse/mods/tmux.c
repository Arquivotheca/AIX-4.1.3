static char sccsid[] = "@(#)48        1.5  src/bos/kernext/pse/mods/tmux.c, sysxpse, bos411, 9428A410j 8/27/93 09:43:38";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   ORIGINS: 27 63 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#define	service_proc

/* tmux.c 2.2
 */

/* This simple mux was typed in from the example mux in the
 * Streams Programmer's Guide.
 */

#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/strlog.h>

typedef	struct iocblk	* IOCP;
typedef struct linkblk	* LINKP;

static	int	muxopen(), muxclose(), muxuwput(), muxlwsrv(), muxlrput();
#ifdef	service_proc
static int muxlrsrv();
#endif

static struct module_info info = {
	0, "tmux", 0, INFPSZ, 512, 128
};

static struct qinit urinit = {	/* upper read */
	NULL, NULL, muxopen, muxclose, NULL, &info, NULL
};

static struct qinit uwinit = {	/* upper write */
	muxuwput, NULL, NULL, NULL, NULL, &info, NULL
};

static struct qinit lrinit = {	/* lower read */
#ifdef	service_proc
	muxlrput, muxlrsrv, NULL, NULL, NULL, &info, NULL
#else
	muxlrput, NULL, NULL, NULL, NULL, &info, NULL
#endif
};

static struct qinit lwinit = {	/* lower write */
	NULL, muxlwsrv, NULL, NULL, NULL, &info, NULL
};

struct streamtab tmuxinfo = { &urinit, &uwinit, &lrinit, &lwinit };

struct mux {
	queue_t	* qptr;		/* back pointer to read queue */
};

struct mux	mux_mux[10];
int	mux_cnt = 10;

queue_t	* muxbot;	/* linked lower queue */
int	muxerr;		/* set if error or hangup on lower stream */

static	queue_t	* get_next_q(   void   );

static int
muxopen (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	minor_dev;
	struct mux	* mux;

	if (sflag == CLONEOPEN) {
		for (minor_dev = 0; minor_dev < mux_cnt; minor_dev++) {
			if (mux_mux[minor_dev].qptr == NULL)
				break;
		}
	} else
		minor_dev = geteminor(*devp);
	if (minor_dev >= mux_cnt)
		return ENOENT;
	mux = &mux_mux[minor_dev];
	mux->qptr = q;
	q->q_ptr = (char *)mux;
	WR(q)->q_ptr = (char *)mux;
	*devp = makedevice(getemajor(*devp), minor_dev);
	return 0;
}

static int
muxuwput (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	struct mux	* mux;
	IOCP	iocp;
	LINKP	linkp;

	mux = (struct mux *)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		/* Ioctl. Only channel 0 can do ioctls.  Two
		 * calls are recognized: LINK and UNLINK */
		if (mux != mux_mux)
			goto iocnak;
		iocp = (IOCP)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case I_LINK:
		case I_PLINK:
			/* Link. The data contains a linkblk structure.
			 * Remember the bottom queue in muxbot */
			if (muxbot != NULL)
				goto iocnak;
			linkp = (LINKP)mp->b_cont->b_rptr;
			muxbot = linkp->l_qbot;
			muxerr = 0;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(q, mp);
			break;
		case I_UNLINK:
		case I_PUNLINK:
			/* Unlink.  The data contains a linkblk structure.
			 * Should not fail an unlink.  Null out muxbot. */
			linkp = (LINKP)mp->b_cont->b_rptr;
			muxbot = NULL;
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(q, mp);
			break;
		default:
iocnak:
			/* fail ioctl */
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
		}
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (*mp->b_rptr & FLUSHR) {
			flushq(RD(q), FLUSHDATA);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		} else
			freemsg(mp);
		break;
	case M_DATA:
		/* Data.  If we have no bottom queue --> fail.
		 * Otherwisse, queue the data, and invoke the lower
		 * service procedure. */
		if (muxerr  ||  muxbot == NULL)
			goto bad;
		putq(q, mp);
		qenable(muxbot);
		break;
	default:
bad:
		/* Send an error message upstream. */
		mp->b_datap->db_type = M_ERROR;
		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		*mp->b_wptr++ = EINVAL;
		qreply(q, mp);
	}
}

static int
muxlwsrv (q)
	queue_t	* q;
{
register mblk_t	* mp, * bp;
register queue_t	* nq;

	/* While lower stream is not blocked, find an upper queue to
	 * service (get_next_q) and send one message from it downstream. */
	while (canput(q->q_next)) {
		nq = get_next_q();
		if (nq == NULL)
			break;
		mp = getq(nq);
		/* Prepend the outgoing message with a single byte header
		 * that indicates the minor device number it came from. */
		if ((bp = allocb(1, BPRI_MED)) == NULL) {
			freemsg(mp);
			continue;
		}
		*bp->b_wptr++ = (struct mux *)nq->q_ptr - mux_mux;
		bp->b_cont = mp;
		putnext(q, bp);
	}
}

/* Round robin scheduling.
 * Return next upper queue that needs servicing.
 * Returns NULL when no more work needs to be done.
 */
static queue_t *
get_next_q ()
{
static	int	next;
	int	i, start;
register queue_t	* q;

	start = next;
	for (i = next; i < mux_cnt; i++) {
		if (q = mux_mux[i].qptr) {
			q = WR(q);
			if (q->q_first) {
				next = i + 1;
				return q;
			}
		}
	}
	for (i = 0; i < start; i++) {
		if (q = mux_mux[i].qptr) {
			q = WR(q);
			if (q->q_first) {
				next = i + 1;
				return q;
			}
		}
	}
	return NULL;
}

static int
muxlrput (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	queue_t	* uq;
	mblk_t	* b_cont;
	int	dev;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		/* Flush queues.  NOTE: sense of tests is reversed
		 * since we are acting like a "stream head". */
		if (*mp->b_rptr & FLUSHR)
			flushq(q, 0);
		if (*mp->b_rptr & FLUSHW) {
			*mp->b_rptr &= ~FLUSHR;
			qreply(q, mp);
		} else
			freemsg(mp);
		break;
	case M_ERROR:
	case M_HANGUP:
		muxerr = 1;
		freemsg(mp);
		break;
	case M_DATA:
		/* Route message.  First byte indicates
		 * device to send to.  No flow control.
		 * Extract and delete the device number.  If the leading block
		 * is now empty adn more blocks follow, strip the leading
		 * block.  The stream head interprets a leading zero length
		 * block as an EOF regardless of what follows (sigh). */
#ifndef service_proc
		dev = *mp->b_rptr++;
		if (mp->b_rptr == mp->b_wptr  &&  (b_cont = mp->b_cont)) {
			freeb(mp);
			mp = b_cont;
		}
		/* Sanity check.  Device must be in range */
		if (dev < 0  ||  dev >= mux_cnt) {
			freemsg(mp);
			break;
		}
		/* If the upper stream is open and not backed up,
		 * send the message there, otherwise discard it. */
		uq = mux_mux[dev].qptr;
		if (uq != NULL  &&  canput(uq->q_next))
			putnext(uq, mp);
		else
			freemsg(mp);
#else
		putq(q, mp);
#endif
		break;
	default:
		freemsg(mp);
	}
}

#ifdef	service_proc
static int
muxlrsrv(q)
	queue_t *q;
{
	queue_t	*uq;
	mblk_t *mp, *b_cont;
	int dev;

	while (mp = getq(q)) {
		dev = *mp->b_rptr++;
		if (mp->b_rptr == mp->b_wptr && (b_cont = mp->b_cont)) {
			freeb(mp);
			mp = b_cont;
		}
		if (dev < 0 || dev >= mux_cnt) {
			freemsg(mp);
			continue;
		}
		uq = mux_mux[dev].qptr;
		if (uq && canput(uq->q_next))
			putnext(uq, mp);
		else {
			putbq(q, mp);
			return;
		}
	}
}
#endif

/* Upper queue close */
static int
muxclose (q)
	queue_t	* q;
{
	((struct mux *)q->q_ptr)->qptr = NULL;
	return 0;
}

#include <sys/device.h>
#include <sys/strconf.h>

int
tmux_config(dev, cmd, uiop)
	dev_t dev;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"tmux", &tmuxinfo, STR_NEW_OPEN,
	};

	conf.sc_major = major(dev);

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_DEV, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_DEV, &conf);
	default:	return EINVAL;
	}
}
