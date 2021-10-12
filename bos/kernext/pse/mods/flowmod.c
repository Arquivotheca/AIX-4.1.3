static char sccsid[] = "@(#)38        1.2  src/bos/kernext/pse/mods/flowmod.c, sysxpse, bos411, 9428A410j 8/27/93 09:40:34";
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
 * flowmod - streams pass-thru module with flow control
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

static struct module_info minfo = {
	0,		/* module id */
	"flowmod",	/* module name */
	0,		/* min pkt size */
	INFPSZ,		/* max pkt size */
	HIWATER,	/* hiwater */
	LOWATER,	/* lowater */
};

static int flowopen(), flowclose(), flowrput(), flowwput(), flowsrv();

static struct qinit rinit = {
	flowrput, flowsrv, flowopen, flowclose, NULL, &minfo, NULL
};

static struct qinit winit = {
	flowwput, flowsrv, NULL, NULL, NULL, &minfo, NULL
};

struct streamtab flowinfo = { &rinit, &winit, NULL, NULL };

flowopen(q, dev, flag, sflag)
	queue_t *q;
	dev_t dev;
	int flag;
	int sflag;
{
	return 0;
}

flowclose(q, flag)
	queue_t *q;
	int flag;
{
	return 0;
}

flowwput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
	int size;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		putq(q, mp);
		return;
	
	case M_IOCTL:
		if (iocp->ioc_cmd == PERF_SIZE) {
			size = *(int *)mp->b_cont->b_rptr;
			q->q_qinfo->qi_minfo->mi_hiwat = size * HIMSGS;
			q->q_qinfo->qi_minfo->mi_lowat = size * LOMSGS;
			RD(q)->q_qinfo->qi_minfo->mi_hiwat = size * HIMSGS;
			RD(q)->q_qinfo->qi_minfo->mi_lowat = size * LOMSGS;
		}
		break;

	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, 0);
		break;
	
	default:
		break;
	}
	putnext(q, mp);
}

flowrput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	switch (mp->b_datap->db_type) {
	case M_DATA:
		putq(q, mp);
		return;

	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR)
			flushq(q, 0);
		break;

	default:
		break;
	}
	putnext(q, mp);
}

flowsrv(q)
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

#include <sys/device.h>
#include <sys/strconf.h>

/* ARGSUSED */
flow_config(cmd, uiop)
	int cmd;
	struct uiop *uiop;
{
	static strconf_t conf = {
		"flowmod", &flowinfo, STR_OLD_OPEN,
	};

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_MOD, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_MOD, &conf);
	default:	return EINVAL;
	}
}
