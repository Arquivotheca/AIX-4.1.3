static char sccsid[] = "@(#)25  1.2  src/bos/kernext/dlpi/misc.c, sysxdlpi, bos41J, 9519A_all 5/4/95 15:09:21";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_daddr
 *		dl_error
 *		dl_flush
 *		dl_gethdr
 *		dl_okack
 *		dl_saddr
 *		dl_trace
 *		eproto
 *		findconn
 *		findpend
 *		findstream
 *		notsupp
 *		setconn
 *		setdsap
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
 * misc.c - miscellaneous support routines
 *
 * public routines:
 *	dl_trace, dl_error - shorthands for strlog
 *	dl_flush - flush stream, and internal queues
 *	dl_gethdr - get a big enough mblk for a DLPI primitive
 *	setdsap - CLDLS dsap setup
 *	dl_daddr - format a destination address
 *	dl_saddr - format a source address
 *	eproto - signal a fatal protocol error
 *	notsupp - reply to an unsupported request
 *	dl_okack - send an okack
 * public, searching routines:
 *	setconn - fill in a connection record
 *	findconn - find the connection record
 *	findpend - get pointer to pointer to pending connection record
 *	findstream - get pointer to token stream's connection record
 */

#include "include.h"

/*
 * dl_trace, dl_error - shorthands for strlog
 */

dl_trace(dlb, m, a1, a2, a3, a4)
	DLB *dlb;
	char *m;
	int a1,a2,a3,a4;
{
	/* mi_strlog(q, level, flags, fmt, ...) */
	queue_t *q = dlb ? dlb->dlb_wq : 0;
	mi_strlog(q, 0, SL_TRACE, m, a1, a2, a3, a4);
}

dl_error(dlb, m, a1, a2, a3, a4)
	DLB *dlb;
	char *m;
	int a1,a2,a3,a4;
{
	queue_t *q = dlb ? dlb->dlb_wq : 0;
	mi_strlog(q, 0, SL_ERROR, m, a1, a2, a3, a4);
}

/*
 * dl_flush - flush stream, and internal queues
 */

void
dl_flush(dlb, how)
	DLB *dlb;
	int how;
{
	if (how & FLUSHR)
		flushq(dlb->dlb_rq, FLUSHDATA);
	if (how & FLUSHW) {
		dl_deq(dlb);
		flushq(dlb->dlb_wq, FLUSHDATA);
	}
	putctl1(dlb->dlb_rq->q_next, M_FLUSH, how);
}

/*
 * dl_gethdr - get a big enough mblk for a DLPI primitive
 */

mblk_t *
dl_gethdr(dlb, extra)
	DLB *dlb;
	int extra;
{
	mblk_t *mp;

	if (mp = allocb(DLPIHDR_LEN + extra, BPRI_HI)) {
		mp->b_datap->db_type = M_PROTO;
		return mp;
	}
	incstats(dlb, no_bufs);
	return 0;
}

/*
 * setdsap - CLDLS dsap handler, adds snap if SAP_SNAP
 *
 * returns ptr to llc
 */

llc_t *
setdsap(dlb, m, daddr, dlen)
	DLB *dlb;
	struct mbuf *m;
	uchar *daddr;
	int dlen;
{
	int plen;
	llc_t *llc;

	llc = mtod(m, llc_t *);
	plen = dlb->dlb_physlen;

	if (dlen <= plen || !daddr[plen]) {
		BCOPY(&dlb->dlb_llc, llc, LLC_SNAP_LEN);
	} else {
		llc->dsap = daddr[plen];
		if (llc->dsap == SAP_SNAP)
			BCOPY(&daddr[plen+1], ((uchar*)llc)+LLC_ULEN, SNAP_LEN);
	}
	if (llc->dsap == SAP_SNAP) {
		m->m_len += SNAP_LEN;
		m->m_pkthdr.len += SNAP_LEN;
	}

	return llc;
}

/*
 * dl_daddr - format a destination address
 */

int
dl_daddr(p, fmt, isr)
	uchar *p;
	int fmt;
	struct isr_data_ext *isr;
{
	llc_t *llc = (llc_t *)isr->llcp;
	int dlen = 0;

