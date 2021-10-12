static char sccsid[] = "@(#)30  1.1  src/bos/kernext/dlpi/tx.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:38";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: mkframe
 *		tx_adm_dm
 *		tx_disc
 *		tx_ether
 *		tx_frame
 *		tx_frame2
 *		tx_raw
 *		tx_reset
 *		tx_rsp
 *		tx_sabme
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
 * tx.c - transmit LLC frames
 *
 * public routines:
 *	mkframe - make an LLC frame
 *	tx_raw - raw send
 *	tx_frame - transmit a frame, using DRD if necessary
 *	tx_frame2 - transmit a frame with explicit source routing
 *	tx_rsp - transmit a response frame; does not retry
 *	tx_adm_dm - transmit a DM response with no DLB
 *	tx_sabme - transmit a SABME with retries
 *	tx_disc - transmit a DISC with retries
 *	tx_reset - transmit a reset (SABME) with retries
 */

#include "include.h"

static struct output_bundle zerobundle;

/*
 * mkframe - make an LLC frame
 *
 * streams message is unmolested on failure
 */

struct mbuf *
mkframe(dlb, llclen, mp)
	DLB *dlb;
	int llclen;
	mblk_t *mp;
{
	struct mbuf *hdr, *m;
	llc_t *llc;

	/*
	 * frames must have a pkthdr;
	 * reserve space for mac header
	 */

	if (!(hdr = m_gethdr(M_DONTWAIT, MT_DATA)))
		return 0;
	MH_ALIGN(hdr, LLC_SNAP_LEN);
	hdr->m_len = 0;
	hdr->m_pkthdr.len = 0;
	hdr->m_pkthdr.rcvif = 0;

	/* if there is to be an llc header */
	if (llclen) {
		llc = mtod(hdr, llc_t *);
		llc->ssap = dlb->dlb_ssap;
		llc->dsap = dlb->dlb_dsap;
		hdr->m_len += llclen;
		hdr->m_pkthdr.len += llclen;
	}

	/* if there is data */
	if (mp) {
		if (!(m = mblk_to_mbuf(hdr, mp, dlb->dlb_netware))) {
			m_free(hdr);
			return 0;
		}
		hdr = m;
	}

	return hdr;
}

/*
 * tx_raw - raw send
 */

void
tx_raw(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	struct mbuf *m;
	NDD *ndd = dlb->dlb_ndd;

	if (m = mblk_to_mbuf(0, mp, 0)) {
		if ((*ndd->ndd_output)(ndd, m))
			incstats(dlb, tx_discards);
		else
			incstats(dlb, tx_pkts);
	} else
		freemsg(mp);
}

/*
 * tx_ether - low-level routine to transmit an ethernet frame
 */

int
tx_ether(dlb, m, bp)
	DLB *dlb;
	struct mbuf *m;
	struct output_bundle *bp;
{
	int err;

	if (err = (*dlb->dlb_output)(bp, m, dlb->dlb_ndd))
		incstats(dlb, tx_discards);
	else
		incstats(dlb, tx_pkts);
	return -err;
}

/*
 * tx_frame - transmit a frame, using DRD if necessary
 */

int
tx_frame(dlb, m, daddr)
	DLB *dlb;
	struct mbuf *m;
	uchar *daddr;
{
	int rc = 0;
	int tx_frame2();

	if (dlb->dlb_drd)
		drd(dlb, m, daddr, tx_frame2);
	else
		rc = tx_frame2(dlb, m, daddr, dlb->dlb_seg, dlb->dlb_seglen);
	return rc;
}

/*
 * tx_frame2 - transmit a frame with explicit source routing
 */

int
tx_frame2(dlb, m, daddr, segp, seglen)
	DLB *dlb;
	struct mbuf *m;
	uchar *daddr, *segp;
	int seglen;
{
	struct output_bundle b;
	llc_t *llc;
	int err;

	b = zerobundle;
	b.key_to_find = daddr;
	b.helpers.pkt_format = NS_INCLUDE_LLC;
	b.helpers.segp = segp;
	b.helpers.seglen = seglen;

	llc = mtod(m, llc_t *);
	TRC(dlb, LLCMSG(llc), "<-", CRPF(llc), GETNR(llc), GETNS(llc));

	if (err = (*dlb->dlb_output)(&b, m, dlb->dlb_ndd))
		incstats(dlb, tx_discards);
	else
		incstats(dlb, tx_pkts);
	return -err;
}

/*
 * tx_rsp - transmit a response frame; does not retry
 */

void
tx_rsp(c, ctl)
	conn_t *c;
	uchar ctl;
{
	struct mbuf *m;
	llc_t *llc;
	DLB *dlb = c->dlb;

	/* prepare frame for transmission */
	if (m = mkframe(dlb, LLC_ULEN, 0)) {
		llc = mtod(m, llc_t *);
		llc->ctl1 = ctl;
		llc->dsap = c->remsap;
		llc->ssap |= RSP;

		(void)tx_frame(dlb, m, c->remaddr);
	} else
		incstats(dlb, no_bufs);
}

/* 
 * tx_adm_dm - transmit a DM response with no DLB
 */

void
tx_adm_dm(ndd, m, isr)
	NDD *ndd;
	struct mbuf *m;
	struct isr_data_ext *isr;
{
	llc_t *llc = mtod(m, llc_t *);
	int poll = ISPF(llc) ? PF1 : 0;
	struct output_bundle b;

