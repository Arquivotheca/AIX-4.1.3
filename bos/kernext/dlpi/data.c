static char sccsid[] = "@(#)13  1.2  src/bos/kernext/dlpi/data.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:37:59";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: chktimers
 *		dl_datareq
 *		dl_deq
 *		dl_enq
 *		dl_nukem
 *		dl_rexmit
 *		expired_t1
 *		expired_t2
 *		expired_ti
 *		reset_session
 *		rx_i
 *		rx_nr
 *		rx_super
 *		tx_i
 *		tx_super
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
 * data.c - connection-oriented data transfer
 *
 * public routines:
 *	dl_deq - remove all messages from rexmit queue
 *	dl_datareq - send data
 *	dl_rexmit - retransmit I frames
 *	tx_super - send an S-frame modified according to flags
 *	rx_i - receive data
 *	rx_super - process incoming S-frames
 *	dl_nukem - toss all unprocessed input frames (mbufs)
 *	reset_session - retries failed, initiate a provider reset to recover
 *	expired_t1 - T1 expiration; check on poll and retransmissions
 *	expired_t2 - T2 expiration; sent-REJ timeout, or delayed ack timeout
 *	expired_ti - Ti expiration; line was idle, ensure connection still up
 * private routines:
 *	dl_enq - add message to rexmit queue
 *	tx_i - transmit an I frame
 *	rx_nr - common I/S-frame responses
 *	chktimers - check (ensure) that the correct timers are ticking
 */

#include "include.h"

/*
 **************************************************
 * Data transmission
 **************************************************
 */

void expired_t1(), expired_t2(), expired_ti();
void dl_rexmit();
static void rx_nr(), chktimers();
static int tx_i();

/*
 * dl_enq - add message to rexmit queue
 */

static void
dl_enq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	mp->b_next = 0;
	if (dlb->dlb_qtail)
		dlb->dlb_qtail->b_next = mp;
	else
		dlb->dlb_qhead = mp;
	dlb->dlb_qtail = mp;
}

/*
 * dl_deq - remove all messages from rexmit queue
 */

void
dl_deq(dlb)
	DLB *dlb;
{
	mblk_t *mp, *nextmp;

	for (mp = dlb->dlb_qhead; mp; mp = nextmp) {
		nextmp = mp->b_next;
		freemsg(mp);
	}
	dlb->dlb_qhead = dlb->dlb_qtail = 0;
}

/*
 * dl_datareq - send data
 *
 * returns 0 if sent; !0 otherwise (and caller responsible for mp)
 */

int
dl_datareq(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	mblk_t *dmp;
	uchar ns;
	int len;

	switch (dlb->dlb_state) {
	case DL_IDLE:
	case DL_PROV_RESET_PENDING:
		TRC(dlb, "dl_datareq: state %d drop", dlb->dlb_state);
		freemsg(mp);	/* DLPI 2.0 4.2.8 */
		return 0;
	case DL_DATAXFER:
		break;
	default:
		eproto(dlb, mp);
		return 0;
	}

	/* ensure this is a legal message size */
	len = msgdsize(mp);
	if (len < 1 || len > dlb->dlb_n1) {
		eproto(dlb, mp);	/* DLPI 2.0 4.2.8 */
		return 0;
	}

	/* if remote is busy, or window closed, cannot send */
	if ((dlb->dlb_vb & FB_REMOTE) ||
	    (dlb->dlb_va + dlb->dlb_k == dlb->dlb_vs))
		return 1;

	/* stop idle timer */
	tq_stop(dlb->dlb_tqi);

	/* dup message so can rexmit if need be */
	if (!(dmp = dupmsg(mp))) {
		incstats(dlb, no_bufs);
		(void)bufcall(1, BPRI_HI, qenable, dlb->dlb_wq);
		return 1;
	}

	/* piggy-backed response? */
	if (!(dlb->dlb_vb & FB_REJWAIT)) {
		/* stop delayed ack - acking now */
		tq_stop(dlb->dlb_tq2);
		dlb->dlb_ir_ct = dlb->dlb_n3;
	}

	/* remember frame number for rexmit, ack */
	ns = dlb->dlb_vs;
	incr(dlb->dlb_vs);
	mp->b_prev = (mblk_t *)ns;
	dl_enq(dlb, mp);	/* save message on rexmit queue */

	/* send the frame; start T1 if stopped */
	(void)tx_i(dlb, dmp, ns);
	tq_start(dlb->dlb_tq1, dlb->dlb_t1);
	return 0;
}

