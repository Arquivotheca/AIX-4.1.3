static char sccsid[] = "@(#)91	1.25.3.18  src/bos/kernext/inet/tcp_input.c, sysxinet, bos41B, 412_41B_sync 12/5/94 11:28:58";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: 
 *		TCP_REASS
 *		tcp_dooptions
 *		tcp_input
 *		tcp_mss
 *		tcp_pulloutofband1383
 *		tcp_reass
 *		tcp_xmit_timer
 *		
 *
 *   ORIGINS: 26,27,85,90
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1988, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      Base:   tcp_input.c     7.21 (Berkeley) 4/8/89
 *      Merged: tcp_input.c     7.25 (Berkeley) 6/30/90
 */

#include <net/net_globals.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/nettrace.h>

#include <net/if.h>
#include <net/route.h>
#include <net/spl.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>

#include <net/net_malloc.h>

LOCK_ASSERTL_DECL

int	tcpcksum = 1;
int	tcprexmtthresh = 3;
int	tcppredack;	/* XXX debugging: times hdr predict ok for acks */
int	tcppreddat;	/* XXX # times header prediction ok for data packets */

/*
 * RFC1323 'no' command option.  1=enable RFC1323 system-wide, 0=disable.
 */
int	rfc1323=0;
int	rfc1323_dflt=0;

struct	tcpiphdr tcp_saveti;
struct	inpcb tcb;
struct 	inpcb_hash_table tcp_pcb_hash_table[INPCB_HASHSZ];
struct	tcpstat	tcpstat;

struct	tcpcb *tcp_newtcpcb();

extern int tcp_keepinit;

/*
 * Insert segment ti into reassembly queue of tcp with
 * control block tp.  Return TH_FIN if reassembly now includes
 * a segment with FIN. The macro form does the common case inline
 * (segment is the next to be received on an established connection,
 * and the queue is empty), avoiding linkage into and removal
 * from the queue and repetition of various conversions.
 * Set DELACK for segments received in order, but ack immediately
 * when segments are out of order (so fast retransmit can work).
 */
#define	TCP_REASS(tp, ti, m, so, flags) { \
	NETSTAT_LOCK_DECL() \
	if ((ti)->ti_seq == (tp)->rcv_nxt && \
	    (tp)->seg_next == (struct tcpiphdr *)(tp) && \
	    (tp)->t_state == TCPS_ESTABLISHED) { \
		(tp)->t_flags |= TF_DELACK; \
		(tp)->rcv_nxt += (ti)->ti_len; \
		flags = (ti)->ti_flags & TH_FIN; \
		NETSTAT_LOCK(&tcpstat.tcps_lock); \
		tcpstat.tcps_rcvpack++; \
		tcpstat.tcps_rcvbyte += (ti)->ti_len; \
		NETSTAT_UNLOCK(&tcpstat.tcps_lock); \
		SOCKBUF_LOCK(&(so)->so_rcv); \
		sbappend(&(so)->so_rcv, (m)); \
		if ((so)->so_rcv.sb_flags & SB_NOTIFY) { \
			SOCKBUF_UNLOCK(&(so)->so_rcv); \
			sorwakeup(so); \
		} else \
			SOCKBUF_UNLOCK(&(so)->so_rcv); \
	} else { \
		(flags) = tcp_reass((tp), (ti), (m)); \
		tp->t_flags |= TF_ACKNOW; \
	} \
}

tcp_reass(tp, ti, m)
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
	register struct mbuf *m;
{
	register struct tcpiphdr *q;
	struct socket *so = tp->t_inpcb->inp_socket;
	register int i;
	int flags;
	NETSTAT_LOCK_DECL()

	/*
	 * Call with ti==0 after become established to
	 * force pre-ESTABLISHED data up to user socket.
	 */
	if (ti == 0)
		goto present;

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
	    q = (struct tcpiphdr *)q->ti_next)
		if (SEQ_GT(q->ti_seq, ti->ti_seq))
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if ((struct tcpiphdr *)q->ti_prev != (struct tcpiphdr *)tp) {
		q = (struct tcpiphdr *)q->ti_prev;
		/* conversion to int (in i) handles seq wraparound */
		i = q->ti_seq + q->ti_len - ti->ti_seq;
		if (i > 0) {
			if (i >= ti->ti_len) {
				NETSTAT_LOCK(&tcpstat.tcps_lock);
				tcpstat.tcps_rcvduppack++;
				tcpstat.tcps_rcvdupbyte += ti->ti_len;
				NETSTAT_UNLOCK(&tcpstat.tcps_lock);
				m_freem(m);
				return (0);
			}
			m_adj(m, i);
			ti->ti_len -= i;
			ti->ti_seq += i;
		}
		q = (struct tcpiphdr *)(q->ti_next);
	}
	NETSTAT_LOCK(&tcpstat.tcps_lock);
	tcpstat.tcps_rcvoopack++;
	tcpstat.tcps_rcvoobyte += ti->ti_len;
	NETSTAT_UNLOCK(&tcpstat.tcps_lock);
	REASS_MBUF(ti) = m;		/* XXX */

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct tcpiphdr *)tp) {
		i = (ti->ti_seq + ti->ti_len) - q->ti_seq;
		if (i <= 0)
			break;
		if (i < q->ti_len) {
			q->ti_seq += i;
			q->ti_len -= i;
			m_adj(REASS_MBUF(q), i);
			break;
		}
		q = (struct tcpiphdr *)q->ti_next;
		m = REASS_MBUF((struct tcpiphdr *)q->ti_prev);
		remque(q->ti_prev);
		m_freem(m);
	}

	/*
	 * Stick new segment in its place.
	 */
	insque(ti, q->ti_prev);

present:
	/*
	 * Present data to user, advancing rcv_nxt through
	 * completed sequence space.
	 */
	if (TCPS_HAVERCVDSYN(tp->t_state) == 0)
		return (0);
	ti = tp->seg_next;
	if (ti == (struct tcpiphdr *)tp || ti->ti_seq != tp->rcv_nxt)
		return (0);
	if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
		return (0);
	do {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		remque(ti);
		m = REASS_MBUF(ti);
		ti = (struct tcpiphdr *)ti->ti_next;
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m);
		else {
			SOCKBUF_LOCK(&so->so_rcv);
			sbappend(&so->so_rcv, m);
			i = so->so_rcv.sb_flags;
			SOCKBUF_UNLOCK(&so->so_rcv);
		}
	} while (ti != (struct tcpiphdr *)tp && ti->ti_seq == tp->rcv_nxt);
	if (i & SB_NOTIFY)
		sorwakeup(so);


	return (flags);
}
/*
 * TCP input routine, follows pages 65-76 of the
 * protocol specification dated September, 1981 very closely.
 */
