static char sccsid[] = "@(#)27  1.2  src/bos/kernext/dlpi/reset.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:38:14";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_resetcon
 *		dl_resetind
 *		dl_resetreq
 *		dl_resetres
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
 * reset.c - DLPI connection reset and response
 *
 * public routines:
 *	dl_resetreq - request a reset on an existing connection
 *	dl_resetcon - confirm a requested reset
 *	dl_resetind - indicate an incoming reset request
 *	dl_resetres - ack an incoming reset request
 */

#include "include.h"

/*
 **************************************************
 * Downstream reset messages
 **************************************************
 */

/*
 * dl_resetreq - request a reset on an existing connection
 */

void
dl_resetreq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	/* verify request */
	if (dlb->dlb_state != DL_DATAXFER) {
		putq(dlb->dlb_rq, errack(mp, DL_RESET_REQ, DL_OUTSTATE));
		return;
	}

	dlb->dlb_state = DL_USER_RESET_PENDING;
	freemsg(mp);

	/*
	 * DLPI is vague about when the flush should occur.
	 * Sending it here prevents a race where the DLS User
	 * can read data after the reset was requested, but
	 * before the remote side responds.
	 */
	dl_flush(dlb, FLUSHRW);	/* see DLPI B.5 */

	/* go send reset */
	stop_timers(dlb);
	dlb->dlb_retry = dlb->dlb_n2;
	dlb->dlb_s_flag = 0;
	dlb->dlb_poll = 1;
	tx_reset(&dlb);
	nincstats(dlb, tx_reset);
}

/*
 * dl_resetcon - send a DL_RESET_CON
 */

void
dl_resetcon(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_reset_con_t *con;

	/* create DLPI message */
	con = (dl_reset_con_t *)mp->b_rptr;
	con->dl_primitive = DL_RESET_CON;
	mp->b_wptr += sizeof(dl_reset_con_t);

	/* send confirmation to user */
	putq(dlb->dlb_rq, mp);
}

/*
 **************************************************
 * Upstream reset messages
 **************************************************
 */

/*
 * dl_resetind - indicate an incoming reset request
 */

void
dl_resetind(dlb, mp, who, why)
	DLB *dlb;
	mblk_t *mp;
	ulong who, why;
{
	dl_reset_ind_t *ri;

	/*
	 * prepare DLPI indication
	 */

	ri = (dl_reset_ind_t *)mp->b_rptr;
	ri->dl_primitive = DL_RESET_IND;
	ri->dl_originator = who;
	ri->dl_reason = why;

	mp->b_wptr += sizeof(dl_reset_ind_t);

	dlb->dlb_state = DL_PROV_RESET_PENDING;

	dl_flush(dlb, FLUSHRW);	/* see DLPI B.5 */

	putq(dlb->dlb_rq, mp);
	nincstats(dlb, rx_reset);
}

/*
 * dl_resetres - ack an incoming reset request
 */

void
dl_resetres(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	int pf = dlb->dlb_final ? PF1 : 0;

	if (dlb->dlb_state != DL_PROV_RESET_PENDING) {
		putq(dlb->dlb_rq, errack(mp, DL_CONNECT_RES, DL_OUTSTATE));
		return;
	}

	/*
	 * If we initiated the reset (no sabme received), then
	 * the reset response needs to restart the connection
	 * process.  The DRD is flushed because the reset may
	 * have been caused by a stale source route.
	 */
	if (dlb->dlb_localreset) {
		void cb_connreq();
		drd_flush(dlb, dlb->dlb_remaddr);
		dlb->dlb_retry = dlb->dlb_n2;
		dlb->dlb_s_flag = 0;
		dlb->dlb_poll = 0;
		if (dlb->dlb_drd)
			drd(dlb, 0, dlb->dlb_remaddr, cb_connreq);
		else
			tx_sabme(&dlb);
		nincstats(dlb, tx_reset);
		freemsg(mp);	/* ack'd by rx_ua() */
		return;
	}

	dlb->dlb_state = DL_DATAXFER;
	putq(dlb->dlb_rq, okack(mp, DL_RESET_RES));

	init_session(dlb);
	tx_rsp(dlb->dlb_conn, UA | pf);
}