/*
 * dl_rexmit - retransmit I frames
 */

void
dl_rexmit(dlb)
	DLB *dlb;
{
	mblk_t *mp, **pp;
	int sent;

	TRC(dlb, "dl_rexmit: retransmitting I-frames");

	/* retransmit until all gone or an error */
	sent = 0;
	for (pp = &dlb->dlb_qhead; *pp; pp = &(*pp)->b_next) {
		if (!(mp = dupmsg(*pp))) {
			TRC(dlb, "dl_rexmit: cannot dup");
			incstats(dlb, no_bufs);
			break;
		}
		if (tx_i(dlb, mp, (*pp)->b_prev))
			break;
		sent = 1;
		nincstats(dlb, rexmit);
		/*
		 * Stop retransmission upon receipt of any frame from
		 * the remote machine.  This rx'd frame _may_ be an
		 * RNR.  If not, all this does is slow down sends,
		 * which isn't a big deal anyway because this is only
		 * done if the remote side isn't responding, or REJected
		 * several frames.
		 */
		if (dlb->dlb_mhead)
			break;
	}
	if (sent)
		tq_restart(dlb->dlb_tq1, dlb->dlb_t1);
	else
		TRC(dlb, "dl_rexmit: no outstanding I-frames");
}

/*
 * tx_i - transmit an I frame
 *
 * returns success (0) or fail (errno); discards mp
 */

static int
tx_i(dlb, mp, ns)
	DLB *dlb;
	mblk_t *mp;
	uchar ns;
{
	struct mbuf *m;
	llc_t *llc;
	int mlen, err;

	if (!(m = mkframe(dlb, LLC_ILEN, mp))) {
		freemsg(mp);
		incstats(dlb, no_bufs);
		TRC(dlb, "tx_i: mkframe failed");
		return ENOMEM;
	}
	mlen = m->m_pkthdr.len;

	llc = mtod(m, llc_t *);
	SETNR(llc, dlb->dlb_vr);
	SETNS(llc, ns);
	SETCMD(llc);

	if (err = tx_frame(dlb, m, dlb->dlb_conn->remaddr))
		TRC(dlb, "tx_i: tx_frame failed %d", err);
	else
		addstats(dlb, tx_bytes, mlen);

	return err;
}

/*
 * tx_super - send an S-frame modified according to flags
 */

int
tx_super(dlb, flags)
	DLB *dlb;
	int flags;
{
	struct mbuf *m;
	llc_t *llc;
	int err;

	if (!(m = mkframe(dlb, LLC_ILEN, 0)))
		return ENOMEM;

	llc = mtod(m, llc_t *);
	if ((dlb->dlb_vb & (FB_REJECT | FB_REJWAIT)) == FB_REJECT) {
		llc->ctl1 = REJ;
		nincstats(dlb, tx_rej);
	} else if (dlb->dlb_vb & FB_LOCAL) {
		llc->ctl1 = RNR;
		nincstats(dlb, tx_rnr);
	} else
		llc->ctl1 = RR;
	SETNR(llc, dlb->dlb_vr);
	if (flags & S_RSP)
		SETRSP(llc);
	if (flags & S_POLL)
		SETPF2(llc);

	/* whenever an S-frame is sent, stop T2 and reset N3 */
	if (!(dlb->dlb_vb & FB_REJECT))
		tq_stop(dlb->dlb_tq2);
	dlb->dlb_ir_ct = dlb->dlb_n3;

	if (err = tx_frame(dlb, m, dlb->dlb_conn->remaddr))
		TRC(dlb, "tx_super: tx_frame failed %d", err);

	/* if cmd and poll, set polling and start poll timer */
	if (flags == S_POLL) {
		if (!(dlb->dlb_vb & FB_POLL)) {
			dlb->dlb_vb |= FB_POLL;
			dlb->dlb_retry = dlb->dlb_n2;
		}
		tq_restart(dlb->dlb_tq1, dlb->dlb_t1);
	}
	return err;
}