void
tcp_input(m, iphlen)
	register struct mbuf *m;
	int iphlen;
{
#define DROP_OFF 64
	register struct tcpiphdr *ti;
	register struct inpcb *inp = 0;
	struct mbuf *om = 0;
	int len, tlen, off;
	register struct tcpcb *tp = 0;
	register int tiflags;
	struct socket *so = 0;
	int todrop, acked, ourfinisacked, needoutput = 0;
	short ostate;
	struct in_addr laddr;
	struct tcpiphdr tcp_saveti;
	int dropsocket = 0;
	int iss = 0;
	int index;
	u_long scaled_window, ts_val = 0, ts_ecr = 0, *lp = 0;
	int timestamp_received = 0;
	NETSTAT_LOCK_DECL()
	INHEAD_LOCK_DECL()
	TCPMISC_LOCK_DECL()


	NETSTAT_LOCK(&tcpstat.tcps_lock);
	tcpstat.tcps_rcvtotal++;
	NETSTAT_UNLOCK(&tcpstat.tcps_lock);
	/*
	 * Get IP and TCP header together in first mbuf.
	 * Note: IP leaves IP header in first mbuf.
	 */
	ti = mtod(m, struct tcpiphdr *);
	if (iphlen > sizeof (struct ip))
		ip_stripoptions(m, (struct mbuf *)0);
	if (m->m_len < sizeof (struct tcpiphdr)) {
		if ((m = m_pullup(m, sizeof (struct tcpiphdr))) == 0) {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvshort++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			return;
		}
		ti = mtod(m, struct tcpiphdr *);
	}

	/*
	 * Checksum extended TCP header and data.
	 */
	tlen = ((struct ip *)ti)->ip_len;
	len = sizeof (struct ip) + tlen;
	ti->ti_next = ti->ti_prev = 0;
	ti->ti_x1 = 0;
	ti->ti_len = (u_short)tlen;
	ti->ti_len = htons((u_short)ti->ti_len);
	if (m->m_pkthdr.rcvif != &loif) {
		if (ti->ti_sum = in_cksum(m, len)) {
#if	INETPRINTFS
			if (inetprintfs)
				printf("tcp sum: src 0x%x\n",ntohl(ti->ti_src));
#endif
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvbadsum++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			goto drop;
		}
	} else
		ti->ti_sum = 0;

	/*
	 * Check that TCP offset makes sense,
	 * pull out TCP options and adjust length.
	 */
	off = (ti->ti_xoff & 0xf0) >> 2;
	if (off < sizeof (struct tcphdr) || off > tlen) {
#if	INETPRINTFS
		if (inetprintfs)
			printf("tcp off: src 0x%x off %d\n",
							ntohl(ti->ti_src), off);
#endif
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_rcvbadoff++;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		goto drop;
	}
	tlen -= off;
	ti->ti_len = tlen;
	if (off > sizeof (struct tcphdr)) {
		if (m->m_len < sizeof(struct ip) + off) {
			if ((m = m_pullup(m, sizeof (struct ip) + off)) == 0) {
				NETSTAT_LOCK(&tcpstat.tcps_lock);
				tcpstat.tcps_rcvshort++;
				NETSTAT_UNLOCK(&tcpstat.tcps_lock);
				return;
			}
			ti = mtod(m, struct tcpiphdr *);
		}

		/* 
		 * RFC 1323 - Look at the options to see if the
		 * timestamp opt is there.  If so, extract tsval and tsecr.
		 * This is done without allocating an mbuf as per the RFC's
		 * recommendations...option prediction!
		 */
		if (((off - sizeof(struct tcphdr)) == TCP_TIMESTAMP_OPTLEN) && 
		   !(ti->ti_flags & TH_SYN) && 
		   *(lp = (mtod(m, u_long *) + sizeof(struct tcpiphdr) /
		   sizeof(u_long))) == htonl(TCP_FASTNAME)) {
			ts_val = ntohl(*(lp+1));
			ts_ecr = ntohl(*(lp+2));
			timestamp_received = 1;
		}
		else {
			/*
			 * RFC 1323 - don't remove the opts from the
			 * packet.  This saves us the (potentially large)
			 * bcopy to move the rest of the tcp data in this
			 * mbuf to the front of the mbuf.  I'm not sure
			 * why you cannot just bump m->m_data but the 
			 * bcopy was there for a reason, right?
			 */
			om = m_get(M_DONTWAIT, MT_DATA);
			if (om == 0)
				goto drop;
			om->m_len = off - sizeof (struct tcphdr);
			{ caddr_t op = mtod(m, caddr_t) + sizeof (struct tcpiphdr);
			  bcopy(op, mtod(om, caddr_t), (unsigned)om->m_len);
			}
		}
	}
	tiflags = ti->ti_flags;

	/*
	 * Convert TCP protocol specific fields to host format.
	 */
	NTOHL(ti->ti_seq);
	NTOHL(ti->ti_ack);
	NTOHS(ti->ti_win);
	NTOHS(ti->ti_urp);

	/*
	 * Locate pcb for segment.
	 *
	 * BSD "Reno" caches the previous pcb looked up here. This is
	 * problematic with parallelization and thread safety, due to
	 * the synchronization required, and taking/releasing refcounts,
	 * so we push it down to in_pcblookup (where it belongs?).
	 */
findpcb:
	index = INPCB_TCPHASH(ti->ti_src.s_addr, ti->ti_sport, 
		ti->ti_dst.s_addr, ti->ti_dport);
	INHEAD_READ_LOCK(&tcb);
	inp = in_pcbhashlookup(&(tcp_pcb_hash_table[index]), 
		ti->ti_src, ti->ti_sport, ti->ti_dst, ti->ti_dport, 
		INPLOOKUP_USECACHE|INPLOOKUP_WILDCARD);

	if (inp == 0) {
	
		/*
		 * We must now walk the linear linked list because we 
		 * might match on a listener that's only bound locally
	 	 * (wildcards).
		 */
		inp = in_pcblookup_nolock(&tcb, ti->ti_src, 
			ti->ti_sport, ti->ti_dst, 
			ti->ti_dport, INPLOOKUP_WILDCARD|INPLOOKUP_USECACHE);
	}
	INHEAD_READ_UNLOCK(&tcb);


	/*
	 * If the state is CLOSED (i.e., TCB does not exist) then
	 * all data in the incoming segment is discarded.
	 * If the TCB exists but is in CLOSED state, it is embryonic,
	 * but should either do a listen or a connect soon.
	 */
	if (inp == 0)
		goto dropwithreset;
	so = inp->inp_socket;		/* Follow lock order here! */
	SOCKET_LOCK(so);
	INPCB_LOCK(inp);
	tp = intotcpcb(inp);
	if (tp == 0) {
		INPCB_UNLOCK(inp);
		goto dropwithreset;
	}
	if (tp->t_state == TCPS_CLOSED)
		goto drop;

	/* 
	 * RFC 1323 - Get window size and scale it for use through tcp_input(). 
	 * Save it later in the tcpcb.  Note if no window scale, then
	 * shift of 0 bits is done...
	 */
	if ((tiflags & TH_SYN) == 0)
		scaled_window = ti->ti_win << tp->snd_wnd_scale;
	else
		scaled_window = ti->ti_win;

	if (so->so_options & (SO_DEBUG|SO_ACCEPTCONN)) {
		if (so->so_options & SO_DEBUG) {
			ostate = tp->t_state;
			tcp_saveti = *ti;
		}
		if (so->so_options & SO_ACCEPTCONN) {
			u_short flags = tp->t_flags;

			INPCB_UNLOCK(inp); 	/* Unlock for new create */
			INPCBRC_UNREF(inp);
			tp = 0; inp = 0;
			so = sonewconn(so, 0);
			if (so == 0)
				goto drop;
			/*
			 * This is ugly, but ....
			 *
			 * Mark socket as temporary until we're
			 * committed to keeping it.  The code at
			 * ``drop'' and ``dropwithreset'' check the
			 * flag dropsocket to see if the temporary
			 * socket created here should be discarded.
			 * We mark the socket as discardable until
			 * we're committed to it below in TCPS_LISTEN.
			 */
			dropsocket++;
			inp = (struct inpcb *)so->so_pcb;
			INPCB_LOCK(inp);
			INHEAD_WRITE_LOCK(&tcb);
			INPCBRC_REF(inp); 	/* re-reference (total 2) */
			inp->inp_laddr = ti->ti_dst;
			inp->inp_lport = ti->ti_dport;
			INHEAD_WRITE_UNLOCK(&tcb);
			inp->inp_options = ip_srcroute();
			tp = intotcpcb(inp);
			tp->t_flags |= flags & TF_RFC1323 ? TF_RFC1323 : 0;
			tp->t_state = TCPS_LISTEN;
		}
	}

	/*
	 * Segment received on connection.
	 * Reset idle time and keep-alive timer.
	 */
	tp->t_idle = 0;
	tp->t_timer[TCPT_KEEP] = tcp_keepidle;

	/*
	 * Process options if not in LISTEN state,
	 * else do it below (after getting remote address).
	 */
	if (om && tp->t_state != TCPS_LISTEN) {
		tcp_dooptions(tp, om, ti, &ts_val, &ts_ecr, 
			&timestamp_received);
		om = 0;
	}
	/*
	 * Header prediction: check for the two common cases
	 * of a uni-directional data xfer.  If the packet has
	 * no control flags,  (RFC1323) Either we did NOT receive
         * a timestamp -or- ts_val >= the last valid timestamp received,
	 * is in-sequence, the window didn't
	 * change and we're not retransmitting, it's a
	 * candidate.  If the length is zero and the ack moved
	 * forward, we're the sender side of the xfer.  Just
	 * free the data acked & wake any higher level process
	 * that was blocked waiting for space.  If the length
	 * is non-zero and the ack didn't move, we're the
	 * receiver side.  If we're getting packets in-order
	 * (the reassembly queue is empty), add the data to
	 * the socket buffer and note that we need a delayed ack.
	 */
	if (tp->t_state == TCPS_ESTABLISHED &&
	    (tiflags & (TH_SYN|TH_FIN|TH_RST|TH_URG|TH_ACK)) == TH_ACK &&
	    (!timestamp_received || SEQ_GEQ(ts_val, tp->timestamp_recent)) &&
	    ti->ti_seq == tp->rcv_nxt &&
	    scaled_window && scaled_window == tp->snd_wnd &&
	    tp->snd_nxt == tp->snd_max) {

                /* If this segment is at the left edge of the window advertised
                 * in the last ACK, record the time-stamp.
                 */
                if (timestamp_received && 
			SEQ_LEQ(ti->ti_seq, tp->last_ack_sent)) {
                        tp->timestamp_recent = ts_val;
			tp->timestamp_age = 0;
		}

		if (ti->ti_len == 0) {
			if (SEQ_GT(ti->ti_ack, tp->snd_una) &&
			    SEQ_LEQ(ti->ti_ack, tp->snd_max) &&
			    tp->snd_cwnd >= tp->snd_wnd) {
				/*
				 * this is a pure ack for outstanding data.
				 */
				/*++tcppredack;*/
				if (timestamp_received)
					tcp_xmit_timer(tp, 
						timestamp_clock - ts_ecr + 1);
				else 
					if (tp->t_rtt && 
						SEQ_GT(ti->ti_ack,tp->t_rtseq))
						tcp_xmit_timer(tp, tp->t_rtt);
				acked = ti->ti_ack - tp->snd_una;
				NETSTAT_LOCK(&tcpstat.tcps_lock);
				tcpstat.tcps_rcvackpack++;
				tcpstat.tcps_rcvackbyte += acked;
				NETSTAT_UNLOCK(&tcpstat.tcps_lock);
				tp->snd_una = ti->ti_ack;
				m_freem(m);

				/*
				 * If all outstanding data are acked, stop
				 * retransmit timer, otherwise restart timer
				 * using current (possibly backed-off) value.
				 * If process is waiting for space,
				 * wakeup/selwakeup/signal.  If data
				 * are ready to send, let tcp_output
				 * decide between more output or persist.
				 */
				if (tp->snd_una == tp->snd_max)
					tp->t_timer[TCPT_REXMT] = 0;
				else if (tp->t_timer[TCPT_PERSIST] == 0)
					tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

				SOCKBUF_LOCK(&so->so_snd);
				sbdrop(&so->so_snd, acked);
				needoutput = (so->so_snd.sb_cc != 0);
				if (so->so_snd.sb_flags & SB_NOTIFY) {
					SOCKBUF_UNLOCK(&so->so_snd);
					if (sowriteable(so)) {
						sowwakeup(so);
					}
				} else
					SOCKBUF_UNLOCK(&so->so_snd);
				goto done;
			}
		} else if (ti->ti_ack == tp->snd_una &&
		    tp->seg_next == (struct tcpiphdr *)tp &&
		    (int)ti->ti_len <= sbspace(&so->so_rcv)) { /* XXX lock it */
			/*
			 * this is a pure, in-sequence data packet
			 * with nothing on the reassembly queue and
			 * we have enough buffer space to take it.
			 */
			/*++tcppreddat;*/
			tp->rcv_nxt += ti->ti_len;
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvpack++;
			tcpstat.tcps_rcvbyte += ti->ti_len;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			/*
			 * Drop TCP and IP headers along with the
			 * TCP options, then add data
			 * to socket buffer
			 */
			m->m_data += sizeof(struct tcpiphdr) + off - 
				sizeof(struct tcphdr); 
			m->m_len -= sizeof(struct tcpiphdr) + off - 
				sizeof(struct tcphdr);
			m->m_pkthdr.len -= sizeof(struct tcpiphdr) + off - 
				sizeof(struct tcphdr);
			SOCKBUF_LOCK(&so->so_rcv);
			sbappend(&so->so_rcv, m);
			if (so->so_rcv.sb_flags & SB_NOTIFY) {
				SOCKBUF_UNLOCK(&(so)->so_rcv);
				sorwakeup(so);
			} else
				SOCKBUF_UNLOCK(&so->so_rcv);
			tp->t_flags |= TF_DELACK;
#ifdef _AIX_FULLOSF
			len = so->so_rcv.sb_hiwat - (tp->rcv_adv - tp->rcv_nxt);
			if (len > tp->max_rcvd)
				tp->max_rcvd = len;
#endif
			goto done;
		}
	}

	/*
	 * Drop TCP and IP headers along with the TCP options.
	 */
	m->m_data += sizeof(struct tcpiphdr) + off - sizeof(struct tcphdr);
	m->m_len -= sizeof(struct tcpiphdr) + off - sizeof(struct tcphdr);
	m->m_pkthdr.len -= sizeof(struct tcpiphdr) + off - 
		sizeof(struct tcphdr);

	/*
	 * Calculate amount of space in receive window,
	 * and then do TCP input processing.
	 * Receive window is amount of space in rcv queue,
	 * but not less than advertised window.
	 */
	{ int win;

	SOCKBUF_LOCK(&so->so_rcv);
	win = sbspace(&so->so_rcv);
	SOCKBUF_UNLOCK(&so->so_rcv);
	if (win < 0)
		win = 0;
	tp->rcv_wnd = MAX(win, (int)(tp->rcv_adv - tp->rcv_nxt));
	}

	switch (tp->t_state) {

	/*
	 * If the state is LISTEN then ignore segment if it contains an RST.
	 * If the segment contains an ACK then it is bad and send a RST.
	 * If it does not contain a SYN then it is not interesting; drop it.
	 * Don't bother responding if the destination was a broadcast.
	 * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
	 * tp->iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
	 * Fill in remote peer address fields if not previously specified.
	 * Enter SYN_RECEIVED state, and process any other fields of this
	 * segment in this state.
	 */
	case TCPS_LISTEN: {
		struct mbuf *am;
		register struct sockaddr_in *sin;

		if (tiflags & (TH_RST|TH_FIN))
			goto drop;
		if (tiflags & TH_ACK)
			goto dropwithreset;
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		if (m_broadcast(m))
			goto drop;
		if (ti->ti_dst.s_addr == (u_long) INADDR_BROADCAST)
			goto drop;
		am = m_get(M_DONTWAIT, MT_SONAME);	/* XXX */
		if (am == NULL)
			goto drop;
		am->m_len = sizeof (struct sockaddr_in);
		sin = mtod(am, struct sockaddr_in *);
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = ti->ti_src;
		sin->sin_port = ti->ti_sport;
		INHEAD_WRITE_LOCK(&tcb);
		laddr = inp->inp_laddr;
		if (inp->inp_laddr.s_addr == INADDR_ANY)
			inp->inp_laddr = ti->ti_dst;
		if (in_pcbconnect_nolock(inp, am)) {
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&tcb);
			(void) m_free(am);
			goto drop;
		}
		INHEAD_WRITE_UNLOCK(&tcb);
		(void) m_free(am);
		(void) tcp_template(tp);
		if (om)	{
			tcp_dooptions(tp, om, ti, &ts_val, &ts_ecr,
				&timestamp_received);
			om = 0;
		}
		TCPMISC_LOCK();
		if (iss)
			tp->iss = iss;
		else
			tp->iss = tcp_iss;
		tcp_iss += TCP_ISSINCR/2;
		TCPMISC_UNLOCK();
		tp->irs = ti->ti_seq;
		tcp_sendseqinit(tp);
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
		tp->t_state = TCPS_SYN_RECEIVED;
		tp->t_timer[TCPT_KEEP] = tcp_keepinit;
		dropsocket = 0;		/* committed to socket */
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_accepts++;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		goto trimthenstep6;
		}

	/*
	 * If the state is SYN_SENT:
	 * 	if seg contains an ACK, but not for our SYN, drop the input.
	 * 	if seg contains a RST, then drop the connection.
	 * 	if seg does not contain SYN, then drop it.
	 * Otherwise this is an acceptable SYN segment
	 * 	initialize tp->rcv_nxt and tp->irs
	 * 	if seg contains ack then advance tp->snd_una
	 * 	if SYN has been acked change to ESTABLISHED else SYN_RCVD state
	 * 	arrange for segment to be acked (eventually)
	 * 	continue processing rest of data/controls, beginning with URG
	 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
		    (SEQ_LEQ(ti->ti_ack, tp->iss) ||
		     SEQ_GT(ti->ti_ack, tp->snd_max)))
			goto dropwithreset;
		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK)
				tp = tcp_drop(tp, ECONNREFUSED);
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		if (tiflags & TH_ACK) {
			tp->snd_una = ti->ti_ack;
			if (SEQ_LT(tp->snd_nxt, tp->snd_una))
				tp->snd_nxt = tp->snd_una;
		} else					/* Crossing SYNs */
			tp->snd_nxt = ++tp->snd_una;
		tp->t_timer[TCPT_REXMT] = 0;
		tp->irs = ti->ti_seq;
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
		if (tiflags & TH_ACK && SEQ_GT(tp->snd_una, tp->iss)) {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_connects++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			soisconnected(so);
			tp->t_state = TCPS_ESTABLISHED;

			/*
			 * RFC 1323 - if we did negotiate window scales,
			 * then move the negotiated scales to the actual scales.
			 */
			if ((tp->t_flags&TF_SENT_WS) && 
			    (tp->t_flags&TF_RCVD_WS)) {
				tp->rcv_wnd_scale=tp->req_scale_sent;
				tp->snd_wnd_scale=tp->req_scale_rcvd;
			}
			(void) tcp_reass(tp, (struct tcpiphdr *)0,
				(struct mbuf *)0);
			/*
			 * if we didn't have to retransmit the SYN,
			 * use its rtt as our initial srtt & rtt var.
			 */
			if (timestamp_received)
				tcp_xmit_timer(tp, 
					timestamp_clock - ts_ecr + 1);
			else
				if (tp->t_rtt)
					tcp_xmit_timer(tp, tp->t_rtt);
		} else
			tp->t_state = TCPS_SYN_RECEIVED;

trimthenstep6:
		/*
		 * Advance ti->ti_seq to correspond to first data byte.
		 * If data, trim to stay within window,
		 * dropping FIN if necessary.
		 */
		ti->ti_seq++;
		if (ti->ti_len > tp->rcv_wnd) {
			todrop = ti->ti_len - tp->rcv_wnd;
			m_adj(m, -todrop);
			ti->ti_len = tp->rcv_wnd;
			tiflags &= ~TH_FIN;
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvpackafterwin++;
			tcpstat.tcps_rcvbyteafterwin += todrop;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		}
		tp->snd_wl1 = ti->ti_seq - 1;
		tp->rcv_up = ti->ti_seq;
		goto step6;
	}

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check that at least some bytes of segment are within 
	 * receive window.  If segment begins before rcv_nxt,
	 * drop leading data (and SYN); if nothing left, just ack.
	 */

	/* PAWS: If we have a time-stamp reply on this segment and it's
	 * less than timestamp_recent, drop it.
	 */
	if (timestamp_received && !(tiflags & TH_RST) && 
		tp->timestamp_recent != 0 &&
	    	SEQ_LT(ts_val, tp->timestamp_recent)) {

		/* 
		 * Check to see if timestamp_recent is over 24 days old.
		 */
		if (tp->timestamp_age > TCP_24DAYS_WORTH_OF_SLOWTICKS ) {

			/* Invalidate ts_recent.  If this segment updates
			 * ts_recent, the age will be reset later and ts_recent
			 * will get a valid value.  If it does not, setting
			 * ts_recent to zero will at least satisfy the
			 * requirement that zero be placed in the time-stamp
			 * echo reply when ts_recent isn't valid.  The
			 * age isn't reset until we get a valid ts_recent
			 * because we don't want out-of-order segments to be
			 * dropped when ts_recent is old.
			 */
			tp->timestamp_recent = 0;
		} else {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvduppack++;
			tcpstat.tcps_rcvdupbyte += ti->ti_len;
			tcpstat.tcps_pawsdrop++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			goto dropafterack;
		}
	}

	todrop = tp->rcv_nxt - ti->ti_seq;
	if (todrop > 0) {
		if (tiflags & TH_SYN) {
			tiflags &= ~TH_SYN;
			ti->ti_seq++;
			if (ti->ti_urp > 1)
				ti->ti_urp--;
			else
				tiflags &= ~TH_URG;
			todrop--;
		}
		if (todrop > ti->ti_len ||
		    todrop == ti->ti_len && (tiflags&TH_FIN) == 0) {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvduppack++;
			tcpstat.tcps_rcvdupbyte += ti->ti_len;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			/* 
			 * If segment is just one to the left of the window,
			 * check two special cases:
			 * 1. Don't toss RST in response to 4.2-style keepalive.
			 * 2. If the only thing to drop is a FIN, we can drop
			 *    it, but check the ACK or we will get into FIN
			 *    wars if our FINs crossed (both CLOSING).
			 * In either case, send ACK to resynchronize,
			 * but keep on processing for RST or ACK.
			 */
			if ((tiflags & TH_FIN && todrop == ti->ti_len + 1)
#ifdef TCP_COMPAT_42
			  || (tiflags & TH_RST && ti->ti_seq == tp->rcv_nxt - 1)
#endif
			   ) {
				todrop = ti->ti_len;
				tiflags &= ~TH_FIN;
				tp->t_flags |= TF_ACKNOW;
			} else {
				if (tp->snd_wnd >=  DROP_OFF)
					goto dropafterack;
				else {
					if (tiflags & TH_RST)
						goto drop;
					tp->t_flags |= TF_DELACK;
					goto drop;
				}
			}
		} else {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvpartduppack++;
			tcpstat.tcps_rcvpartdupbyte += todrop;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		}
		m_adj(m, todrop);
		ti->ti_seq += todrop;
		ti->ti_len -= todrop;
		if (ti->ti_urp > todrop)
			ti->ti_urp -= todrop;
		else {
			tiflags &= ~TH_URG;
			ti->ti_urp = 0;
		}
	}

	/*
	 * If new data are received on a connection after the
	 * user processes are gone, then RST the other end.
	 */
	if ((so->so_state & SS_NOFDREF) &&
	    tp->t_state > TCPS_CLOSE_WAIT && ti->ti_len) {
		tp = tcp_close(tp);
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_rcvafterclose++;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		goto dropwithreset;
	}

	/*
	 * If segment ends after window, drop trailing data
	 * (and PUSH and FIN); if nothing left, just ACK.
	 */
	todrop = (ti->ti_seq+ti->ti_len) - (tp->rcv_nxt+tp->rcv_wnd);
	if (todrop > 0) {
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_rcvpackafterwin++;
		if (todrop >= ti->ti_len) {
			tcpstat.tcps_rcvbyteafterwin += ti->ti_len;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			/*
			 * If a new connection request is received
			 * while in TIME_WAIT, drop the old connection
			 * and start over if the sequence numbers
			 * are above the previous ones.
			 */
			if (tiflags & TH_SYN &&
			    tp->t_state == TCPS_TIME_WAIT &&
			    SEQ_GT(ti->ti_seq, tp->rcv_nxt)) {
				iss = tp->rcv_nxt + TCP_ISSINCR;
				tp = tcp_close(tp);
				INPCBRC_UNREF(inp);
				SOCKET_UNLOCK(so);
				so = 0;

				/* RFC 1323 - We must restore the options
				 * mbuf (om) so that they can be reparsed
				 * with the new tcpcb that will be created.
				 * Also need to restore the mbuf ptr/length.
				 */
				om = m_get(M_DONTWAIT, MT_DATA);
				if (om == 0)
					goto drop;
				om->m_len = off - sizeof (struct tcphdr);
				{ 
					caddr_t op = (caddr_t)ti + 
						sizeof(struct tcpiphdr);
					bcopy(op, mtod(om, caddr_t), 
						(unsigned)om->m_len);
				}
				m->m_data = (caddr_t)ti;
				m->m_len += sizeof(struct tcpiphdr) + off - 
					sizeof(struct tcphdr);
				m->m_pkthdr.len += sizeof(struct tcpiphdr) + 
					off - sizeof(struct tcphdr);
				goto findpcb;
			}
			/*
			 * If window is closed can only take segments at
			 * window edge, and have to drop data and PUSH from
			 * incoming segments.  Continue processing, but
			 * remember to ack.  Otherwise, drop segment
			 * and ack.
			 */
			if (tp->rcv_wnd == 0 && ti->ti_seq == tp->rcv_nxt) {
				tp->t_flags |= TF_ACKNOW;
				NETSTAT_LOCK(&tcpstat.tcps_lock);
				tcpstat.tcps_rcvwinprobe++;
				NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			} else
				goto dropafterack;
		} else {
			tcpstat.tcps_rcvbyteafterwin += todrop;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		}
		m_adj(m, -todrop);
		ti->ti_len -= todrop;
		tiflags &= ~(TH_PUSH|TH_FIN);
	}

        /* If this segment is at the left edge of the window advertised
         * in the last ACK.
         */
        if (timestamp_received && SEQ_LEQ(ti->ti_seq, tp->last_ack_sent)) {
                tp->timestamp_recent = ts_val;
		tp->timestamp_age = 0;
	} 

	/*
	 * If the RST bit is set examine the state:
	 *    SYN_RECEIVED STATE:
	 * 	If passive open, return to LISTEN state.
	 * 	If active open, inform user that connection was refused.
	 *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
	 * 	Inform user that connection was reset, and close tcb.
	 *    CLOSING, LAST_ACK, TIME_WAIT STATES
	 * 	Close the tcb.
	 */
	if (tiflags&TH_RST) switch (tp->t_state) {

	case TCPS_SYN_RECEIVED:
		so->so_error = ECONNREFUSED;
		goto close;

	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
		so->so_error = ECONNRESET;
	close:
		tp->t_state = TCPS_CLOSED;
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_drops++;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		tp = tcp_close(tp);
		goto drop;

	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
		tp = tcp_close(tp);
		goto drop;
	}

	/*
	 * If a SYN is in the window, then this is an
	 * error and we send an RST and drop the connection.
	 */
	if (tiflags & TH_SYN) {
		tp = tcp_drop(tp, ECONNRESET);
		goto dropwithreset;
	}

	/*
	 * If the ACK bit is off we drop the segment and return.
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;
	
	/*
	 * Ack processing.
	 */
	switch (tp->t_state) {

	/*
	 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
	 * ESTABLISHED state and continue processing, otherwise
	 * send an RST.
	 */
	case TCPS_SYN_RECEIVED:
		if (SEQ_GT(tp->snd_una, ti->ti_ack) ||
		    SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropwithreset;
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_connects++;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		soisconnected(so);
		tp->t_state = TCPS_ESTABLISHED;

		/*
		 * RFC 1323 - if we did negotiate window scales,
		 * then move the negotiated scales to the actual scales.
		 */
		if ((tp->t_flags&TF_SENT_WS) && (tp->t_flags&TF_RCVD_WS)) {
			tp->rcv_wnd_scale=tp->req_scale_sent;
			tp->snd_wnd_scale=tp->req_scale_rcvd;
		}

		(void) tcp_reass(tp, (struct tcpiphdr *)0, (struct mbuf *)0);
		tp->snd_wl1 = ti->ti_seq - 1;
		/* fall into ... */

	/*
	 * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
	 * ACKs.  If the ack is in the range
	 * 	tp->snd_una < ti->ti_ack <= tp->snd_max
	 * then	advance tp->snd_una to ti->ti_ack and drop
	 * data from the retransmission queue. If this ACK reflects
	 * more up to date window information we update our window information.
	 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:

		if (SEQ_LEQ(ti->ti_ack, tp->snd_una)) {
			if (ti->ti_len == 0 && ti->ti_win == tp->snd_wnd) {
				NETSTAT_LOCK(&tcpstat.tcps_lock);
				tcpstat.tcps_rcvdupack++;
				NETSTAT_UNLOCK(&tcpstat.tcps_lock);
				/*
				 * If we have outstanding data (other than
				 * a window probe), this is a completely
				 * duplicate ack (ie, window info didn't
				 * change), the ack is the biggest we've
				 * seen and we've seen exactly our rexmt
				 * threshhold of them, assume a packet
				 * has been dropped and retransmit it.
				 * Kludge snd_nxt & the congestion
				 * window so we send only this one
				 * packet.
				 *
				 * We know we're losing at the current
				 * window size so do congestion avoidance
				 * (set ssthresh to half the current window
				 * and pull our congestion window back to
				 * the new ssthresh).
				 *
				 * Dup acks mean that packets have left the
				 * network (they're now cached at the receiver) 
				 * so bump cwnd by the amount in the receiver
				 * to keep a constant cwnd packets in the
				 * network.
				 */
				if (tp->t_timer[TCPT_REXMT] == 0 ||
				    ti->ti_ack != tp->snd_una)
					tp->t_dupacks = 0;
				else if (++tp->t_dupacks == tcprexmtthresh) {
					tcp_seq onxt = tp->snd_nxt;
					u_int win =
					    MIN(tp->snd_wnd, tp->snd_cwnd) / 2 /
						tp->t_maxseg;

					if (win < 2)
						win = 2;
					tp->snd_ssthresh = win * tp->t_maxseg;

					tp->t_timer[TCPT_REXMT] = 0;
					tp->t_rtt = 0;
					tp->snd_nxt = ti->ti_ack;
					tp->snd_cwnd = tp->t_maxseg;
					(void) tcp_output(tp);
					tp->snd_cwnd = tp->snd_ssthresh +
					       tp->t_maxseg * tp->t_dupacks;
					if (SEQ_GT(onxt, tp->snd_nxt))
						tp->snd_nxt = onxt;
					goto drop;
				} else if (tp->t_dupacks > tcprexmtthresh) {
					tp->snd_cwnd += tp->t_maxseg;
					(void) tcp_output(tp);
					goto drop;
				}
			} else
				tp->t_dupacks = 0;
			break;
		}
		/*
		 * If the congestion window was inflated to account
		 * for the other side's cached packets, retract it.
		 */
		if (tp->t_dupacks > tcprexmtthresh &&
		    tp->snd_cwnd > tp->snd_ssthresh)
			tp->snd_cwnd = tp->snd_ssthresh;
		tp->t_dupacks = 0;
		if (SEQ_GT(ti->ti_ack, tp->snd_max)) {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvacktoomuch++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
			goto dropafterack;
		}
		acked = ti->ti_ack - tp->snd_una;
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_rcvackpack++;
		tcpstat.tcps_rcvackbyte += acked;
		NETSTAT_UNLOCK(&tcpstat.tcps_lock);

		/*
		 * If transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 * Since we now have an rtt measurement, cancel the
		 * timer backoff (cf., Phil Karn's retransmit alg.).
		 * Recompute the initial retransmit timer.
		 */

		/*
		 * RFC 1323 - REPLACES Karn's alg!!!
		 */
		if (timestamp_received) {
			tcp_xmit_timer(tp, timestamp_clock - ts_ecr + 1);
		}
		else
			if (tp->t_rtt && SEQ_GT(ti->ti_ack, tp->t_rtseq))
				tcp_xmit_timer(tp, tp->t_rtt);

		/*
		 * If all outstanding data is acked, stop retransmit
		 * timer and remember to restart (more output or persist).
		 * If there is more data to be acked, restart retransmit
		 * timer, using current (possibly backed-off) value.
		 */
		if (ti->ti_ack == tp->snd_max) {
			tp->t_timer[TCPT_REXMT] = 0;
			needoutput = 1;
		} else if (tp->t_timer[TCPT_PERSIST] == 0)
			tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;
		/*
		 * When new data is acked, open the congestion window.
		 * If the window gives us less than ssthresh packets
		 * in flight, open exponentially (maxseg per packet).
		 * Otherwise open linearly: maxseg per window
		 * (maxseg^2 / cwnd per packet), plus a constant
		 * fraction of a packet (maxseg/8) to help larger windows
		 * open quickly enough.
		 */
		{
		register u_int cw = tp->snd_cwnd;
		register u_int incr = tp->t_maxseg;

		if (cw > tp->snd_ssthresh)
			incr = incr * incr / cw + incr / 8;
			tp->snd_cwnd = MIN(cw + incr, 
				TCP_MAXWIN<<tp->snd_wnd_scale);
		}
		SOCKBUF_LOCK(&so->so_snd);
		if (acked > so->so_snd.sb_cc) {
			if (tp->snd_wnd > so->so_snd.sb_cc)
				tp->snd_wnd -= so->so_snd.sb_cc;
			else 
				tp->snd_wnd = 0;
			sbdrop(&so->so_snd, (int)so->so_snd.sb_cc);
			ourfinisacked = 1;
		} else {
			sbdrop(&so->so_snd, acked);
			if (tp->snd_wnd > acked)
				tp->snd_wnd -= acked;
			else
				tp->snd_wnd = 0;
			ourfinisacked = 0;
		}
		if (so->so_snd.sb_flags & SB_NOTIFY) {
			SOCKBUF_UNLOCK(&so->so_snd);
			if (sowriteable(so)) {
				sowwakeup(so);
			}
		} else
			SOCKBUF_UNLOCK(&so->so_snd);
		tp->snd_una = ti->ti_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;

		switch (tp->t_state) {

		/*
		 * In FIN_WAIT_1 STATE in addition to the processing
		 * for the ESTABLISHED state if our FIN is now acknowledged
		 * then enter FIN_WAIT_2.
		 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more
				 * data, then closing user can proceed.
				 * Starting the timer is contrary to the
				 * specification, but if we don't get a FIN
				 * we'll hang forever.
				 */
				if (so->so_state & SS_CANTRCVMORE) {
					soisdisconnected(so);
					tp->t_timer[TCPT_2MSL] = tcp_maxidle;
				}
				tp->t_state = TCPS_FIN_WAIT_2;
			}
			break;

		/*
		 * In CLOSING STATE in addition to the processing for
		 * the ESTABLISHED state if the ACK acknowledges our FIN
		 * then enter the TIME-WAIT state, otherwise ignore
		 * the segment.
		 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				tp->t_state = TCPS_TIME_WAIT;
				tcp_canceltimers(tp);
				tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
				soisdisconnected(so);
			}
			break;

		/*
		 * In LAST_ACK, we may still be waiting for data to drain
		 * and/or to be acked, as well as for the ack of our FIN.
		 * If our FIN is now acknowledged, delete the TCB,
		 * enter the closed state and return.
		 */
		case TCPS_LAST_ACK:
			if (ourfinisacked) {
				tp = tcp_close(tp);
				goto drop;
			}
			break;

		/*
		 * In TIME_WAIT state the only thing that should arrive
		 * is a retransmission of the remote FIN.  Acknowledge
		 * it and restart the finack timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		}
	}

step6:
	/*
	 * Update window information.
	 * Don't look at window if no ACK: TAC's send garbage on first SYN.
	 */
	if ((tiflags & TH_ACK) &&
	    (SEQ_LT(tp->snd_wl1, ti->ti_seq) || tp->snd_wl1 == ti->ti_seq &&
	    (SEQ_LT(tp->snd_wl2, ti->ti_ack) ||
	     tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd))) {
		/* keep track of pure window updates */
		if (ti->ti_len == 0 &&
		    tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd) {
			NETSTAT_LOCK(&tcpstat.tcps_lock);
			tcpstat.tcps_rcvwinupd++;
			NETSTAT_UNLOCK(&tcpstat.tcps_lock);
		}
		/*
		 * RFC 1323 - save scaled window.
		 */
		tp->snd_wnd = scaled_window;
		if (tp->snd_wnd > tp->max_sndwnd)
			tp->max_sndwnd = tp->snd_wnd;
		needoutput = 1;
		tp->snd_wl1 = ti->ti_seq;
		tp->snd_wl2 = ti->ti_ack;
	}

	/*
	 * Process segments with URG.
	 */
	if ((tiflags & TH_URG) && ti->ti_urp &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		SOCKBUF_LOCK(&so->so_rcv);
		/*
		 * This is a kludge, but if we receive and accept
		 * random urgent pointers, we'll crash in
		 * soreceive.  It's hard to imagine someone
		 * actually wanting to send this much urgent data.
		 */
		if (ti->ti_urp + so->so_rcv.sb_cc > sb_max) {
			ti->ti_urp = 0;			/* XXX */
			tiflags &= ~TH_URG;		/* XXX */
			goto dodata;			/* XXX */
		}
		/*
		 * If this segment advances the known urgent pointer,
		 * then mark the data stream.  This should not happen
		 * in CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since
		 * a FIN has been received from the remote side. 
		 * In these states we ignore the URG.
		 *
		 * According to RFC961 (Assigned Protocols),
		 * the urgent pointer points to the last octet
		 * of urgent data.  We continue, however,
		 * to consider it to indicate the first octet
		 * of data past the urgent section as the original 
		 * spec states (in one of two places).
		 */
		if (SEQ_GT(ti->ti_seq+ti->ti_urp, tp->rcv_up)) {
			tp->rcv_up = ti->ti_seq + ti->ti_urp;
			so->so_oobmark = so->so_rcv.sb_cc +
			    (tp->rcv_up - tp->rcv_nxt) - 1;
			if (so->so_oobmark == 0)
				so->so_state |= SS_RCVATMARK;
			sohasoutofband(so);
			tp->t_oobflags &= ~(TCPOOB_HAVEDATA | TCPOOB_HADDATA);
		}
		/*
		 * Remove out of band data so doesn't get presented to user.
		 * This can happen independent of advancing the URG pointer,
		 * but if two URG's are pending at once, some out-of-band
		 * data may creep in... ick.
		 */
		if (ti->ti_urp <= ti->ti_len
#ifdef SO_OOBINLINE
		     && (so->so_options & SO_OOBINLINE) == 0
#endif
		   )
			tcp_pulloutofband(so, ti, m);
		SOCKBUF_UNLOCK(&so->so_rcv);
	} else
		/*
		 * If no out of band data is expected,
		 * pull receive urgent pointer along
		 * with the receive window.
		 */
		if (SEQ_GT(tp->rcv_nxt, tp->rcv_up))
			tp->rcv_up = tp->rcv_nxt;
