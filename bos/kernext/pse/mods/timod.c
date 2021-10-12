static char sccsid[] = "@(#)45        1.5  src/bos/kernext/pse/mods/timod.c, sysxpse, bos411, 9428A410j 11/10/93 16:21:26";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS:	timod_open, timod_close, timod_rput, timod_wput, timod_config
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

/** Copyright (c) 1990  Mentat Inc.
 ** timod.c 2.3
 **/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <pse/common.h>

#define	MAXPSZ_TO_FORCE_PUTMSG_ERRS	1
#define	MINPSZ_TO_FORCE_PUTMSG_ERRS	32767

typedef	struct iocblk	* IOCP;

typedef struct timod_s {
	uint	timod_mode;	/* TLI or XTI */
	uint	timod_qlen;	/* Saved qlen parameter for XTI */
	MBLKP	timod_mp;	/* Save mp on IOCTL requests */
} TIMOD, * TIMODP;

/* Definitions for timod_mode */
#define	TIMOD_TLI	0	/* TLI stream */
#define	TIMOD_XTI	1	/* XTI stream */

staticf	int	timod_close(   queue_t * q   );
staticf	int	timod_open(   queue_t * q, dev_t * devp, int flag, int sflag, cred_t * credp   );
staticf	int	timod_rput(   queue_t * q, MBLKP mp   );
staticf	int	timod_wput(   queue_t * q, MBLKP mp   );

static struct module_info minfo =  {
#define	MODULE_ID	5006
	MODULE_ID, "mi_timod", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	timod_rput, nil(pfi_t), timod_open, timod_close, nil(pfi_t), &minfo
};

static struct qinit winit = {
	timod_wput, nil(pfi_t), nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab timodinfo = { &rinit, &winit };

static	IDP	timod_g_head;

staticf int
timod_close (q)
	queue_t	* q;
{
	TIMODP	timod;

	timod = (TIMODP)q->q_ptr;
	if (timod->timod_mp)
		freemsg(timod->timod_mp);
	return mi_close_comm(&timod_g_head, q);
}


staticf int
timod_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	return mi_open_comm(&timod_g_head, sizeof(TIMOD), q, devp, flag, sflag, credp);
}

staticf int
timod_rput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	IOCP	iocp;
	MBLKP	mp1;
	struct T_error_ack	* terr;
	TIMODP	timod;

	timod = (TIMODP)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
		mp1 = timod->timod_mp;
		switch (((union T_primitives *)mp->b_rptr)->type) {
		case T_ERROR_ACK:
			if (!mp1) {
				putnext(q, mp);
				return 0;
			}
			terr = (struct T_error_ack *)mp->b_rptr;
			iocp = (IOCP)mp1->b_rptr;
			iocp->ioc_rval = (terr->UNIX_error << 8) | (terr->TLI_error & 0xff);
			iocp->ioc_count = 0;
			freemsg(mp);
			break;
		case T_BIND_ACK:
			timod->timod_qlen = ((struct T_bind_ack *)mp->b_rptr)->CONIND_number;
			fallthru;
		case T_OK_ACK:
		case T_OPTMGMT_ACK:
		case T_INFO_ACK:
		case T_ADDR_ACK:
			if (!mp1) {
				putnext(q, mp);
				return 0;
			}
			iocp = (IOCP)mp1->b_rptr;
			mp1->b_cont = mp;
			do {
				mp->b_datap->db_type = M_DATA;
			} while (mp = mp->b_cont);
			iocp->ioc_count = msgdsize(mp1->b_cont);
			break;
		case T_DISCON_IND:
		case T_ORDREL_IND:
		case T_UDERROR_IND:
			WR(q)->q_minpsz = MINPSZ_TO_FORCE_PUTMSG_ERRS;
			WR(q)->q_maxpsz = MAXPSZ_TO_FORCE_PUTMSG_ERRS;
			fallthru;
		default:
			goto done;
		}
		mp = mp1;
		mp->b_datap->db_type = M_IOCACK;
		timod->timod_mp = nil(MBLKP);
		break;
	default:
		break;
	}
done:
	putnext(q, mp);
	return 0;
}

staticf int
timod_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	IOCP	iocp;
	long	* lp;
	MBLKP	mp1;
	TIMODP	timod;

	timod = (TIMODP)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
		switch (((union T_primitives *)mp->b_rptr)->type) {
		case T_FEEDBACK_REQ:
			lp = (long *)mp->b_rptr;
			q = RD(q)->q_next;
			while (q->q_next) {
				q = q->q_next;
			}
			mp->b_rptr += (sizeof(long) * 2);
			if (lp[1] != 0  &&  !(mp1 = getq(q))) {
				freemsg(mp);
				return 0;
			}
			switch (lp[1]) {
			case 0:
				break;
			case MOREDATA:
				linkb(mp, mp1);
				break;
			case MORECTL|MOREDATA:
				linkb(mp, mp1->b_cont);
				fallthru;
			case MORECTL:
				mp1->b_cont = mp->b_cont;
				mp->b_cont = mp1;
				break;
			default:
				freemsg(mp);
				return 0;
			}
			insq(q, q->q_first, mp);
			return 0;
		default:
			break;
		}
		break;
	case M_IOCTL:
		mp1 = mp->b_cont;
		switch (((IOCP)mp->b_rptr)->ioc_cmd) {
		case TI_GETINFO:
			mp1->b_datap->db_type = M_PCPROTO;
			break;
		case TI_BIND:
		case TI_UNBIND:
		case TI_OPTMGMT:
		case TI_ADDR:
			mp1->b_datap->db_type = M_PROTO;
			break;
		case TI_XTI_HELLO:
			timod->timod_mode = TIMOD_XTI;
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			return 0;
		case TI_XTI_GET_STATE:
			iocp = (IOCP)mp->b_rptr;
			mp->b_datap->db_type = M_IOCACK;
			if (iocp->ioc_count >= sizeof(XTIS)) {
				XTISP	xtis = (XTISP)mp1->b_rptr;

				xtis->xtis_qlen = timod->timod_qlen;
				mp1->b_wptr = mp1->b_rptr + sizeof(XTIS);
				iocp->ioc_count = sizeof(XTIS);
			} else
				iocp->ioc_error = EINVAL;
			qreply(q, mp);
			return 0;
		case TI_XTI_CLEAR_EVENT:
			q->q_maxpsz = minfo.mi_maxpsz;
			q->q_minpsz = minfo.mi_minpsz;
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			return 0;
		default:
			putnext(q, mp);
			return 0;
		}
		mp->b_cont = nil(MBLKP);
		timod->timod_mp = mp;
		mp = mp1;
		break;
	default:
		break;
	}
	putnext(q, mp);
	return 0;
}

#include <sys/device.h>
#include <sys/strconf.h>

int
timod_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"timod", &timodinfo, STR_NEW_OPEN,
	};

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_MOD, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_MOD, &conf);
	default:	return EINVAL;
	}
}