/*
 **************************************************
 * Data reception
 **************************************************
 */

/*
 * rx_i - receive data
 */

void
rx_i(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc = mtod(m, llc_t *);
	uchar ns, nr, pf, rsp;

	DB(assert(mp == 0););

	/* no data if stream isn't expecting it */
	if (dlb->dlb_state != DL_DATAXFER) {
		m_freem(m);
		return;
	}

	ns = GETNS(llc);
	nr = GETNR(llc);
	pf = ISPF(llc);
	rsp = ISRSP(llc);

	/* check for local busy */
	if (!canput(dlb->dlb_rq))
		local_busy(dlb);

	/* unexpected NS - check window, enter REJ */
	if (ns != dlb->dlb_vr) {
		int t = dlb->dlb_vr - ns;
		m_freem(m);
		if (t < 0)
			t += 128;
		if (t > dlb->dlb_k) {
			TRC(dlb, "rx_i: reject drop (t = %d)", t);
			if (!(dlb->dlb_vb & FB_REJWAIT)) {
				dlb->dlb_vb |= FB_REJECT;
				(void)tx_super(dlb, S_RSP | pf);
				dlb->dlb_vb |= FB_REJWAIT;
				dlb->dlb_ir_ct = dlb->dlb_n3;
				tq_restart(dlb->dlb_tq2, dlb->dlb_t2);
			}
			rx_nr(dlb, nr, rsp, pf);
		}
		return;
	}

	/* valid frame - update local variables */

	/* end of sent-REJ */
	if (dlb->dlb_vb & FB_REJWAIT) {
		TRC(dlb, "rx_i: sent reject ended");
		dlb->dlb_vb &= ~(FB_REJECT|FB_REJWAIT);
		tq_stop(dlb->dlb_tq2);
	}

	/* if I response with final, clear remote busy */
	if (rsp && pf) {
		dlb->dlb_vb &= ~FB_REMOTE;
		qenable(dlb->dlb_wq);
		tq_stop(dlb->dlb_tqi);
	}

	/* convert to M_DATA, count only if successful */
	if (mp = mbuf_to_mblk(m, LLC_ILEN)) {
		/* count frame as received */
		incr(dlb->dlb_vr);
		--dlb->dlb_ir_ct;
	}

	/* always handle common ack actions, even if drop frame */
	rx_nr(dlb, nr, rsp, pf);

	/* if conversion failed, drop frame */
	if (!mp) {
		TRC(dlb, "rx_i: mbuf convert drop");
		m_freem(m);
		return;
	}

	/* send data upstream */
	addstats(dlb, rx_bytes, m->m_pkthdr.len);
	putq(dlb->dlb_rq, mp);
}

/*
 * rx_super - process incoming S-frames
 */

void
rx_super(dlb, m, mp)
	DLB *dlb;
	struct mbuf *m;
	mblk_t *mp;
{
	llc_t *llc;
	uchar prim, rsp, nr, pf;

	DB(assert(mp == 0););

	/* toss bogus S-frame if not connected */
	if (dlb->dlb_state != DL_DATAXFER) {
		m_freem(m);
		return;
	}

	llc = mtod(m, llc_t *);
	prim = PRIM(llc);
	rsp = ISRSP(llc);
	nr = GETNR(llc);
	pf = ISPF(llc);
	m_freem(m);

	switch (prim) {
	case RNR:
		if (!(dlb->dlb_vb & FB_REMOTE))
			dlb->dlb_vb |= FB_REMOTE;
		nincstats(dlb, rx_rnr);
		break;
	case REJ:
		/* XXX if nr not in rexmitq, cannot recover - so reset */
		dlb->dlb_flags |= FD_REXMIT;
		if (dlb->dlb_vb & FB_REMOTE) {
			dlb->dlb_vb &= ~FB_REMOTE;
			qenable(dlb->dlb_wq);
			tq_stop(dlb->dlb_tqi);
		}
		nincstats(dlb, rx_rej);
		break;
	case RR:
		if (dlb->dlb_vb & FB_REMOTE) {
			dlb->dlb_vb &= ~FB_REMOTE;
			qenable(dlb->dlb_wq);
			tq_stop(dlb->dlb_tqi);
		}
		break;
	default:
		DB(assert(0););
		return;
	}

	/* common responses */
	rx_nr(dlb, nr, rsp, pf);
}

