#ifndef lint
static char sccsid[] = "@(#)65 1.3 src/bos/kernext/tty/uc_sjis.c, sysxldterm, bos412, 9445C412a 9/26/94 14:18:34";
#endif
/*
 * COMPONENT_NAME: (sysxtty) SJIS upper converter streams module
 *
 * FUNCTIONS: uc_sjis_open, uc_sjis_close, uc_sjis_rput,
 *            uc_sjis_rsrv, uc_sjis_wput, uc_sjis_wsrv,
 *            uc_sjis_ioctl, uc_sjis_config
 *
 * ORIGINS: 40, 71, 83
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
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

#include <sys/sysconfig.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <sys/str_tty.h>
#include <sys/strconf.h>
#include <sys/device.h>
#include "stream_sjis.h"

struct module_info uc_sjis_modinfo = {
        UC_SJIS_MODULE_ID, UC_SJIS_MODULE_NAME, 0, INFPSZ, UC_SJIS_HIWAT,
        UC_SJIS_LOWAT
};

struct qinit uc_sjis_rinit = {
        uc_sjis_rput, uc_sjis_rsrv,
        uc_sjis_open, uc_sjis_close, 0, &uc_sjis_modinfo, 0
};

struct qinit uc_sjis_winit = {
        uc_sjis_wput, uc_sjis_wsrv, 0, 0, 0, &uc_sjis_modinfo, 0
};

struct streamtab uc_sjis_info = { &uc_sjis_rinit, &uc_sjis_winit };

lock_t	uc_sjis_conf_lock = LOCK_AVAIL;	/* sjis module configuration lock */

static	int	uc_sjis_count = 0;	/* count loads uc_sjis module.	*/

int
uc_sjis_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	register struct sjis_s *jp;
	int error;

	if (q->q_ptr)
		return(0);
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
uc_sjis_close(queue_t *q, int flag, cred_t *credp)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	if (jp->rbid)
		unbufcall(jp->rbid);
	if (jp->wbid)
		unbufcall(jp->wbid);

	if (jp->rtid)
		unbufcall(jp->rtid);
	if (jp->wtid)
		unbufcall(jp->wtid);

	if (jp->rspare)
		freemsg(jp->rspare);
	if (jp->wspare)
		freemsg(jp->wspare);

	he_free(jp);
	q->q_ptr = 0;
	return(0);
}

/*
 * read side should convert from ajec to sjis
 * also peek at the ioctl(M_IOCACK) to look for IEXTEN flag
 */
int
uc_sjis_rput(register queue_t *q, register mblk_t *mp)
{
	struct sjis_s *jp = (struct sjis_s *)q->q_ptr;
	struct iocblk *iocp;

	switch (mp->b_datap->db_type) {
	case M_IOCACK:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF: {
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
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			flushq(q, FLUSHDATA);
			jp->asc[0] = jp->asc[1] = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first 
		    || !sjis_readdata(jp, q, mp, conv_ajec2sjis, 1))
			putq(q, mp);
		break;
	default:
		if (canput(q->q_next) || mp->b_datap->db_type >= QPCTL)
			putnext(q, mp);
		else
			putq(q, mp);
	}
	return(0);
}

int
uc_sjis_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_readdata(jp, q, mp, conv_ajec2sjis, 1)) 
			{
				putbq(q, mp);
				return(0);
			}
			break;
		default:
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

/*
 * write side should convert from sjis to ajec and do ioctl
 */
int
uc_sjis_wput(register queue_t *q, register mblk_t *mp)
{
	struct sjis_s *jp;

	jp = (struct sjis_s *) q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (q->q_first 
		    || !sjis_writedata(jp, q, mp, conv_sjis2ajec, 2))
			putq(q, mp);
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHDATA);
			jp->sac = 0;
		}
		putnext(q, mp);
		break;
	case M_IOCTL:
		if (q->q_first || !uc_sjis_ioctl(jp, q, mp))
			putq(q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp);
		else
			putq(q, mp);
	}
	return(0);
}

