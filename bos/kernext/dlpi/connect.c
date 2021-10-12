static char sccsid[] = "@(#)11  1.2  src/bos/kernext/dlpi/connect.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:37:50";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: cb_connreq
 *		dl_conncon
 *		dl_connind
 *		dl_connreq
 *		dl_connres
 *		dl_token
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
 * connect.c - DLPI connection primitives
 *
 * public routines:
 *	dl_connreq - request a connection to a remote host
 *	dl_conncon - confirm a connection to a remote host
 *	dl_token - provide a token for use in dl_connres
 *	dl_connind - send a connection indication
 *	dl_connres - accept a connection from a remote host
 */

#include "include.h"

void cb_connreq();

/*
 * dl_connreq - establish a connection to a remote host
 */

void
dl_connreq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_connect_req_t *req = (dl_connect_req_t *)mp->b_rptr;
	uchar *daddr;
	int dlen;
	int err = 0;

	extern void tx_sabme();

	/* verify request */
	if (dlb->dlb_state != DL_IDLE)
		err = DL_OUTSTATE;
	else if (!(dlb->dlb_mode & DL_CODLS))
		err = DL_OUTSTATE;
	else if (dlb->dlb_conind > 0)
		err = DL_OUTSTATE;
	else if (req->dl_qos_length)
		err = DL_UNSUPPORTED;
	else {
		dlen = req->dl_dest_addr_length;
		daddr = mp->b_rptr + req->dl_dest_addr_offset;
		if (daddr + dlen > mp->b_wptr)
			err = DL_BADADDR;
		if (dlen != dlb->dlb_physlen + 1)
			err = DL_BADADDR;
	}

	if (err) {
discard:
		putq(dlb->dlb_rq, errack(mp, DL_CONNECT_REQ, err));
		return;
	}

	/* add to conn table */
	/* see search note in misc.c */
	if (err = setconn(dlb, dlb->dlb_ssap, daddr, daddr[dlen-1], 0))
		goto discard;

	dlb->dlb_dsap = daddr[dlen-1];
	bcopy(daddr, dlb->dlb_remaddr, dlen);	/* save address for later */
	dlb->dlb_remaddrlen = dlen;

	/* committed to connection attempt */
	dlb->dlb_state = DL_OUTCON_PENDING;
	freemsg(mp);

	/* go send SABME */
	dlb->dlb_retry = dlb->dlb_n2;
	dlb->dlb_s_flag = 0;
	dlb->dlb_poll = 0;

	if (dlb->dlb_drd)
		drd(dlb, 0, dlb->dlb_remaddr, cb_connreq);
	else
		tx_sabme(&dlb);
	nincstats(dlb, tx_conn);
}

/*
 * cb_connreq - DRD callback for connection attempts
 *
 * This routine will be called by the DRD when it determines the
 * proper source route, or fails to find it.
 */

void
cb_connreq(dlb, m, daddr, segp, seglen)
	DLB *dlb;
	struct mbuf *m;
	uchar *daddr, *segp;
	int seglen;
{
	mblk_t *mp;

	DB(assert(!m););

	if (seglen < 0) {
		dlb->dlb_seglen = 0;
		if (!(mp = dl_gethdr(dlb, 0))) {
			putq(dlb->dlb_rq, dlb->dlb_failmp);
			dlb->dlb_failmp = 0;
		} else
			dl_discind(dlb, mp, 0, DL_PROVIDER,
				DL_CONREJ_DEST_UNREACH_PERMANENT);
		return;
	}

	tx_sabme(&dlb);
}

/*
 * dl_connconn - send a DL_CONNECT_CON
 */

void
dl_conncon(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_connect_con_t *con;
	int off = sizeof(dl_connect_con_t);

	/* create DLPI message */
	con = (dl_connect_con_t *)mp->b_rptr;
	con->dl_primitive = DL_CONNECT_CON;
	con->dl_resp_addr_offset = off;
	con->dl_resp_addr_length = dlb->dlb_remaddrlen;
	bcopy(dlb->dlb_remaddr, mp->b_rptr + off, dlb->dlb_remaddrlen);
	con->dl_qos_length = 0;
	con->dl_qos_offset = 0;
	con->dl_growth = 0;

	mp->b_wptr += off + dlb->dlb_remaddrlen;

	/* send confirmation to user */
	putq(dlb->dlb_rq, mp);
}

/*
 * dl_token - provide a token for use in dl_connres
 */