dodata:							/* XXX */

	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data
	 * is presented to the user (this happens in tcp_usrreq.c,
	 * case PRU_RCVD).  If a FIN has already been received on this
	 * connection then we just ignore the text.
	 */
	if ((ti->ti_len || (tiflags&TH_FIN)) &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		TCP_REASS(tp, ti, m, so, tiflags);

#ifdef _AIX_FULLOSF
		/*
		 * Note the amount of data that peer has sent into
		 * our window, in order to estimate the sender's
		 * buffer size.
		 */
		len = so->so_rcv.sb_hiwat - (tp->rcv_adv - tp->rcv_nxt);
		if (len > tp->max_rcvd)
			tp->max_rcvd = len;
#endif

	} else {
		m_freem(m);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know
	 * that the connection is closing.
	 */
	if (tiflags & TH_FIN) {
		if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {
			socantrcvmore(so);
			tp->t_flags |= TF_ACKNOW;
			tp->rcv_nxt++;
		}
		switch (tp->t_state) {

		/*
	 	 * In SYN_RECEIVED and ESTABLISHED STATES
	 	 * enter the CLOSE_WAIT state.
	 	 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			tp->t_state = TCPS_CLOSE_WAIT;
			break;

		/*
		 * If still in FIN_WAIT_1 STATE FIN has not been acked so
		 * enter the CLOSING state.
		 */
		case TCPS_FIN_WAIT_1:
			tp->t_state = TCPS_CLOSING;
			break;

		/*
		 * In FIN_WAIT_2 state enter the TIME_WAIT state,
		 * starting the time-wait timer, turning off the other 
		 * standard timers.
		 */
		case TCPS_FIN_WAIT_2:
			tp->t_state = TCPS_TIME_WAIT;
			tcp_canceltimers(tp);
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			soisdisconnected(so);
			break;

		/*
		 * In TIME_WAIT state restart the 2 MSL time_wait timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;
		}
	}
	if (so->so_options & SO_DEBUG)
		tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0);

	/*
	 * Return any desired output.
	 */
done:
	if (needoutput || (tp->t_flags & TF_ACKNOW))
		(void) tcp_output(tp);
	INPCB_UNLOCK(inp);
	INPCBRC_UNREF(inp);
	SOCKET_UNLOCK(so);


	return;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies
	 * sequence space, where the ACK reflects our state.
	 */
	if (tiflags & TH_RST)
		goto drop;
	m_freem(m);
	tp->t_flags |= TF_ACKNOW;
	(void) tcp_output(tp);
	INPCB_UNLOCK(inp);
	INPCBRC_UNREF(inp);
	SOCKET_UNLOCK(so);


	return;

dropwithreset:
	if (om) {
		(void) m_free(om);
		om = 0;
	}
	/*
	 * Generate a RST, dropping incoming segment.
	 * Make ACK acceptable to originator of segment.
	 * Don't bother to respond if destination was broadcast.
	 */
	if ((tiflags & TH_RST) || m_broadcast(m))
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(tp, ti, m, (tcp_seq)0, ti->ti_ack, TH_RST);
	else {
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(tp, ti, m, ti->ti_seq+ti->ti_len, (tcp_seq)0,
		    TH_RST|TH_ACK);
	}
	if (inp) {
		if (tp) INPCB_UNLOCK(inp);
		INPCBRC_UNREF(inp);
	}

	/* destroy temporarily created socket */
	if (dropsocket)
		(void)tcp_usrreq(so, PRU_ABORT, (struct mbuf *)0, 
			(struct mbuf *)0, (struct mbuf *)0);
	if (so)
		SOCKET_UNLOCK(so);


	return;

drop:
	if (om)
		(void) m_free(om);
	/*
	 * Drop space held by incoming segment and return.
	 */
	if (tp && (tp->t_inpcb->inp_socket->so_options & SO_DEBUG))
		tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0);
	m_freem(m);
	if (inp) {
		if (tp) INPCB_UNLOCK(inp);
		INPCBRC_UNREF(inp);
	}

	/* destroy temporarily created socket */
	if (dropsocket)
		(void)tcp_usrreq(so, PRU_ABORT, (struct mbuf *)0, 
			(struct mbuf *)0, (struct mbuf *)0);

	if (so)
		SOCKET_UNLOCK(so);

	return;
}

/*
 * tcp_adjust_buffers is called when the TIMESTAMP option has been
 * negotiated.  It adjusts the send and receive buffers to be a multiple
 * of the maxseg value which was altered after negotiation of the TIMESTAMP
 * option.  
 *
 * ASSUMES THE SOCKET LOCK IS HELD!!!!
 */
void
tcp_adjust_buffers(struct tcpcb *tp)
{
	int bufsize;
	struct socket *so;
	struct inpcb *inp;

	so = tp->t_inpcb->inp_socket;

	bufsize = so->so_snd.sb_hiwat;
	if (bufsize > tp->t_maxseg) {
		bufsize = MIN(bufsize, sb_max) / tp->t_maxseg * tp->t_maxseg;
		if (bufsize < tp->t_maxseg * 2)
			bufsize = tp->t_maxseg * 2;
		(void) sbreserve(&so->so_snd, bufsize);
	}

	bufsize = so->so_rcv.sb_hiwat;
	if (bufsize > tp->t_maxseg) {
		bufsize = MIN(bufsize, sb_max) / tp->t_maxseg * tp->t_maxseg;
		if (bufsize < tp->t_maxseg * 2)
			bufsize = tp->t_maxseg * 2;
		(void) sbreserve(&so->so_rcv, bufsize);
	}
}

void
tcp_dooptions(tp, om, ti, ts_val, ts_ecr, timestamp_received)
	struct tcpcb *tp;
	struct mbuf *om;
	struct tcpiphdr *ti;
	u_long *ts_val, *ts_ecr;
	int *timestamp_received;
{
	register u_char *cp;
	u_short mss;
	int opt, optlen, cnt;

	cp = mtod(om, u_char *);
	cnt = om->m_len;
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == TCPOPT_EOL)
			break;
		if (opt == TCPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[1];
			if (optlen <= 0)
				break;
		}
		switch (opt) {

		default:
			continue;

		case TCPOPT_MAXSEG:
			if (optlen != 4)
				continue;
			if (!(ti->ti_flags & TH_SYN))
				continue;
			bcopy((char *) cp + 2, (char *) &mss, sizeof(mss));
			NTOHS(mss);
			(void) tcp_mss(tp, mss);	/* sets t_maxseg */
			break;

		case TCPOPT_WINDOWSCALE:
			if (!(tp->t_flags & TF_RFC1323)) 
				continue;
			if (optlen != 3)
				continue;
			if (!(ti->ti_flags & TH_SYN))
				continue;
			tp->req_scale_rcvd = MIN(cp[2], TCP_MAXWINDOWSCALE);
			tp->t_flags |= TF_RCVD_WS;
			break;
		
		case TCPOPT_TIMESTAMP:
			if (!(tp->t_flags & TF_RFC1323))
				continue;
			if (optlen != 10)
				continue;
			bcopy((char *) cp + 2, (char *) ts_val, 
				sizeof(*ts_val));
			bcopy((char *) cp + 6, (char *) ts_ecr, 
				sizeof(*ts_ecr));
			NTOHL(*ts_val);
			NTOHL(*ts_ecr);
			if (ti->ti_flags & TH_SYN) {
				tp->t_flags |= TF_RCVD_TS;
				tp->timestamp_recent = *ts_val;
				tp->timestamp_age = 0;
				tp->t_maxseg -= 12;
				tcp_adjust_buffers(tp);
			}
			*timestamp_received=1;
			break;
		}
	}
	(void) m_free(om);
}