	if (fmt != NS_PROTO_DL_DONTCARE) {
		/* bcopy optimization, if possible */
		dlen = isr->dstlen;
		if (dlen == PHYSLEN) {
			BCOPY(isr->dstp, p, PHYSLEN);
		} else
			bcopy(isr->dstp, p, dlen);

		if (fmt == NS_PROTO_DL_COMPAT) {
			p[dlen++] = llc->dsap;
			if (llc->dsap == SAP_SNAP) {
				snap_t *snap = (snap_t *)(isr->llcp + LLC_ULEN);
				BCOPY(snap, p + dlen, SNAP_LEN);
				dlen += SNAP_LEN;
			}
		}
	}
	return dlen;
}

/*
 * dl_saddr - format a source address
 */

int
dl_saddr(p, fmt, isr)
	uchar *p;
	int fmt;
	struct isr_data_ext *isr;
{
	llc_t *llc = (llc_t *)isr->llcp;
	int slen = 0;

	if (fmt != NS_PROTO_DL_DONTCARE) {
		/* bcopy optimization, if possible */
		slen = isr->srclen;
		if (slen == PHYSLEN) {
			BCOPY(isr->srcp, p, PHYSLEN);
		} else
			bcopy(isr->srcp, p, slen);

		switch (fmt) {
		case NS_PROTO:
		case NS_PROTO_SNAP:
			BCOPY(llc, p + slen, LLC_ULEN);
			slen += LLC_ULEN;
			break;
		case NS_PROTO_DL_COMPAT:
			p[slen++] = llc->ssap & 0xfe;
			break;
		}

		if (fmt != NS_INCLUDE_LLC) {
			if (llc->dsap == SAP_SNAP) {
				snap_t *snap = (snap_t *)(isr->llcp + LLC_ULEN);
				BCOPY(snap, p + slen, SNAP_LEN);
				slen += SNAP_LEN;
			}
#ifdef	XXX_REPORT_SEGMENTS
			/* design says not supposed to report segments */
			if (isr->seglen) {
				bcopy(isr->segp, p + slen, isr->seglen);
				slen += isr->seglen;
			}
#endif
		}
	}
	return slen;
}

/*
 * eproto - signal a fatal protocol error
 */

void
eproto(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	abort_session(dlb);
	mp->b_datap->db_type = M_ERROR;
	mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
	*mp->b_wptr++ = EPROTO;
	putq(dlb->dlb_rq, mp);
}

/*
 * notsupp - reply to an unsupported request
 */

void
notsupp(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	putq(dlb->dlb_rq, errack(mp, *(ulong *)mp->b_rptr, DL_NOTSUPPORTED));
}

/*
 * dl_okack - send an okack
 */

void
dl_okack(dlb, prim)
	DLB *dlb;
	int prim;
{
	mblk_t *mp;
	dl_ok_ack_t *ok;

	mp = dl_gethdr(dlb, 0);
	if (!mp) {
		/* XXX recovery */
		return;
	}
	ok = (dl_ok_ack_t *)mp->b_rptr;
	ok->dl_primitive = DL_OK_ACK;
	ok->dl_correct_primitive = prim;
	mp->b_wptr += sizeof(dl_ok_ack_t);

	if (prim == DL_DISCONNECT_REQ)
		dl_flush(dlb, FLUSHRW);
	putq(dlb->dlb_rq, mp);
}

/*
 **************************************************
 * Connection table searching routines
 **************************************************
 *
 * Search Note:
 * Searching for a remote connection via the local sap (incoming dsap)
 * is correct because connections between saps must be 1:1 between the
 * two stations.  Thus, the incoming ssap must be uniquely paired with
 * the local sap, and using the local sap to find the remote station
 * will find the correct match.
 */

/*
 * setconn - fill in a connection record
 *
 * side-effects: dlb_conn set if successful
 */