/*
 * rx_nr - common I/S-frame responses
 */

static void
rx_nr(dlb, nr, rsp, pf)
	DLB *dlb;
	uchar nr, rsp, pf;
{
	mblk_t *mp;

	if (pf) {
		if (rsp) {
			/*
			 * If we are FB_POLL, this is a response to a poll
			 * we sent because of T1, so stop T1.
			 * Otherwise, FB_REMOTE was just cancelled (see rx_i),
			 * so if there are unacked I frames, they
			 * must now be retransmitted.
			 */
			if (dlb->dlb_vb & FB_POLL) {
				dlb->dlb_vb &= ~FB_POLL;
				tq_stop(dlb->dlb_tq1);
			}
			if (dlb->dlb_va != dlb->dlb_vs) {
				dlb->dlb_flags |= FD_REXMIT;
				qenable(dlb->dlb_wq);
			}
		} else {
			(void)tx_super(dlb, S_RSP|S_FINAL);
		}
	}

	/* this ack'd some I frames */
	if (nr != dlb->dlb_va) {
		/* remove ack'd I frames */
		dlb->dlb_va = nr;
		while (mp = dlb->dlb_qhead) {
			if ((uchar)mp->b_prev == nr)
				break;
			dlb->dlb_qhead = mp->b_next;
			mp->b_next = mp->b_prev = 0;
			freemsg(mp);
		}
		if (!dlb->dlb_qhead)
			dlb->dlb_qtail = 0;

		/* restart retry counter, and T1 (if needed) */
		dlb->dlb_retry = dlb->dlb_n2;
		if (dlb->dlb_va != dlb->dlb_vs)
			tq_restart(dlb->dlb_tq1, dlb->dlb_t1);
		else
			tq_stop(dlb->dlb_tq1);

		/* if deferred frames, allow them now */
		if (dlb->dlb_flags & FD_QENABLE)
			qenable(dlb->dlb_wq);
	}

	/* if we just received the N3rd I-frame, ack it now */
	if (dlb->dlb_ir_ct == 0)
		(void)tx_super(dlb, S_RSP);

	/* keep the correct timers running */
	chktimers(dlb);
}

/*
 * chktimers - check (ensure) that the correct timers are ticking
 */

static void
chktimers(dlb)
	DLB *dlb;
{
	int i = 0;

	/*
	 * We (almost) always need a timer running; see which kind:
	 *
	 * If a send-poll or sent I-frames are outstanding, start T1.
	 * If a sent-REJ or recvd I-frames are outstanding, start T2.
	 * If remote busy or no other timers are ticking, start Ti.
	 */

	if ((dlb->dlb_vb & FB_POLL) || (dlb->dlb_va != dlb->dlb_vs)) {
		tq_start(dlb->dlb_tq1, dlb->dlb_t1);
		++i;
	} else {
		DB(if (tq_valid(dlb->dlb_tq1))
			TRC(dlb, "chktimers: why is T1 still running");
		);
	}

	if ((dlb->dlb_vb & FB_REJWAIT) || (dlb->dlb_ir_ct != dlb->dlb_n3)) {
		tq_start(dlb->dlb_tq2, dlb->dlb_t2);
		++i;
	} else {
		DB(if (tq_valid(dlb->dlb_tq2))
			TRC(dlb, "chktimers: why is T2 still running");
		);
	}

	if ((dlb->dlb_vb & FB_REMOTE) || (i == 0)) {
		/* if truly idle, do not allow retry exhausted to nuke link */
		if (!(dlb->dlb_vb & FB_REMOTE))
			dlb->dlb_retry = dlb->dlb_n2;
		tq_restart(dlb->dlb_tqi, dlb->dlb_ti);
	}
}