/*
 * Pull out of band byte out of a segment so
 * it doesn't appear in the user's data queue.
 * It is still reflected in the segment length for
 * sequencing purposes.
 */
void
tcp_pulloutofband(so, ti, m)
	struct socket *so;
	struct tcpiphdr *ti;
	register struct mbuf *m;
{
	int cnt = ti->ti_urp - 1;
	
	while (cnt >= 0) {
		if (m->m_len > cnt) {
			char *cp = mtod(m, caddr_t) + cnt;
			struct tcpcb *tp = sototcpcb(so);

			tp->t_iobc = *cp;
			tp->t_oobflags |= TCPOOB_HAVEDATA;
			bcopy(cp+1, cp, (unsigned)(m->m_len - cnt - 1));
			m->m_len--;
			return;
		}
		cnt -= m->m_len;
		m = m->m_next;
		if (m == 0)
			break;
	}
	panic("tcp_pulloutofband");
}

/*
 * Collect new round-trip time estimate
 * and update averages and current timeout.
 * Note, if RFC1323 is in effect, then rtt is computed using the timestamp
 * option, else it's computed the traditional way.
 */
void
tcp_xmit_timer(tp, rtt)
	register struct	tcpcb *tp;
	short	rtt;
{
	register short delta;
	NETSTAT_LOCK_DECL()

	NETSTAT_LOCK(&tcpstat.tcps_lock);
	tcpstat.tcps_rttupdated++;
	NETSTAT_UNLOCK(&tcpstat.tcps_lock);
	if (tp->t_srtt != 0) {
		/*
		 * srtt is stored as fixed point with 3 bits after the
		 * binary point (i.e., scaled by 8).  The following magic
		 * is equivalent to the smoothing algorithm in rfc793 with
		 * an alpha of .875 (srtt = rtt/8 + srtt*7/8 in fixed
		 * point).  Adjust t_rtt to origin 0.
		 */
		delta = rtt - 1 - (tp->t_srtt >> TCP_RTT_SHIFT);
		if ((tp->t_srtt += delta) <= 0)
			tp->t_srtt = 1;
		/*
		 * We accumulate a smoothed rtt variance (actually, a
		 * smoothed mean difference), then set the retransmit
		 * timer to smoothed rtt + 4 times the smoothed variance.
		 * rttvar is stored as fixed point with 2 bits after the
		 * binary point (scaled by 4).  The following is
		 * equivalent to rfc793 smoothing with an alpha of .75
		 * (rttvar = rttvar*3/4 + |delta| / 4).  This replaces
		 * rfc793's wired-in beta.
		 */
		if (delta < 0)
			delta = -delta;
		delta -= (tp->t_rttvar >> TCP_RTTVAR_SHIFT);
		if ((tp->t_rttvar += delta) <= 0)
			tp->t_rttvar = 1;
	} else {
		/* 
		 * No rtt measurement yet - use the unsmoothed rtt.
		 * Set the variance to half the rtt (so our first
		 * retransmit happens at 2*rtt)
		 */
		tp->t_srtt = rtt << TCP_RTT_SHIFT;
		tp->t_rttvar = rtt << (TCP_RTTVAR_SHIFT - 1);
	}
	tp->t_rtt = 0;
	tp->t_rxtshift = 0;

	/*
	 * the retransmit should happen at rtt + 4 * rttvar.
	 * Because of the way we do the smoothing, srtt and rttvar
	 * will each average +1/2 tick of bias.  When we compute
	 * the retransmit timer, we want 1/2 tick of rounding and
	 * 1 extra tick because of +-1/2 tick uncertainty in the
	 * firing of the timer.  The bias will give us exactly the
	 * 1.5 tick we need.  But, because the bias is
	 * statistical, we have to test that we don't drop below
	 * the minimum feasible timer (which is 2 ticks).
	 */
/* If user has set retransmit times, give them exactly what they want. */
	if ((tcp_rtolow != RTO_DFLT_LOW) || (tcp_rtohigh != RTO_DFLT_HIGH)) {
		TCPT_RANGESET(tp->t_rxtcur, TCP_REXMTVAL(tp),
			tcp_rtolow, tcp_rtohigh);
	} else {	
		TCPT_RANGESET(tp->t_rxtcur, TCP_REXMTVAL(tp),
		    tp->t_rttmin, TCPTV_REXMTMAX);
	}
	
	/*
	 * We received an ack for a packet that wasn't retransmitted;
	 * it is probably safe to discard any error indications we've
	 * received recently.  This isn't quite right, but close enough
	 * for now (a route might have failed after we sent a segment,
	 * and the return path might not be symmetrical).
	 */
	tp->t_softerror = 0;
}

