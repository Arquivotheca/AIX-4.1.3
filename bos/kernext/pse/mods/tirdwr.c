/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: ntirdwr_open, tirdwr_close, tirdwr_open, tirdwr_rput,
 *		tirdwr_wput, tirdwr_config
 *
 *   ORIGINS: 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** tirdwr.c 2.1, last change 11/14/90
 **/

/* static	char	sccsid[] = "@(#)tirdwr.c\t\t2.1"; */

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h>
#include <pse/mi.h>

#ifndef staticf
#define staticf static
#endif
typedef struct msgb     * MBLKP;

staticf	int	ntirdwr_open(   queue_t * q, dev_t dev, int flag, int sflag   );
staticf	int	tirdwr_close(   queue_t * q   );
staticf	int	tirdwr_open(   queue_t * q, dev_t dev, int flag, int sflag   );
staticf	int	tirdwr_rput(   queue_t * q, MBLKP mp   );
staticf	int	tirdwr_wput(   queue_t * q, MBLKP mp   );

static struct module_info nminfo =  {
	0, "ntirdwr", 0, INFPSZ, 2048, 128
};

static struct qinit nrinit = {
	tirdwr_rput, nil(pfi_t), ntirdwr_open, tirdwr_close, nil(pfi_t), &nminfo
};

static struct qinit nwinit = {
	tirdwr_wput, nil(pfi_t), nil(pfi_t), nil(pfi_t), nil(pfi_t), &nminfo
};

struct streamtab ntirdwrinfo = { &nrinit, &nwinit };

static struct module_info minfo =  {
	0, "tirdwr", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	tirdwr_rput, nil(pfi_t), tirdwr_open, tirdwr_close, nil(pfi_t), &minfo
};

static struct qinit winit = {
	tirdwr_wput, nil(pfi_t), nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab tirdwrinfo = { &rinit, &winit };

staticf int
ntirdwr_open (q, dev, flag, sflag)
	queue_t	* q;
	dev_t	dev;
	int	flag;
	int	sflag;
{
reg	MBLKP	mp;
	MBLKP	mp1, mp2;
	union T_primitives	* tprim;

	if (q->q_ptr)
		return 0;
	/* Munge through the stream head queue looking for mblks that
	** 'tirdwr_rput' would not have allowed through in the first place.
	** If any one of these 'untouchables' is present return OPENFAIL.
	*/
	q->q_ptr = (IDP)(T_DISCON_REQ ^ T_ORDREL_REQ);
	q = q->q_next;
	for (mp = q->q_first; mp; mp = mp1) {
		mp1 = mp->b_next;
		switch (mp->b_datap->db_type) {
		case M_PROTO:
			tprim = (union T_primitives *)mp->b_rptr;
			if ((mp->b_wptr - mp->b_rptr) < sizeof(tprim->type))
				break;
			if (tprim->type != T_DATA_IND
			&&  tprim->type != T_EXDATA_IND)
				break;
			mp2 = mp->b_cont;
			rmvq(q, mp);
			freeb(mp);
			if (!(mp = mp2))
				continue;
			insq(q, mp1, mp);
			fallthru;
		case M_DATA:
			if (msgdsize(mp) == 0) {
				rmvq(q, mp);
				freemsg(mp);
			}
			continue;
		case M_SIG:
		case M_PCSIG:
			continue;
		default:
			break;
		}
		return EPROTO;
	}
	return 0;
}

/* NOTE: the read queue 'q_ptr' field designates what type of
** request to issue to the underlying transport provider when we close.
*/

staticf int
tirdwr_close (q)
reg	queue_t	* q;
{
	MBLKP	mp;

	switch ((uint)q->q_ptr) {
	case T_ORDREL_REQ:
		mp = mi_tpi_ordrel_req();
		break;
	case T_DISCON_REQ:
		mp = mi_tpi_discon_req(nil(MBLKP), -1);
		break;
	default:
		return 0;
	}
	if (mp)
		putnext(WR(q), mp);
	return 0;
}

staticf int
tirdwr_open (q, dev, flag, sflag)
	queue_t	* q;
	dev_t	dev;
	int	flag;
	int	sflag;
{
	if (q->q_ptr)
		return 0;
	dev = ntirdwr_open(q, dev, flag, sflag);
	/* Set for default close action */
	q->q_ptr = (IDP)T_DISCON_REQ;
	return dev;
}

staticf int
tirdwr_rput (q, mp)
	queue_t	* q;
reg	MBLKP	mp;
{
reg	MBLKP	mp1;
reg	union T_primitives	* tprim;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		/* Blow off zero length pkts so as not to create false EOF */
		if (mp->b_wptr == mp->b_rptr
		&&  msgdsize(mp) == 0) {
			freemsg(mp);
			return 0;
		}
		break;
	case M_PCPROTO:
		freemsg(mp);
		if ((uint)q->q_ptr == T_ORDREL_REQ)
			q->q_ptr = (IDP)T_DISCON_REQ;
		return putctl1(q->q_next, M_ERROR, EPROTO);
	case M_PROTO:
		tprim = (union T_primitives *)mp->b_rptr;
		if ((mp->b_wptr - mp->b_rptr) < sizeof(tprim->type)) {
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		}
		switch (tprim->type) {
		case T_EXDATA_IND:
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		case T_UNITDATA_IND:
		case T_DATA_IND:	/* Blow off the control portion */
			if (!(mp1 = mp->b_cont) ||  msgdsize(mp1) == 0) {
				freemsg(mp);
				return 0;
			}
			freeb(mp);
			mp = mp1;
			break;
		case T_DISCON_IND:
			q->q_ptr = (IDP)(T_DISCON_REQ ^ T_ORDREL_REQ);
			freemsg(mp);
			return putctl(q->q_next, M_HANGUP);
		case T_ORDREL_IND:
			q->q_ptr = (IDP)T_ORDREL_REQ;
			freemsg(mp);
			/* TODO: bufcall, enable service routine ... */
			if (!(mp = allocb(0, BPRI_HI)))
				return 0;
			break;
		default:
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		}
		break;
	default:
		break;
	}
	putnext(q, mp);
	return 0;
}

staticf int
tirdwr_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (mp->b_wptr == mp->b_rptr
		&&  msgdsize(mp) == 0) {
			freemsg(mp);
			return 0;
		}
		break;
	case M_PCPROTO:
	case M_PROTO:
		freemsg(mp);
		return putctl1(RD(q)->q_next, M_ERROR, EPROTO);
	default:
		break;
	}
	putnext(q, mp);
	return 0;
}

#include <sys/device.h>
#include <sys/strconf.h>

int
tirdwr_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	/* XXX what about ntirdwr? */
	static strconf_t conf = {
		"tirdwr", &tirdwrinfo, STR_NEW_OPEN,
	};

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_MOD, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_MOD, &conf);
	default:	return EINVAL;
	}
}