	swapsap(llc);
	llc->ctl1 = DM | poll;
	llc->ssap |= RSP;
	m->m_len = LLC_ULEN;
	m->m_pkthdr.len = LLC_ULEN;

	b = zerobundle;
	b.key_to_find = isr->srcp;
	b.helpers.pkt_format = NS_INCLUDE_LLC;

	(void)(*ndd->ndd_demuxer->nd_address_resolve)(&b, m, ndd);
}

/*
 * tx_sabme - transmit a SABME with retries
 *
 * this routine will retry itself retry times spaced T1 apart
 */

void
tx_sabme(arg)
	ulong *arg;
{
	DLB *dlb;
	struct mbuf *m;
	llc_t *llc;

	dlb = (DLB *)arg[0];

	/* saw a sabme while sending connection */
	if (dlb->dlb_s_flag) {
		mblk_t *mp;
		if (!(mp = dl_gethdr(dlb, 0))) {
			tq_start(dlb->dlb_tq1, 1);
		} else if (dlb->dlb_localreset) {
			init_session(dlb);
			putq(dlb->dlb_rq, okack(mp, DL_RESET_RES));
		} else {
			init_session(dlb);
			dl_conncon(dlb, mp);
		}
		return;
	}

	/* try sending up to retry times */
	if (--dlb->dlb_retry < 0) {
		mblk_t *mp;
		if (!(mp = dl_gethdr(dlb, 0))) {
			tq_start(dlb->dlb_tq1, 1);
			return;
		}
		/* XXX should flush drd cache and retry (maybe stale route)? */
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_PROVIDER, DL_CONREJ_DEST_UNKNOWN);
		return;
	}

	/* T1 acts somewhat like a bufcall for us if mkframe fails */
	DB(assert(dlb->dlb_tq1->fired););
	dlb->dlb_tq1->func = tx_sabme;
	dlb->dlb_tq1->arg[0] = (ulong)dlb;
	tq_start(dlb->dlb_tq1, dlb->dlb_t1);

	/* prepare frame for transmission */
	if (m = mkframe(dlb, LLC_ULEN, 0)) {
		llc = mtod(m, llc_t *);
		llc->ctl1 = SABME | (dlb->dlb_poll ? PF1 : 0);
		(void)tx_frame(dlb, m, dlb->dlb_remaddr);
	} else
		incstats(dlb, no_bufs);
}

/*
 * tx_disc - transmit a DISC with retries
 *
 * this routine will retry itself retry times spaced T1 apart
 */

void
tx_disc(arg)
	ulong *arg;
{
	DLB *dlb;
	struct mbuf *m;
	llc_t *llc;

	dlb = (DLB *)arg[0];

	/* try sending up to retry times */
	if (--dlb->dlb_retry < 0) {
		term_session(dlb);
		dl_okack(dlb, DL_DISCONNECT_REQ);
		return;
	}

	/* prepare frame for transmission */
	if (m = mkframe(dlb, LLC_ULEN, 0)) {
		llc = mtod(m, llc_t *);
		llc->ctl1 = DISC | (dlb->dlb_poll ? PF1 : 0);
		(void)tx_frame(dlb, m, dlb->dlb_remaddr);
	} else
		incstats(dlb, no_bufs);

	/* T1 acts somewhat like a bufcall for us if mkframe fails */
	DB(assert(dlb->dlb_tq1->fired););
	dlb->dlb_tq1->func = tx_disc;
	dlb->dlb_tq1->arg[0] = (ulong)dlb;
	tq_start(dlb->dlb_tq1, dlb->dlb_t1);
	return;
}

/*
 * tx_reset - transmit a reset (SABME) with retries
 *
 * this routine will retry itself retry times spaced T1 apart
 *
 * mostly like tx_sabme, except for responses to user
 */

void
tx_reset(arg)
	ulong *arg;
{
	DLB *dlb;
	struct mbuf *m;
	llc_t *llc;

	dlb = (DLB *)arg[0];

	/* saw a sabme while sending connection */
	if (dlb->dlb_s_flag) {
		mblk_t *mp;
		if (!(mp = dl_gethdr(dlb, 0))) {
			tq_start(dlb->dlb_tq1, 1);
			return;
		}
		init_session(dlb);
		dl_resetcon(dlb, mp);
		return;
	}

	/* try sending up to retry times */
	if (--dlb->dlb_retry < 0) {
		mblk_t *mp;
		if (!(mp = dl_gethdr(dlb, 0))) {
			tq_start(dlb->dlb_tq1, 1);
			return;
		}
		term_session(dlb);
		dl_discind(dlb, mp, 0, DL_PROVIDER,DL_DISC_TRANSIENT_CONDITION);
		return;
	}

	/* T1 acts somewhat like a bufcall for us if mkframe fails */
	DB(assert(dlb->dlb_tq1->fired););
	dlb->dlb_tq1->func = tx_reset;
	dlb->dlb_tq1->arg[0] = (ulong)dlb;
	tq_start(dlb->dlb_tq1, dlb->dlb_t1);

	/* prepare frame for transmission */
	if (m = mkframe(dlb, LLC_ULEN, 0)) {
		llc = mtod(m, llc_t *);
		llc->ctl1 = SABME | (dlb->dlb_poll ? PF1 : 0);
		(void)tx_frame(dlb, m, dlb->dlb_remaddr);
	} else
		incstats(dlb, no_bufs);
}