/*
 * Determine a reasonable value for maxseg size.
 * If the route is known, check route for mtu.
 * If none, use an mss that can be handled on the outgoing
 * interface without forcing IP to fragment; if bigger than
 * an mbuf cluster (MCLBYTES), round down to nearest multiple of MCLBYTES
 * to utilize large mbufs.  If no route is found, route has no mtu,
 * or the destination isn't local, use a default, hopefully conservative
 * size (usually 512 or the default IP max size, but no more than the mtu
 * of the interface), as we can't discover anything about intervening
 * gateways or networks.  We also initialize the congestion/slow start
 * window to be a single segment if the destination isn't local.
 * While looking at the routing entry, we also initialize other path-dependent
 * parameters from pre-set or cached values in the routing entry.
 */
tcp_mss(
	register struct tcpcb *tp,
	u_short offer)
{
	struct route *ro;
	register struct rtentry *rt;
	struct ifnet *ifp;
	register int rtt, mss;
	u_long bufsize;
	struct inpcb *inp;
	struct socket *so;
	ROUTE_LOCK_DECL()
	extern int tcp_mssdflt, tcp_rttdflt;
	int datasize;

	inp = tp->t_inpcb;
	ro = &inp->inp_route;

	ROUTE_WRITE_LOCK();
	if ((rt = ro->ro_rt) == (struct rtentry *)0) {
		/* No route yet, so try to acquire one */
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			ro->ro_dst.sa_family = AF_INET;
			ro->ro_dst.sa_len = sizeof(ro->ro_dst);
			((struct sockaddr_in *) &ro->ro_dst)->sin_addr =
				inp->inp_faddr;
			rtalloc_nolock(ro);
		}
		if ((rt = ro->ro_rt) == (struct rtentry *)0) {
			ROUTE_WRITE_UNLOCK();
			return (tcp_mssdflt);
		}
	}
	ifp = rt->rt_ifp;
	ROUTE_WRITE_UNLOCK();
	so = inp->inp_socket;

