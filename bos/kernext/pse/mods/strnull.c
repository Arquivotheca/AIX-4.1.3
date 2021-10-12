static char sccsid[] = "@(#)44        1.4  src/bos/kernext/pse/mods/strnull.c, sysxpse, bos411, 9428A410j 12/9/93 10:47:55";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   ORIGINS: 27, 83
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

/*
 * strnull - streams null device
 *
 * act as a source or sink.  Differs from /dev/null in that
 * read()s will not get eof, but will read garbage forever.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/errno.h>

#include <sys/dir.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/signal.h>
#include <sys/user.h>

#include <pse/perftest.h>

#define	ACNT(ary)	(sizeof(ary)/sizeof(ary[0]))

static struct module_info minfo = {
	0,		/* module id */
	"strnull",	/* module name */
	0,		/* min pkt size */
	INFPSZ,		/* max pkt size */
	0,		/* hiwater */
	0,		/* lowater */
};

int snopen(), snclose(), snwput();
#ifndef	DOREPLY
int snrsrv();
#endif

static struct qinit rinit = {
#ifdef	DOREPLY
	NULL, NULL, snopen, snclose, NULL, &minfo, NULL
#else
	NULL, snrsrv, snopen, snclose, NULL, &minfo, NULL
#endif
};

static struct qinit winit = {
	snwput, NULL, NULL, NULL, NULL, &minfo, NULL
};

struct streamtab sninfo = { &rinit, &winit, NULL, NULL };

static struct {
	int size;
} tab[10];

int Buf_id;
unsigned int Count;

snopen(q, dev, flag, sflag)
	queue_t *q;
	dev_t dev;
	int flag, sflag;
{
	mblk_t *mp;
	int i;

	if (q->q_ptr) {
		setuerror(EBUSY);
		return OPENFAIL;
	}
	if (sflag == CLONEOPEN) {
		for (i = 0; i < ACNT(tab); i++)
			if (!tab[i].size)
				break;
	} else
		i = minor(dev);
	if (i >= ACNT(tab)) {
		setuerror(ENXIO);
		return OPENFAIL;
	}
	q->q_ptr = (char *)&tab[i].size;
	*(int *)q->q_ptr = DFLT_SIZE;

#ifdef	DOREPLY
	/* turn read notification on */
	if (mp = allocb(sizeof(struct stroptions), BPRI_HI)) {
		mp->b_datap->db_type = M_SETOPTS;
		((struct stroptions *)mp->b_wptr)->so_flags = SO_MREADON;
		mp->b_wptr += sizeof(struct stroptions);
		putnext(q, mp);		/* tell stream head */
	} else {
		*(int *)q->q_ptr = 0
		q->q_ptr = (char *)0;
		setuerror(ENOMEM);
		return OPENFAIL;
	}
#endif

	return i;
}

snclose(q, flag)
	queue_t *q;
	int flag;
{
	/* all enqueued data flushed by sth on close */
	*(int *)q->q_ptr = 0;
	q->q_ptr = (char *)0;	/* available */
#ifndef	SVR3_2
	if (Buf_id)
		unbufcall(Buf_id);
#endif
	return 0;
}

snrsrv(q)
	queue_t *q;
{
	mblk_t *mp;
	int size = *(int *)q->q_ptr;

	while (Count && canput(q->q_next)) {
		if (!(mp = allocb(size, BPRI_HI))) {
			Buf_id = bufcall(size, BPRI_HI, qenable, q);
			return;
		}
		mp->b_wptr += size;
		putnext(q, mp);
		--Count; /* generate only as many messages as needed */
	}
}

snwput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	int okay = 0;
	struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

	switch (mp->b_datap->db_type) {
#ifdef	DOREPLY
	case M_READ:
		/*
		 * someone is waiting for input in read(), getmsg(), etc.
		 * we are supposed to be a source, so supply the info.
		 */
		/*
		 * free the message instead of reusing it, because
		 * we want all upstream messages to be of size size
		 */
		freemsg(mp);
		if (!(mp = allocb(size, BPRI_LO))) {
			/* what now? if ignored, read() will hang */
			/* but cannot reply, no msg avail */
			/* reuse incoming M_READ mblk? */
			break;
		}
		/* respond with a size M_DATA message */
		qreply(q, mp);
		break;
#endif

	case M_FLUSH:
		/* handle flushing */
		if (*mp->b_rptr & FLUSHW)
			flushq(q, 0);
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~ FLUSHW;
			qreply(q, mp);
		} else
			freemsg(mp);
		break;

	case M_IOCTL:
		switch (iocp->ioc_cmd) {
		case PERF_SIZE: {
			int size = *(int *)mp->b_cont->b_rptr;
			if (size < 0 || size > MAX_PKTSZ)
				break;
			*(int *)(RD(q)->q_ptr) = size;
			okay = 1;
			break;
			}

		case PERF_GEN:
			/* retrieve # of msgs to generate */
			Count = *(unsigned int *)mp->b_cont->b_rptr;
			snrsrv(RD(q));
			okay = 1;
			break;

		case PERF_FLOW:
			okay = 1;
			break;

		default:
			break;
		}
		if (okay) {
			mp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
		} else {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EINVAL;
		}
		qreply(q, mp);
		break;

	default:	/* data, proto, etc */
		freemsg(mp);
		/* ignore everything - we're a sink! */
	}
}

#include <sys/device.h>
#include <sys/strconf.h>

/* ARGSUSED */
sn_config(devno, cmd, uiop)
	dev_t devno;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"strnull", &sninfo, STR_OLD_OPEN,
	};

	conf.sc_major = major(devno);

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_DEV, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_DEV, &conf);
	default:	return EINVAL;
	}
}