void
dl_token(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_token_ack_t *ack = (dl_token_ack_t *)mp->b_rptr;

	/* NB: smallest mblk will hold the response */
	mp->b_datap->db_type = M_PCPROTO;	/* just to be sure */
	mp->b_wptr = mp->b_rptr + sizeof(dl_token_ack_t);
	ack->dl_primitive = DL_TOKEN_ACK;
	ack->dl_token = (ulong)dlb;
	putq(dlb->dlb_rq, mp);
}

/*
 * dl_connind - send a connection indication
 */

void
dl_connind(dlb, llc, mp)
	DLB *dlb;
	llc_t *llc;
	mblk_t *mp;
{
	ppa_t *ppa;
	dl_connect_ind_t *ci;
	conn_t *c, *free;
	uchar *saddr;
	int dlen, slen;
	int poll;

	ppa = dlb->dlb_ppa;
	poll = ISPF(llc);
	saddr = mp->b_rptr + DL_SADDR_OFFSET;
	slen = mp->b_rptr[1];

	/*
	 * this may be either a new attempt or a retry;
	 * if the record is in the pending table, then it is a retry.
	 */

	free = 0;
	for (c = dlb->dlb_pend; c < &dlb->dlb_pend[MAXCONIND]; ++c) {
		if (c->dlb) {
			if (!memcmp(c->remaddr, saddr, dlb->dlb_physlen))
				break;
		} else if (!free)
			free = c;
	}
	/* if already seen, ignore retry, but save poll */
	if (c < &dlb->dlb_pend[MAXCONIND]) {
		c->poll = poll;
		TRC(dlb, "rx_sabme: discard dup");
discard:
		return;
	}

	/* count all _attempted_ connections */
	nincstats(dlb, rx_conn);

	/*
	 * if cannot accept connection attempt, discard it,
	 * and perhaps the next retry by the remote will be successful
	 */

	/* room for another connection? */
	if (dlb->dlb_npend == dlb->dlb_conind) {
		TRC(dlb, "rx_sabme: discard pend table full");
		goto discard;
	}

	/* save pending connection */
	DB(assert(free););
	++dlb->dlb_npend;
	c = free;
	bcopy(saddr, c->remaddr, dlb->dlb_physlen);
	c->remsap = llc->ssap & 0xfe;
	c->poll = poll;
	c->dlb = dlb;	/* conn_t points here until connected */

	/*
	 * prepare DLPI indication
	 */

	dlen = mp->b_rptr[0];
	slen = mp->b_rptr[1];

	ci = (dl_connect_ind_t *)mp->b_rptr;

	ci->dl_primitive = DL_CONNECT_IND;
	ci->dl_correlation = (ulong)c;
	ci->dl_qos_length = 0;
	ci->dl_qos_offset = 0;
	ci->dl_growth = 0;

	/* destination address */
	ci->dl_called_addr_offset = DL_DADDR_OFFSET;
	ci->dl_called_addr_length = dlen;

	/* source address */
	ci->dl_calling_addr_offset = DL_SADDR_OFFSET;
	ci->dl_calling_addr_length = slen;

	mp->b_wptr += DL_SADDR_OFFSET + slen;

	dlb->dlb_state = DL_INCON_PENDING;

	putq(dlb->dlb_rq, mp);
}

/*
 * dl_connres - accept a connection from a remote host
 */

void
dl_connres(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_connect_res_t *res = (dl_connect_res_t *)mp->b_rptr;
	conn_t *p, *t;
	int err;

	err = 0;
	if (dlb->dlb_state != DL_INCON_PENDING)
		err = DL_OUTSTATE;
	else if (res->dl_qos_length != 0)
		err = DL_BADQOSPARAM;
	else if (!(p = findpend(dlb, res->dl_correlation)))
		err = DL_BADCORR;
	else if (!(t = findstream(dlb, res->dl_resp_token)))
		err = DL_BADTOKEN;
	else if (t->dlb == dlb)
		err = DL_BADTOKEN;

	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_CONNECT_RES, err));
		return;
	}

	/* copy connection information to responding stream */
	t->dlb->dlb_conn = t;
	bcopy(p->remaddr, t->remaddr, dlb->dlb_physlen);
	t->remsap = p->remsap;
	t->poll = 0;
	t->dlb->dlb_final = p->poll;

	/* remove pending entry */
	p->dlb = 0;
	if (--dlb->dlb_npend == 0)
		dlb->dlb_state = DL_IDLE;

	init_session(t->dlb);
	tx_rsp(t, UA | (p->poll ? PF1 : 0));

	putq(dlb->dlb_rq, okack(mp, DL_CONNECT_RES));
}