#ifdef RTV_MTU	/* if route characteristics exist ... */
	/*
	 * While we're here, check if there's an initial rtt
	 * or rttvar.  Convert from the route-table units
	 * to scaled multiples of the slow timeout timer.
	 */
	if (tp->t_srtt == 0 && (rtt = rt->rt_rmx.rmx_rtt)) {
		if (rt->rt_rmx.rmx_locks & RTV_RTT)
			tp->t_rttmin = rtt / (RTM_RTTUNIT / PR_SLOWHZ);
		tp->t_srtt = rtt / (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
		if (rt->rt_rmx.rmx_rttvar)
			tp->t_rttvar = rt->rt_rmx.rmx_rttvar /
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTTVAR_SCALE));
		else
			/* default variation is +- 1 rtt */
			tp->t_rttvar =
			    tp->t_srtt * TCP_RTTVAR_SCALE / TCP_RTT_SCALE;
/* If user has set retransmit times, give them exactly what they want. */
		if ((tcp_rtolow != RTO_DFLT_LOW) || (tcp_rtohigh != RTO_DFLT_HIGH)) {
			TCPT_RANGESET(tp->t_rxtcur,
				((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
				tcp_rtolow, tcp_rtohigh);
		} else {
			TCPT_RANGESET(tp->t_rxtcur,
			    ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
			    tp->t_rttmin, TCPTV_REXMTMAX);
		}
	}
	/*
	 * if there's an mtu associated with the route, use it
	 */
	if (rt->rt_rmx.rmx_mtu)
		mss = rt->rt_rmx.rmx_mtu - sizeof(struct tcpiphdr);
	else
#endif /* RTV_MTU */
	{
		mss = ifp->if_mtu - sizeof(struct tcpiphdr);
#if	(MCLBYTES & (MCLBYTES - 1)) == 0
		if (mss > MCLBYTES)
			mss &= ~(MCLBYTES-1);
#else
		if (mss > MCLBYTES)
			mss = mss / MCLBYTES * MCLBYTES;
#endif
		if (!in_localaddr(inp->inp_faddr))
			mss = MIN(mss, tcp_mssdflt);
		else {

			/*
			 * If RFC 1323 is to be attempted, 
			 * then bump the MSS by 12, so
			 * in the event that it is 
			 * negotiated successfully, 
			 * we will have user data segments 
			 * that are on 4K boundaries 
			 * (because user data sement size is MSS-12).  
			 * If RFC1323 does NOT get negotiated, then 
			 * we might end up with an MSS that 
			 * is NOT page aligned...bummer...
			 * Note: Don't bump mss if it will 
			 * cause ip framentation...
			 */
			if ((tp->t_flags & TF_RFC1323) && ((mss+12) 
				<= (ifp->if_mtu - sizeof(struct tcpiphdr))))
				mss +=12;
		}
	}


	/*
	 * The current mss, t_maxseg, is initialized to the default value.
	 * If we compute a smaller value, reduce the current mss.
	 * If we compute a larger value, return it for use in sending
	 * a max seg size option, but don't store it for use
	 * unless we received an offer at least that large from peer.
	 * However, do not accept offers under 32 bytes.
	 */
	if (offer)
		mss = MIN(mss, offer);
	mss = MAX(mss, 32);		/* sanity */
	if (mss < tp->t_maxseg || offer != 0) {
		/*
		 * If there's a pipesize, change the socket buffer
		 * to that size.  Make the socket buffers an integral
		 * number of mss units; if the mss is larger than
		 * the socket buffer, decrease the mss.
		 */
		LOCK_ASSERT("tcp_mss so", SOCKET_ISLOCKED(so));
		SOCKBUF_LOCK(&so->so_snd);

		/* datasize is MSS less and TCP options to be used. */
		if (tp->t_flags & TF_RCVD_TS)
			datasize = mss-12;
		else
			datasize = mss;	
#ifdef RTV_SPIPE
		if ((bufsize = rt->rt_rmx.rmx_sendpipe) == 0)
#endif
			bufsize = so->so_snd.sb_hiwat;
		if (bufsize < datasize) {
			if (tp->t_flags & TF_RCVD_TS)
				mss = bufsize + 12;
			else
				mss = bufsize;
			datasize = bufsize;
		} else {
			bufsize = MIN(bufsize, sb_max) / datasize * datasize;
			if (bufsize < datasize * 2)
				bufsize = datasize * 2;
			(void) sbreserve(&so->so_snd, bufsize);
		}
		SOCKBUF_UNLOCK(&so->so_snd);
		tp->t_maxseg = datasize;

		SOCKBUF_LOCK(&so->so_rcv);
#ifdef RTV_RPIPE
		if ((bufsize = rt->rt_rmx.rmx_recvpipe) == 0)
#endif
			bufsize = so->so_rcv.sb_hiwat;
		if (bufsize > datasize) {
			bufsize = MIN(bufsize, sb_max) / datasize * datasize;
			if (bufsize < datasize * 2)
				bufsize =  datasize * 2;
			(void) sbreserve(&so->so_rcv, bufsize);
		}
		SOCKBUF_UNLOCK(&so->so_rcv);
	}
	if (in_localaddr(inp->inp_faddr))
		tp->snd_cwnd = MIN(2 * mss, 60*1024);
	else
		tp->snd_cwnd = mss;

#ifdef RTV_SSTHRESH
	if (rt->rt_rmx.rmx_ssthresh) {
		/*
		 * There's some sort of gateway or interface
		 * buffer limit on the path.  Use this to set
		 * the slow start threshhold, but set the
		 * threshold to no less than 2*mss.
		 */
		tp->snd_ssthresh = MAX(2 * mss, rt->rt_rmx.rmx_ssthresh);
	}
#endif /* RTV_MTU */
	return (mss);
}
