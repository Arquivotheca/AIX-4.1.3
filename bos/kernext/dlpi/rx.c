static char sccsid[] = "@(#)28  1.2  src/bos/kernext/dlpi/rx.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:37:32";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: rx_disc
 *		rx_dm
 *		rx_frmr
 *		rx_sabme
 *		rx_ua
 *		rx_ui
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
 * rx.c - handle (most) incoming LLC frames
 *
 * public routines:
 *	rx_sabme - incoming connections and resets
 *	rx_ua - acknowledgements to connects, disconnects, and resets
 *	rx_dm - negative acknowledgements to connects, disconnects, resets
 *	rx_disc - incoming disconnects
 *	rx_frmr - incoming frame rejects
 *	rx_ui - incoming datagrams
 */

#include "include.h"

/*
 * rx_sabme - incoming connections and resets
 *
 * creates a new connection record in the pending list with no dlb
 */

void
rx_sabme(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc = mtod(m, llc_t *);
	int final = ISPF(llc);

	switch (dlb->dlb_state) {
	case DL_IDLE:
		/* willing to accept connections */
		if (dlb->dlb_conind > 0)
			dl_connind(dlb, llc, mp);
		break;

	case DL_INCON_PENDING:
		/* new or retried connection attempt */
		dl_connind(dlb, llc, mp);
		break;

	case DL_PROV_RESET_PENDING:
		/* retried reset attempt; just remember poll bit */
		if (!dlb->dlb_localreset) {
			dlb->dlb_final = final;
			freeb(mp);
			break;
		}
		/* else: simultaneous reset/reset attempt */
		/* fallthrough */

	case DL_OUTCON_PENDING:
	case DL_USER_RESET_PENDING:
		/* simultaneous connection/reset attempt */
		dlb->dlb_retry = dlb->dlb_n2;
		dlb->dlb_s_flag = 1;
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, UA | (final ? PF1 : 0));
		freeb(mp);
		break;

	case DL_DATAXFER:
		/* a reset, not a new connection */
		if (!dlb->dlb_s_flag) {
			/* possibly a retransmitted SABME, not a reset */
			dlb->dlb_s_flag = 1;
			freeb(mp);
			break;
		}
		dlb->dlb_s_flag = 0;
		dlb->dlb_final = final;
		stop_timers(dlb);
		dl_resetind(dlb, mp, DL_USER, DL_RESET_RESYNCH);
		break;

	case DL_DISCON11_PENDING:
		/* we'll take this as an ack */
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, DM | (final ? PF1 : 0));
		term_session(dlb);
		dl_flush(dlb, FLUSHRW);
		putq(dlb->dlb_rq, okack(mp, DL_DISCONNECT_REQ));
		break;

	default:
		/* "impossible" */
		DB(assert(0););
		freeb(mp);
		break;
	}

	m_freem(m);	/* NB: must be last, since use llc in conind */
}

/*
 * rx_ua - acknowledgements to connects, disconnects, and resets
 */

void
rx_ua(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc = mtod(m, llc_t *);
	int final = ISPF(llc);

	m_freem(m);

	/* discard as bogus if final doesn't match */
	/* XXX supposed to generate a FRMR */
	if (dlb->dlb_poll != final) {
		TRC(dlb, "rx_ua: p%d f%d, dropped UA", !final, final);
		return;
	}

	switch (dlb->dlb_state) {
	case DL_OUTCON_PENDING:
		init_session(dlb);
		dl_conncon(dlb, mp);
		break;

	case DL_USER_RESET_PENDING:
		init_session(dlb);
		dl_resetcon(dlb, mp);
		break;

	case DL_PROV_RESET_PENDING:
		if (dlb->dlb_localreset) {
			init_session(dlb);
			dl_okack(dlb, DL_RESET_RES);
		}
		break;

	case DL_DISCON11_PENDING:
		term_session(dlb);
		dl_flush(dlb, FLUSHRW);
		putq(dlb->dlb_rq, okack(mp, DL_DISCONNECT_REQ));
		break;

	default:
		TRC(dlb, "rx_ua: state %d dropped UA", dlb->dlb_state);
		break;
	}
}