/*
 **************************************************
 * Timeouts
 **************************************************
 */

/*
 * dl_nukem - toss all unprocessed input frames (mbufs)
 */

void
dl_nukem(dlb)
	DLB *dlb;
{
	mblk_t *mp;
	struct mbuf *m, *nextm;
	INTR_LOCK_DECL;

	INTR_LOCK();
	for (m = dlb->dlb_mhead; m; m = nextm) {
		nextm = m->m_nextpkt;
		mp = (mblk_t *)mtod(m, void **)[1];
		if (mp) freeb(mp);
		m_freem(m);
	}
	dlb->dlb_mhead = dlb->dlb_mtail = 0;
	INTR_UNLOCK();
}

/*
 * reset_session - retries failed, initiate a provider reset to recover
 */

void
reset_session(dlb)
	DLB *dlb;
{
	mblk_t *mp;
	void drd_flush();

	/* lose all messages currently buffered */
	dl_nukem(dlb);

	/* flush route in case it was stale, thus causing reset */
	drd_flush(dlb, dlb->dlb_remaddr);

	if (!(mp = dl_gethdr(dlb, 0))) {
		if (!bufcall(DLPIHDR_LEN, BPRI_HI, reset_session, dlb)) {
			/* truly hopeless */
			abort_session(dlb);
			putq(dlb->dlb_rq, dlb->dlb_failmp);
			dlb->dlb_failmp = 0;
		}
	} else {
		dlb->dlb_localreset = 1;
		dl_resetind(dlb, mp, DL_PROVIDER, DL_RESET_LINK_ERROR);
	}
}

/*
 * expired_t1 - T1 expiration; check on poll and retransmissions
 */

void
expired_t1(arg)
	DLB **arg;
{
	DLB *dlb = *arg;

	TRC(dlb, "expired_t1: retry %d", dlb->dlb_retry - 1);

	if (--dlb->dlb_retry == 0) {
		TRC(dlb, "expired_t1: retry exhausted");
		reset_session(dlb);
		return;
	}
	(void)tx_super(dlb, S_CMD|S_POLL);
}

/*
 * expired_t2 - T2 expiration; sent-REJ timeout, or delayed ack timeout
 */

void
expired_t2(arg)
	DLB **arg;
{
	DLB *dlb = *arg;

	TRC(dlb, "expired_t2: retry %d", dlb->dlb_retry - 1);

	dlb->dlb_ir_ct = dlb->dlb_n3;

	if (dlb->dlb_vb & FB_REJWAIT) {
		if (--dlb->dlb_retry == 0) {
			TRC(dlb, "expired_t2: retry exhausted");
			reset_session(dlb);
			return;
		}
		dlb->dlb_vb &= ~FB_REJWAIT;
		(void)tx_super(dlb, S_RSP);
		dlb->dlb_vb |= FB_REJWAIT;
		tq_restart(dlb->dlb_tq2, dlb->dlb_t2);
	} else {
		/* delayed ack exhausted; ack now */
		(void)tx_super(dlb, S_RSP);
	}

	/* if just acked last I-frame, need to become idle */
	chktimers(dlb);
}

/*
 * expired_ti - Ti expiration; line was idle, make sure connection is still up
 */

void
expired_ti(arg)
	DLB **arg;
{
	DLB *dlb = *arg;

	TRC(dlb, "expired_ti: retry %d", dlb->dlb_retry - 1);

	/* if truly idle, do not allow retry exhausted to nuke link */
	if (!(dlb->dlb_vb & FB_REMOTE)) {
		dlb->dlb_retry = dlb->dlb_n2;
	} else if (--dlb->dlb_retry == 0) {
		TRC(dlb, "expired_ti: busy retry exhausted");
		reset_session(dlb);
		return;
	}

	/* solicit response to see if still alive */
	(void)tx_super(dlb, S_CMD|S_POLL);
}
