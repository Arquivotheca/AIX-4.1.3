static char sccsid[] = "@(#)32  1.2  src/bos/kernext/dlpi/xid.c, sysxdlpi, bos41J, 9518A_all 4/28/95 16:52:30";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_testreq
 *		dl_testres
 *		dl_xidreq
 *		dl_xidres
 *		dl_xtsend
 *		rx_test
 *		rx_xid
 *		rx_xtauto
 *		rx_xtind
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
 * xid.c - XID and TEST routines
 *
 * entry points:
 *	dl_xidreq - process downstream DL_XID_REQ
 *	dl_xidres - process downstream DL_XID_RES
 *	dl_testreq - process downstream DL_TEST_REQ
 *	dl_testres - process downstream DL_TEST_RES
 *	rx_xid - process incoming XIDs
 *	rx_test - process incoming TESTs
 *
 * private routines:
 *	dl_xtsend - transmit XID or TEST
 *	rx_xtauto - automatic handling of XID and TEST
 *	rx_xtind - send upstream DL_{XID,TEST}_{IND,CON}
 */

#include "include.h"

static void dl_xtsend();
static void rx_xtauto();
static void rx_xtind();

/*
 **************************************************
 * OUTGOING, DOWNSTREAM
 **************************************************
 */

/*
 * dl_xidreq, dl_xidres, dl_testreq, dl_testres - DLPI primitive handlers
 */

void
dl_xidreq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_xtsend(dlb, mp, DL_XID_REQ, XID, CMD);
}

void
dl_xidres(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_xtsend(dlb, mp, 0, XID, RSP);
}

void
dl_testreq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_xtsend(dlb, mp, DL_TEST_REQ, TEST, CMD);
}

void
dl_testres(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_xtsend(dlb, mp, 0, TEST, RSP);
}

/*
 * dl_xtsend - common llc interface for DLPI xid and test handlers
 *
 * Note: all xid and test reqs and responses share same format
 */

static void
dl_xtsend(dlb, mp, prim, ctl, cr)
	DLB *dlb;
	mblk_t *mp;
	int prim;
	int ctl;
	int cr;
{
	dl_xid_req_t *req = (dl_xid_req_t *)mp->b_rptr;
	llc_t *llc;
	struct mbuf *m;
	uchar *daddr;
	int plen, dlen, mlen;
	int err = 0;

	/*
	 * validate request:
	 *	- correct state
	 *	- feature not disabled
	 *	- correct message length
	 *	- address completely within mblk
	 */

	if (dlb->dlb_state != DL_IDLE && dlb->dlb_state != DL_DATAXFER)
		err = DL_OUTSTATE;
	else if (ctl == XID && (dlb->dlb_xidtest & DL_AUTO_XID))
		err = DL_XIDAUTO;
	else if (ctl == TEST && (dlb->dlb_xidtest & DL_AUTO_TEST))
		err = DL_TESTAUTO;
	else if ((mlen = msgdsize(mp->b_cont)) > dlb->dlb_n1)
		err = DL_BADDATA;
	else {
		dlen = req->dl_dest_addr_length;
		daddr = mp->b_rptr + req->dl_dest_addr_offset;
		if (daddr + dlen > mp->b_wptr)
			err = DL_BADADDR;
	}

	if (err) {
discard:
		if (prim)
			putq(dlb->dlb_rq, errack(mp, prim, err));
		else
			freemsg(mp);
		return;
	}

	/*
	 * prepare frame for transmission
	 */

	if (!(m = mkframe(dlb, LLC_ULEN, mp->b_cont))) {
		incstats(dlb, no_bufs);
		err = -ENOSR;
		goto discard;
	}
	mp->b_cont = 0;	/* now owned by mbuf */

	/* complete the addressing */
	llc = setdsap(dlb, m, daddr, dlen);
	llc->ctl1 = ctl | ((req->dl_flag == DL_POLL_FINAL) ? PF1 : 0);
	llc->ssap |= cr;

	/*
	 * send the frame now
	 */

	if (err = tx_frame(dlb, m, daddr)) {
		if (prim) {
			putq(dlb->dlb_rq, errack(mp, prim, err));
			return;
		}
	}
	freeb(mp);
}

/*
 **************************************************
 * INCOMING, UPSTREAM
 **************************************************
 */

/*
 * rx_xid - process an LLC XID command or response
 */

void
rx_xid(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	if (dlb->dlb_xidtest & DL_AUTO_XID)
		rx_xtauto(dlb, m, mp);
	else
		rx_xtind(dlb, m, mp, 1);
}

/*
 * rx_test - process an LLC TEST command or response
 */

void
rx_test(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	if (dlb->dlb_xidtest & DL_AUTO_TEST)
		rx_xtauto(dlb, m, mp);
	else
		rx_xtind(dlb, m, mp, 0);
}

/*
 * rx_xtauto - handle an LLC XID or TEST command
 */

static void
rx_xtauto(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc = mtod(m, llc_t *);
	struct output_bundle b;
	uchar *saddr, *p;

	/* drop unsolicited responses */
	if (ISRSP(llc)) {
		TRC(dlb, "rx_xtauto: unsolicited response drop");
		m_freem(m);
		freeb(mp);
		return;
	}

	/* reverse LLC direction */
	swapsap(llc);
	SETRSP(llc);

	/* if XID, prepare the correct response */
	if (PRIM(llc) == XID) {
		p = m->m_data + LLC_ULEN;
		p[0] = XID_FI;
		switch (dlb->dlb_mode) {
		case DL_CLDLS:
			p[1] = XID_T1;
			p[2] = 0;
			break;
		case DL_CODLS:
			p[1] = XID_T2;
			p[2] = XID_WS;
			break;
		case DL_CODLS | DL_CLDLS:
			p[1] = XID_T12;
			p[2] = XID_WS;
			break;
		}
		m->m_len = LLC_ULEN + 3;	/* 3 = IEEE basic format len */
		m->m_pkthdr.len = m->m_len;
	}

	/* send it from whence it came */
	saddr = mp->b_rptr + DL_SADDR_OFFSET;
	(void)tx_frame(dlb, m, saddr);
	freeb(mp);
}

/*
 * rx_xtind - DLPI XID and TEST indications and confirmations
 */

static void
rx_xtind(dlb, m, mp, isxid)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
	int isxid;
{
	dl_xid_ind_t *xt;	/* xid same as test, ind same as con */
	llc_t *llc = mtod(m, llc_t *);
	int dlen, slen;

	if (!canput(dlb->dlb_rq)) {
		m_freem(m);
		freeb(mp);
		return;
	}

	dlen = mp->b_rptr[0];
	slen = mp->b_rptr[1];

	xt = (dl_xid_ind_t *)mp->b_rptr;

	if (isxid) xt->dl_primitive = ISRSP(llc) ? DL_XID_CON  : DL_XID_IND;
	else	   xt->dl_primitive = ISRSP(llc) ? DL_TEST_CON : DL_TEST_IND;

	xt->dl_flag = ISPF(llc) ? DL_POLL_FINAL : 0;
	xt->dl_dest_addr_offset = DL_DADDR_OFFSET;
	xt->dl_dest_addr_length = dlen;
	xt->dl_src_addr_offset = DL_SADDR_OFFSET;
	xt->dl_src_addr_length = slen;

	mp->b_wptr += DL_SADDR_OFFSET + slen;

	if (!(mp->b_cont = mbuf_to_mblk(m, LLC_ULEN))) {
		freeb(mp);
		m_freem(m);
		return;
	}
	putq(dlb->dlb_rq, mp);
}
