#ifndef lint
static char sccsid[] = "@(#)64 1.3 src/bos/kernext/tty/lc_sjis.c, sysxldterm, bos412, 9445C412a 9/26/94 14:09:24";
#endif
/*
 * COMPONENT_NAME: (sysxtty) SJIS lower converter streams module
 *
 * FUNCTIONS: lc_sjis_open, lc_sjis_close, lc_sjis_rput, lc_sjis_rsrv
 *            lc_sjis_wput, lc_sjis_wsrv, lc_sjis_ioctl, lc_sjis_config
 *
 * ORIGINS: 40, 71, 83
 *
 */
/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1991 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/sleep.h>

#include <sys/ioctl.h>
#include <sys/sysconfig.h>
#include <sys/strconf.h>
#include <sys/device.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <sys/str_tty.h>
#include "stream_sjis.h"

struct module_info lc_sjis_modinfo = {
        LC_SJIS_MODULE_ID, LC_SJIS_MODULE_NAME, 0, INFPSZ, LC_SJIS_HIWAT,
        LC_SJIS_LOWAT
};

struct qinit lc_sjis_rinit = {
        lc_sjis_rput, lc_sjis_rsrv,
        lc_sjis_open, lc_sjis_close, 0, &lc_sjis_modinfo, 0
};

struct qinit lc_sjis_winit = {
        lc_sjis_wput, lc_sjis_wsrv, 0, 0, 0, &lc_sjis_modinfo, 0
};

struct streamtab lc_sjis_info = { &lc_sjis_rinit, &lc_sjis_winit };

lock_t	lc_sjis_conf_lock = LOCK_AVAIL; /* lc_sjis configuration lock	*/

static	int	lc_sjis_count = 0;	/* config method loads count	*/

int
lc_sjis_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	register struct sjis_s *jp;
	int error;

	if (q->q_ptr) {
		return(0);
	}
	jp = (struct sjis_s *)he_alloc(sizeof(struct sjis_s), BPRI_MED);
	if (!jp) 
		return(ENOMEM);
	bzero(jp, sizeof(struct sjis_s));

	/*
	 * Default values for contents of struct sjis_s (NULL fields
	 * already taken care of via bzero() in streams_open_comm()).
	 */
	jp->flags_save = jp->flags = KS_ICONV | KS_OCONV | KS_IEXTEN;
	jp->c1state = SJIS_C1_C0;

	q->q_ptr = (char *)jp;
	WR(q)->q_ptr = (char *)jp;
	return(0);
}

int
lc_sjis_close(queue_t *q, int flag, cred_t *credp)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	if (jp->rbid)
		unbufcall(jp->rbid);
	if (jp->wbid)
		unbufcall(jp->wbid);

	if (jp->rtid)
		pse_untimeout(jp->rtid);
	if (jp->wtid)
		pse_untimeout(jp->wtid);

	if (jp->rspare)
		freemsg(jp->rspare);
	if (jp->wspare)
		freemsg(jp->wspare);

	he_free(jp);
	q->q_ptr = 0;
	return(0);
}

/*
 * read side should convert from sjis to ajec
 * also peek at the ioctl(M_IOCACK) to look for IEXTEN flag
 */
int
lc_sjis_rput(
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct sjis_s *jp = (struct sjis_s *)q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_IOCACK:
	{
		register struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

		switch (iocp->ioc_cmd) {
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
		{
			register struct termios *tp;

			if (!mp->b_cont)
				break;
			tp = (struct termios *)mp->b_cont->b_rptr;
			if (tp->c_lflag & IEXTEN)
				jp->flags |= KS_IEXTEN;
			else
				jp->flags &= ~KS_IEXTEN;
			break;
		}
		default:
			break;
		}
		putnext(q, mp);
		break;
	}
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			flushq(q, FLUSHDATA);
			jp->sac = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first 
		    || !sjis_readdata(jp, q, mp, conv_sjis2ajec, 2))
			putq(q, mp);
		break;
	default: 
		if (mp->b_datap->db_type >= QPCTL)
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	return(0);
}

int
lc_sjis_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_readdata(jp, q, mp, conv_sjis2ajec, 2)) 
			{
				putbq(q, mp);
				return(0);
			}
			break;
		default:
			ASSERT(mp->b_datap->db_type < QPCTL);
			if (!canput(q->q_next)) {
				putbq(q, mp);
				return(0);
			}
			putnext(q, mp);
			break;
		}
	}
	return(0);
}

