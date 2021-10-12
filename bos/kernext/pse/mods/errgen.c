static char sccsid[] = "@(#)67        1.2  src/bos/kernext/pse/mods/errgen.c, sysxpse, bos411, 9428A410j 8/27/93 09:40:12";
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
 * errgen - generates error messages for strlog
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>

#include <pse/errgen.h>

static struct module_info minfo = {
	0,		/* module id */
	"errgen",	/* module name */
	0,		/* min pkt size */
	INFPSZ,		/* max pkt size */
	0,		/* hiwater */
	0,		/* lowater */
};

int erropen(), errclose(), errput();

static struct qinit rinit = {
	NULL, NULL, erropen, errclose, NULL, &minfo, NULL
};

static struct qinit winit = {
	errput, NULL, NULL, NULL, NULL, &minfo, NULL
};

struct streamtab errinfo = { &rinit, &winit, NULL, NULL };

erropen(q, devp, flag, sflag, credp)
	queue_t *q;
	dev_t *devp;
	int flag, sflag;
	cred_t *credp;
{
	mblk_t *mp;

	if (q->q_ptr)
		return EBUSY;
	q->q_ptr = (char *)1;
	*devp = makedev(major(*devp), 0);
	return 0;
}

errclose(q, flag)
	queue_t *q;
	int flag;
{
	q->q_ptr = (char *)0;
	return 0;
}

errput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	struct iocblk *iocp;
	struct errblk *errp;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case 1:
			if (iocp->ioc_count != sizeof(struct errblk)) {
				iocp->ioc_error = EINVAL;
				mp->b_datap->db_type = M_IOCNAK;
				break;
			}
			errp = (struct errblk *)mp->b_cont->b_rptr;
			errp->mess[sizeof(errp->mess)-1] = 0;
			
			strlog(errp->mid, errp->sid, errp->level, errp->flags,
			       errp->mess, errp->arg1, errp->arg2, errp->arg3);
			iocp->ioc_error = 0;
			mp->b_datap->db_type = M_IOCACK;
			break;

		default:
			iocp->ioc_error = EINVAL;
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		qreply(q, mp);
		break;

	default:	/* data, proto, etc */
		freemsg(mp);
		/* ignore everything */
	}
}

#include <sys/device.h>
#include <sys/strconf.h>

/* ARGSUSED */
err_config(devno, cmd, uiop)
	dev_t devno;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"errgen", &errinfo, STR_NEW_OPEN,
	};

	conf.sc_major = major(devno);

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_DEV, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_DEV, &conf);
	default:	return EINVAL;
	}
}