int
uc_sjis_wsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_writedata(jp, q, mp, conv_sjis2ajec, 2))
			{
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			if (!uc_sjis_ioctl(jp, q, mp)) {
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

uc_sjis_ioctl(struct sjis_s *jp, queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;

	/* First check for flow control on ioctl's which we may want to
	 * send downstream (which means everything but SJIS_UC*).
	 */

	switch (iocp->ioc_cmd) {
	case SJIS_UC_C1SET:
	case SJIS_UC_C1GET:
		break;
	default:
		if (!canput(q->q_next))
			return(0);
		else
			break;
	}

	switch (iocp->ioc_cmd) {
	case TIOCSTI:
		if (sjis_proc_sti(jp, *mp->b_cont->b_rptr, q) < 0) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
		} else {
			iocp->ioc_error = 0;
			mp->b_datap->db_type = M_IOCACK;
		}
		iocp->ioc_count = 0;
		qreply(q, mp);
		return(1);
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
	case SJIS_UC_C1SET:
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
	case SJIS_UC_C1GET:
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
 * process the TIOCSTI ioctl
 * there is SJIS -> AJEC conversion in here too
 */
int
sjis_proc_sti(
	register struct sjis_s *jp,
	unsigned char uc,
	register queue_t *q
	)
{
	unsigned char uc1, n1, n2;
	unsigned char tc[2];
	int	return_value;

	uc1 = jp->sti_c;
	if (!uc1) {
		if (INRANGE(0x00, 0x7f, uc)) {
			/*
			 * ascii and C0
			 */
			tc[0] = uc;
			return_value = sjis_send_sti(q, tc, 1);
			return(return_value);
		}
		if (INRANGE(0xa1, 0xdf, uc)) {
			/*
			 * hankaku-kana
			 */
			tc[0] = SS2;
			tc[1] = uc;
			return_value = sjis_send_sti(q, tc, 2);
			return(return_value);
		}
		if (INRANGE(0x81, 0x9f, uc) || INRANGE(0xe0, 0xef, uc)) {
			/*
			 * first byte of kanji
			 */
			jp->sti_c = uc;
			return(0);
		}
		tc[0] = uc & 0x7f;
		return_value = sjis_send_sti(q, tc, 1);
		return(return_value);
	}
	if (INRANGE(0x81, 0x9f, uc1)) {
		/*
		 * kanji
		 */
		if (INRANGE(0x40, 0x7e, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa1;
			n2 = uc + 0x61;
		}
		else if (INRANGE(0x80, 0x9e, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa1;
			n2 = uc + 0x60;
		}
		else if (INRANGE(0x9f, 0xfc, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa2;
			n2 = uc + 2;
		}
		else {
			n1 = uc1 & 0x7f;
			n2 = uc & 0x7f;
		}
		jp->sti_c = 0;
		tc[0] = n1;
		tc[1] = n2;
		return_value = sjis_send_sti(q, tc, 2);
		return(return_value);
	}
	if (INRANGE(0xe0, 0xef, uc1)) {
		/*
		 * kanji
		 */
		if (INRANGE(0x40, 0x7e, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xdf;
			n2 = uc + 0x61;
		}
		else if (INRANGE(0x80, 0x9e, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xdf;
			n2 = uc + 0x60;
		}
		else if (INRANGE(0x9f, 0xfc, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xe0;
			n2 = uc + 2;
		}
		else {
			n1 = uc1 & 0x7f;
			n2 = uc & 0x7f;
		}
		jp->sti_c = 0;
		tc[0] = n1;
		tc[1] = n2;
		return_value = sjis_send_sti(q, tc, 2);
		return(return_value);
	}
	/*
	 * error
	 */
	jp->sti_c = 0;
	tc[0] = n1 & 0x7f;
	tc[1] = n2 & 0x7f;
	return_value = sjis_send_sti(q, tc, 2);
	return(return_value);
}

/*
 * send the TIOCSTI M_CTL message down with the given string
 */
int
sjis_send_sti(
	register queue_t *q,
	register unsigned char *ucp,
	register int len
	)
{
	register mblk_t *mp, *mp1;
	register struct iocblk *iocp;

	mp = allocb(sizeof(struct iocblk), BPRI_MED);
	mp1 = allocb(len, BPRI_MED);
	if (!mp || !mp1) {
		if (mp)
			freemsg(mp);
		if (mp1)
			freemsg(mp1);
		return(-1);
	}
	mp->b_datap->db_type = M_CTL;
	mp->b_cont = mp1;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_cmd = TIOCSTI;
	iocp->ioc_count = len;
	while (len--)
		*mp1->b_wptr++ = *ucp++;
	putnext(q, mp);
	return(0);
}


/*
 * uc_sjis_config() - sjis upper code converter entry point.
 */
int
uc_sjis_config(cmd, uiop)
	int	cmd;
	struct	uio	*uiop;
{
	static	strconf_t	uc_conf = {
		"uc_sjis", &uc_sjis_info, (STR_NEW_OPEN|STR_MPSAFE),
	};
	int	error = 0, locked;
	struct	uc_sjis_dds	init_uc_sjis_dds;

	locked = lockl(&uc_sjis_conf_lock, LOCK_SHORT);
	uc_conf.sc_sqlevel = SQLVL_QUEUEPAIR;

	switch (cmd) {
	case CFG_INIT:
		if (uiop) {
			if (uiomove(&init_uc_sjis_dds, 
				    sizeof(struct uc_sjis_dds),
				    UIO_WRITE, uiop) ||
			    (init_uc_sjis_dds.which_dds != UC_SJIS_DDS))
				break;
			else {
				if (uc_sjis_count == 0)
					error = str_install(STR_LOAD_MOD,
								&uc_conf);
				if (!error)
					uc_sjis_count++;
			}
		}
		else
			error = str_install(STR_LOAD_MOD, &uc_conf);
		break;
	case CFG_TERM:
		if (uiop) {
			if (uiomove(&init_uc_sjis_dds,
                                    sizeof(struct uc_sjis_dds),
                                    UIO_WRITE, uiop) ||
                            (init_uc_sjis_dds.which_dds != UC_SJIS_DDS))
                                break;
                        else {
				if (uc_sjis_count == 1)
					error = str_install(STR_UNLOAD_MOD,
								&uc_conf);
				if (!error)
                                	uc_sjis_count--;
			}
		}
		else
			error = str_install(STR_UNLOAD_MOD, &uc_conf);
		break;
	default:
		error = EINVAL;
		break;
	}
	if (locked != LOCK_NEST)
		unlockl(&uc_sjis_conf_lock);
	return(error);
}