/*
 * write side should convert from ajec to sjis and do ioctl
 */
int
lc_sjis_wput(
	register queue_t *q,
	register mblk_t *mp
	)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHDATA);
			jp->asc[0] = jp->asc[1] = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first 
		    || !sjis_writedata(jp, q, mp, conv_ajec2sjis, 1))
			putq(q, mp);
		break;
	case M_IOCTL:
		if (q->q_first || !lc_sjis_ioctl(jp, q, mp))
			putq(q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	return(0);
}

int
lc_sjis_wsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_writedata(jp, q, mp, conv_ajec2sjis, 1))
			{
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			if (!lc_sjis_ioctl(jp, q, mp)) {
				putbq(q, mp);
				return(0);
			}
			break;
		default:
			ASSERT(mp->b_datap->db_type < QPCTL);
			if (!canput(q->q_next)) {
				putbq(q, mp);
				return(0);
			}
			putnext(q, mp);
			break;
		} /* switch */
	} /* while */
	return(0);
}

int
lc_sjis_ioctl(struct sjis_s *jp, queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;


	iocp = (struct iocblk *)mp->b_rptr;

	/* First check for flow control on ioctl's which we may want to
	 * send downstream (which means everything but SJIS_LC*).
	 */

	switch (iocp->ioc_cmd) {
	case SJIS_LC_C1SET:
	case SJIS_LC_C1GET:
		break;
	default:
		if (!canput(q->q_next))
			return(0);
		else
			break;
	}

	switch (iocp->ioc_cmd) {
	case EUC_IXLON:
		jp->flags |= KS_ICONV;
		break;
	case EUC_IXLOFF:
		jp->flags &= ~KS_ICONV;
		break;
	case EUC_OXLON:
		jp->flags |= KS_OCONV;
		break;
	case EUC_OXLOFF:
		jp->flags &= ~KS_OCONV;
		break;
	case EUC_MSAVE:
		jp->flags_save = jp->flags;
		jp->flags &= ~(KS_ICONV|KS_OCONV);
		break;
	case EUC_MREST:
		jp->flags = jp->flags_save;
		break;
	case SJIS_LC_C1SET:
		if (!mp->b_cont) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_count < sizeof(int)) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		jp->c1state = *(int *)mp->b_cont->b_rptr;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	case SJIS_LC_C1GET:
		if (!mp->b_cont) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_count < sizeof(int)) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		*(int *)mp->b_cont->b_rptr = jp->c1state;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	default:
		break;
	}

	putnext(q, mp);
	return(1);
}


/*
 * lc_sjis_config() - sjis lower code converter entry point.
 */
int
lc_sjis_config(cmd, uiop)
	int	cmd;
	struct	uio	*uiop;
{
	static	strconf_t	lc_conf = {
		"lc_sjis", &lc_sjis_info, (STR_NEW_OPEN|STR_MPSAFE),
	};
	int	error = 0, locked;
	struct	lc_sjis_dds	init_lc_sjis_dds;

	locked = lockl(&lc_sjis_conf_lock, LOCK_SHORT);
	lc_conf.sc_sqlevel = SQLVL_QUEUEPAIR;
	switch (cmd) {
	case CFG_INIT:
		if (uiop) {
			if (uiomove(&init_lc_sjis_dds, 
				    sizeof(struct lc_sjis_dds),
				    UIO_WRITE, uiop) ||
			    (init_lc_sjis_dds.which_dds != LC_SJIS_DDS))
				break;
			else {
				if (lc_sjis_count == 0)
					error=
					str_install(STR_LOAD_MOD,&lc_conf);
				if (!error)
					lc_sjis_count++;
			}
		}
		else
			error = str_install(STR_LOAD_MOD, &lc_conf);
		break;
	case CFG_TERM:
		if (uiop) {
			if (uiomove(&init_lc_sjis_dds,
				    sizeof(struct lc_sjis_dds),
				    UIO_WRITE, uiop) ||
			    (init_lc_sjis_dds.which_dds != LC_SJIS_DDS))
				break;
			else {
				if (lc_sjis_count == 1)
					error = str_install(STR_UNLOAD_MOD,
								&lc_conf);
				if (!error)
					lc_sjis_count--;
			}
		}
		else
			error = str_install(STR_UNLOAD_MOD, &lc_conf);
		break;
	default:
		error = EINVAL;
		break;
	}
	if (locked != LOCK_NEST)
		unlockl(&lc_sjis_conf_lock);
	return(error);
}
