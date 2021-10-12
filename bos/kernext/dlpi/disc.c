static char sccsid[] = "@(#)14  1.2  src/bos/kernext/dlpi/disc.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:37:17";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_discind
 *		dl_discreq
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
 * disc.c - disconnects
 */

#include "include.h"

/*
 * dl_discreq - disconnect the stream
 */

void
dl_discreq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_disconnect_req_t *dr = (dl_disconnect_req_t *)mp->b_rptr;
	conn_t *c;
	int err;

	switch (dlb->dlb_state) {
	case DL_INCON_PENDING:
		if (!(c = findpend(dlb, dr->dl_correlation))) {
			err = DL_BADCORR;
			goto error;
		}
		tx_rsp(c, DM | ((c->poll) ? PF1 : 0));
		c->dlb = 0;
		if (--dlb->dlb_npend == 0)
			dlb->dlb_state = DL_IDLE;
		putq(dlb->dlb_rq, okack(mp, DL_DISCONNECT_REQ));
		return;

	/* XXX not in LLC FSM, but possible */
	case DL_OUTCON_PENDING:
	case DL_USER_RESET_PENDING:
		/* fallthrough */

	case DL_DATAXFER:
	case DL_PROV_RESET_PENDING:
		if (dr->dl_correlation) {
			err = DL_BADCORR;
			goto error;
		}
		break;

	default:
		err = DL_OUTSTATE;
error:
		putq(dlb->dlb_rq, errack(mp, DL_DISCONNECT_REQ, err));
		return;
	}

	/* committed to disconnect */
	dlb->dlb_state = DL_DISCON11_PENDING;

	stop_timers(dlb);
	dlb->dlb_retry = dlb->dlb_n2;
	dlb->dlb_poll = 1;
	tx_disc(&dlb);
	nincstats(dlb, tx_disc);
}

/*
 * dl_discind - send a disconnect indication
 */

void
dl_discind(dlb, mp, corr, who, why)
	DLB *dlb;
	mblk_t *mp;
	ulong corr, who, why;
{
	dl_disconnect_ind_t *di;
	conn_t *c, **cpp;
	char *saddr;

	if (dlb->dlb_state == DL_INCON_PENDING) {
		/* find correlation value and nuke it */
		/*
		 * It is possible for a conn_t to be reused before the
		 * user reads the disconnect, but this is not a problem
		 * since message ordering will prevent its detection.
		 */
		saddr = mp->b_rptr + DL_SADDR_OFFSET;
		for (c = dlb->dlb_pend; c < &dlb->dlb_pend[MAXCONIND]; ++c) {
			if (c->dlb &&
			    !memcmp(c->remaddr, saddr, dlb->dlb_physlen))
				break;
		}
		if (c >= &dlb->dlb_pend[MAXCONIND]) {
			freeb(mp);
			return;
		}
		c->dlb = 0;
		if (--dlb->dlb_npend == 0)
			dlb->dlb_state = DL_IDLE;
		corr = (ulong)c;
	}

	di = (dl_disconnect_ind_t *)mp->b_rptr;
	di->dl_primitive = DL_DISCONNECT_IND;
	di->dl_originator = who;
	di->dl_reason = why;
	di->dl_correlation = corr;

	mp->b_wptr += sizeof(dl_disconnect_ind_t);

	if (dlb->dlb_state != DL_INCON_PENDING) {
		dl_flush(dlb, FLUSHRW);
		dlb->dlb_state = DL_IDLE;
	}
	putq(dlb->dlb_rq, mp);
	nincstats(dlb, rx_disc);
}