int
setconn(dlb, locsap, daddr, remsap, listen)
	DLB *dlb;
	uchar locsap;
	uchar *daddr;
	uchar remsap;
	int listen;
{
	ppa_t *ppa = dlb->dlb_ppa;
	conn_t *c;
	DLB *ldlb;
	INTR_LOCK_DECL;

	INTR_LOCK();
	/* fail if another non-listener, or dup attempt to listen */
	if (c = findconn(ppa, daddr, locsap, 0)) {
		if (!c->listen || listen) {
			INTR_UNLOCK();
			return DL_BADADDR;
		}
		/* check pending list of listener */
		ldlb = c->dlb;
		for (c = ldlb->dlb_pend; c < &ldlb->dlb_pend[MAXCONIND]; ++c) {
			if (!memcmp(daddr, c->remaddr, ppa->ndd->ndd_addrlen)) {
				INTR_UNLOCK();
				return DL_BADADDR;
			}
		}
	}

	/* address not in table; find our conn_t, and fill it in */
	for (c = ppa->saps[SAPINDEX(locsap)][0]; c; c = c->next) {
		if (c->dlb == dlb)
			break;
	}
	DB(assert(c););	/* it must exist */

	bcopy(daddr, c->remaddr, dlb->dlb_physlen);
	c->remsap = remsap;
	c->listen = listen;

	dlb->dlb_conn = c;
	INTR_UNLOCK();

	return 0;
}

/*
 * findconn - find the connection record
 *
 * Logic:
 *	UI traffic is preferentially routed to a CLDLS-only stream.
 *	If there is no such stream, and no stream has asked for CLDLS-also,
 *	then the UI frame should be silently discarded(!).  XID and TEST
 *	traffic are preferentially routed to a CODLS stream, then to a
 *	CLDLS-only stream (if one exists).  Connection-oriented traffic
 *	should only be routed to CODLS streams, and to the listening
 *	stream if no other streams connected to the remote address.
 */

conn_t *
findconn(ppa, daddr, locsap, prim)
	ppa_t *ppa;
	uchar *daddr;
	uchar locsap;
	uchar prim;
{
	conn_t *c, *listener, *xid;
	DLB *dlb;
	INTR_LOCK_DECL;

	listener = xid = 0;
	INTR_LOCK();
	if (c = ppa->saps[SAPINDEX(locsap)][1]) {
		/* Netware hack: 802.3 sap 0xff has no llc(!) */
		if (SAPINDEX(locsap) == 0) {
			INTR_UNLOCK();
			return c;
		}
		switch (prim) {
		case UI:
		case XID:
		case TEST:
			if (locsap == SAP_SNAP) {
				do {
					/* NB: really a SNAP compare */
					if (!memcmp(daddr,c->remaddr,SNAP_LEN))
						break;
				} while (c = c->next);
				INTR_UNLOCK();
				return c;
			}
			if (prim == UI) {
				INTR_UNLOCK();
				return c;
			}
			xid = c;
		}
	}
	for (c = ppa->saps[SAPINDEX(locsap)][0]; c; c = c->next) {
		if (!memcmp(daddr, c->remaddr, ppa->ndd->ndd_addrlen)) {
			if (prim == UI && !(c->dlb->dlb_mode & DL_CLDLS))
				c = listener = 0;
			break;
		}
		if (c->listen)
			listener = c;
	}
	INTR_UNLOCK();
	return c ? c : (xid ? xid : listener);
}

/*
 * findpend - get pointer to pointer to pending connection record
 */

conn_t *
findpend(dlb, corr)
	DLB *dlb;
	conn_t *corr;
{
	conn_t *c;
	int found = 0;
	INTR_LOCK_DECL;

	INTR_LOCK();
	for (c = dlb->dlb_pend; c < &dlb->dlb_pend[MAXCONIND]; ++c) {
		if (corr == c) {
			found = 1;
			break;
		}
	}
	INTR_UNLOCK();
	return found ? c : 0;
}

/* 
 * findstream - get pointer to token stream's connection record
 */

conn_t *
findstream(dlb, token)
	DLB *dlb;
	DLB *token;
{
	conn_t *c;
	ppa_t *ppa;
	INTR_LOCK_DECL;

	ppa = dlb->dlb_ppa;
	INTR_LOCK();
	for (c = ppa->saps[SAPINDEX(dlb->dlb_ssap)][0]; c; c = c->next) {
		if (c->dlb == token)
			break;
	}
	INTR_UNLOCK();
	return c;
}