/*
 * rx_dm - negative acknowledgements to connects, disconnects, resets
 */

void
rx_dm(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	m_freem(m);

	switch (dlb->dlb_state) {
	case DL_INCON_PENDING:
		dl_discind(dlb, mp, 0, DL_USER, DL_DISC_UNSPECIFIED);
		break;

	case DL_OUTCON_PENDING:
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_PROVIDER,
			DL_CONREJ_DEST_UNREACH_TRANSIENT);
		break;

	case DL_DISCON11_PENDING:
		term_session(dlb);
		dl_flush(dlb, FLUSHRW);
		putq(dlb->dlb_rq, okack(mp, DL_DISCONNECT_REQ));
		break;

	case DL_DATAXFER:
	case DL_USER_RESET_PENDING:
	case DL_PROV_RESET_PENDING:	/* not in FSM, but possible */
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_USER, DL_DISC_ABNORMAL_CONDITION);
		break;

	default:
		TRC(dlb, "rx_dm: state %d dropped DM", dlb->dlb_state);
		break;
	}
}

/*
 * rx_disc - incoming disconnects
 */

void
rx_disc(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc = mtod(m, llc_t *);
	int final = ISPF(llc);
	int pf = final ? PF1 : 0;

	m_freem(m);

	switch (dlb->dlb_state) {
	case DL_DISCON11_PENDING:
		/* simultaneous disconnect */
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, UA | pf);
		/* XXX not same mechanism as sabme? */
		break;

	case DL_DATAXFER:
		/* aborted connection */
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, UA | pf);
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_USER, DL_DISC_NORMAL_CONDITION);
		break;

	case DL_OUTCON_PENDING:
		/* improper response to rejected connection */
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, DM | pf);
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_PROVIDER,
			DL_CONREJ_DEST_UNREACH_TRANSIENT);
		break;

	case DL_USER_RESET_PENDING:
	case DL_PROV_RESET_PENDING:
		/* rejected reset */
		dlb->dlb_final = final;
		tx_rsp(dlb->dlb_conn, DM | pf);
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_USER, DL_DISC_TRANSIENT_CONDITION);
		break;

	default:
		TRC(dlb, "rx_disc: state %d dropped DISC", dlb->dlb_state);
		break;
	}
}

/*
 * rx_frmr - incoming frame rejects
 */

void
rx_frmr(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	m_freem(m);
	reset_session(dlb);
	nincstats(dlb, rx_frmr);
}

/*
 * rx_ui - incoming datagrams
 */

void
rx_ui(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	dl_unitdata_ind_t *p;
	int dlen, slen, off;
	int fmt = dlb->dlb_pkt_format;

	if (!canput(dlb->dlb_rq)) {
		TRC(dlb, "rx_udata: flow control drop");
discard:
		m_freem(m);
		freeb(mp);
		incstats(dlb, rx_discards);
		return;
	}

	/* recover address lengths */
	dlen = mp->b_rptr[0];
	slen = mp->b_rptr[1];

	/* format header */
	p = (dl_unitdata_ind_t *)mp->b_rptr;
	mp->b_wptr = mp->b_rptr + DL_SADDR_OFFSET + slen;
	p->dl_primitive = DL_UNITDATA_IND;
	p->dl_group_address = m->m_hdr.mh_flags & (M_MCAST | M_BCAST);
	p->dl_dest_addr_offset = DL_DADDR_OFFSET;
	p->dl_dest_addr_length = dlen;
	p->dl_src_addr_offset = DL_SADDR_OFFSET;
	p->dl_src_addr_length = slen;

	/* construct data portion from mbuf chain */
	off = 0;
	if (fmt != NS_INCLUDE_LLC) {
		llc_t *llc = mtod(m, llc_t *);
		off = LLC_ULEN;
		if (llc->dsap == SAP_SNAP)
			off += SNAP_LEN;
	}
		
	if (!(mp->b_cont = mbuf_to_mblk(m, off)))
		goto discard;

	addstats(dlb, rx_bytes, m->m_pkthdr.len);

	/* send datagram upstream */
	putq(dlb->dlb_rq, mp);
}
