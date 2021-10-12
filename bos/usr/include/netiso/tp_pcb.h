/* @(#)61	1.5  src/bos/usr/include/netiso/tp_pcb.h, sockinc, bos411, 9428A410j 3/5/94 12:41:43 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* 
 * ARGO TP
 *
 * $Header: tp_pcb.h,v 5.2 88/11/18 17:09:32 nhall Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_pcb.h,v $
 *	(#)tp_pcb.h	7.7 (Berkeley) 6/28/90 *
 *
 * 
 * This file defines the transport protocol control block (tpcb).
 * and a bunch of #define values that are used in the tpcb.
 */

#ifndef  __TP_PCB__
#define  __TP_PCB__

#include <netiso/tp_param.h>
#include <netiso/tp_timer.h>
#include <netiso/tp_user.h>
#ifndef sblock
#include <sys/socketvar.h>
#endif /* sblock */

/* NOTE: the code depends on REF_CLOSED > REF_OPEN > the rest, and
 * on REF_FREE being zero
 *
 * Possible improvement:
 * think about merging the tp_ref w/ the tpcb and doing a search
 * through the tpcb list, from tpb. This would slow down lookup
 * during data transfer
 * It would be a little nicer also to have something based on the
 * clock (like top n bits of the reference is part of the clock, to
 * minimize the likelihood  of reuse after a crash)
 * also, need to keep the timer servicing part to a minimum (although
 * the cost of this is probably independent of whether the timers are
 * in the pcb or in an array..
 * Last, would have to make the number of timers a function of the amount of
 * mbufs available, plus some for the frozen references.
 *
 * Possible improvement:
 * Might not need the ref_state stuff either...
 * REF_FREE could correspond to tp_state == CLOSED or nonexistend tpcb,
 * REF_OPEN to tp_state anywhere from AK_WAIT or CR_SENT to CLOSING
 * REF_OPENING could correspond to LISTENING, because that's the
 * way it's used, not because the correspondence is exact.
 * REF_CLOSED could correspond to REFWAIT
 */
#define REF_FROZEN 3	/* has ref timer only */
#define REF_OPEN 2		/* has timers, possibly active */
#define REF_OPENING 1	/* in use (has a pcb) but no timers */
#define REF_FREE 0		/* free to reallocate */

#define N_CTIMERS 		4
#define N_ETIMERS 		2

struct tp_ref {
	u_char	 			tpr_state; /* values REF_FROZEN, etc. above */
	struct Ccallout 	tpr_callout[N_CTIMERS]; /* C timers */
	struct Ecallout		tpr_calltodo;			/* list of active E timers */
	struct tp_pcb 		*tpr_pcb;	/* back ptr to PCB */
};

struct tp_param {
	/* PER system stuff (one static structure instead of a bunch of names) */
	unsigned 	tpp_configed:1;			/* Has TP been initialized? */
};


/*
 * retransmission control and performance measurement 
 */
struct tp_rtc {
	struct tp_rtc	*tprt_next; /* ptr to next rtc structure in the list */
	SeqNum 			tprt_seq;	/* seq # of this TPDU */
	int				tprt_eot;	/* Will this TPDU have the eot bit set? */
	int				tprt_octets;/* # octets in this TPDU */
	struct mbuf		*tprt_data; /* ptr to the octets of data */
};

struct nl_protosw {
	int		nlp_afamily;			/* address family */
	int		(*nlp_putnetaddr)();	/* puts addresses in nl pcb */
	int		(*nlp_getnetaddr)();	/* gets addresses from nl pcb */
	int		(*nlp_cmpnetaddr)();	/* compares address in pcb with sockaddr */
	int		(*nlp_putsufx)();		/* puts transport suffixes in nl pcb */
	int		(*nlp_getsufx)();		/* gets transport suffixes from nl pcb */
	int		(*nlp_recycle_suffix)();/* clears suffix from nl pcb */
	int		(*nlp_mtu)();			/* figures out mtu based on nl used */
	int		(*nlp_pcbbind)();		/* bind to pcb for net level */
	int		(*nlp_pcbconn)();		/* connect for net level */
	int		(*nlp_pcbdisc)();		/* disconnect net level */
	int		(*nlp_pcbdetach)();		/* detach net level pcb */
	int		(*nlp_pcballoc)();		/* allocate a net level pcb */
	int		(*nlp_output)();		/* prepare a packet to give to nl */
	int		(*nlp_dgoutput)();		/* prepare a packet to give to nl */
	int		(*nlp_ctloutput)();		/* hook for network set/get options */
	caddr_t	nlp_pcblist;			/* list of xx_pcb's for connections */
};


struct tp_pcb {
	struct tp_pcb		*tp_next;
	struct tp_pcb		*tp_prev;
	struct tp_pcb		*tp_nextlisten; /* chain all listeners */
	u_short 			tp_state;		/* state of fsm */
	short 				tp_retrans;		/* # times can still retrans */
	struct tp_ref 		*tp_refp;		/* rest of pcb	*/
	caddr_t				tp_npcb;		/* to lower layer pcb */
	struct nl_protosw	*tp_nlproto;	/* lower-layer dependent routines */
	struct socket 		*tp_sock;		/* back ptr */


	RefNum				tp_lref;	 	/* local reference */
	RefNum 				tp_fref;		/* foreign reference */

	u_int				tp_seqmask;		/* mask for seq space */
	u_int				tp_seqbit;		/* bit for seq number wraparound */
	u_int				tp_seqhalf;		/* half the seq space */

	/* credit & sequencing info for SENDING */
	u_short 			tp_fcredit;		/* current remote credit in # packets */

	u_short				tp_cong_win;	/* congestion window : set to 1 on
										 * source quench
										 * Minimizes the amount of retrans-
										 * missions (independently of the
										 * retrans strategy).  Increased
										 * by one for each good ack received.
										 * Minimizes the amount sent in a
										 * regular tp_send() also.
										 */
	u_int   tp_ackrcvd; /* ACKs received since the send window was updated */
	SeqNum              tp_last_retrans;
	SeqNum              tp_retrans_hiwat;
	SeqNum				tp_snduna;		/* seq # of lowest unacked DT */
	struct tp_rtc		*tp_snduna_rtc;	/* lowest unacked stuff sent so far */
	SeqNum				tp_sndhiwat;	/* highest seq # sent so far */

	struct tp_rtc		*tp_sndhiwat_rtc;	/* last stuff sent so far */
	int					tp_Nwindow;		/* for perf. measurement */
	struct mbuf			*tp_ucddata;	/* user connect/disconnect data */

	/* credit & sequencing info for RECEIVING */
	SeqNum	 			tp_sent_lcdt;	/* cdt according to last ack sent */
	SeqNum	 			tp_sent_uwe;	/* uwe according to last ack sent */
	SeqNum	 			tp_sent_rcvnxt;	/* rcvnxt according to last ack sent 
										 * needed for perf measurements only
										 */
	u_short				tp_lcredit;		/* current local credit in # packets */
	SeqNum				tp_rcvnxt;		/* next DT seq # expect to recv */
	struct tp_rtc		*tp_rcvnxt_rtc;	/* unacked stuff recvd out of order */

	/* receiver congestion state stuff ...  */
	u_int               tp_win_recv;

	/* receive window as a scaled int (8 bit fraction part) */

	struct cong_sample {
		ushort  cs_size; 				/* current window size */
		ushort  cs_received;   			/* PDUs received in this sample */
		ushort  cs_ce_set;    /* PDUs received in this sample with CE bit set */
	} tp_cong_sample;


	/* parameters per-connection controllable by user */
	struct tp_conn_param _tp_param; 

#define	tp_Nretrans _tp_param.p_Nretrans
#define	tp_dr_ticks _tp_param.p_dr_ticks
#define	tp_cc_ticks _tp_param.p_cc_ticks
#define	tp_dt_ticks _tp_param.p_dt_ticks
#define	tp_xpd_ticks _tp_param.p_x_ticks
#define	tp_cr_ticks _tp_param.p_cr_ticks
#define	tp_keepalive_ticks _tp_param.p_keepalive_ticks
#define	tp_sendack_ticks _tp_param.p_sendack_ticks
#define	tp_refer_ticks _tp_param.p_ref_ticks
#define	tp_inact_ticks _tp_param.p_inact_ticks
#define	tp_xtd_format _tp_param.p_xtd_format
#define	tp_xpd_service _tp_param.p_xpd_service
#define	tp_ack_strat _tp_param.p_ack_strat
#define	tp_rx_strat _tp_param.p_rx_strat
#define	tp_use_checksum _tp_param.p_use_checksum
#define	tp_use_efc _tp_param.p_use_efc
#define	tp_use_nxpd _tp_param.p_use_nxpd
#define	tp_use_rcc _tp_param.p_use_rcc
#define	tp_tpdusize _tp_param.p_tpdusize
#define	tp_class _tp_param.p_class
#define	tp_winsize _tp_param.p_winsize
#define	tp_no_disc_indications _tp_param.p_no_disc_indications
#define	tp_dont_change_params _tp_param.p_dont_change_params
#define	tp_netservice _tp_param.p_netservice
#define	tp_version _tp_param.p_version

	int tp_l_tpdusize;
		/* whereas tp_tpdusize is log2(the negotiated max size)
		 * l_tpdusize is the size we'll use when sending, in # chars
		 */

	struct timeval	tp_rtv;					/* max round-trip time variance */
	struct timeval	tp_rtt; 					/* smoothed round-trip time */
	struct timeval 	tp_rttemit[ TP_RTT_NUM + 1 ]; 
					/* times that the last TP_RTT_NUM DT_TPDUs were emitted */
	unsigned 
		tp_sendfcc:1,			/* shall next ack include FCC parameter? */
		tp_trace:1,				/* is this pcb being traced? (not used yet) */
		tp_perf_on:1,			/* 0/1 -> performance measuring on  */
		tp_reneged:1,			/* have we reneged on cdt since last ack? */
		tp_decbit:3,			/* dec bit was set, we're in reneg mode  */
		tp_cebit_off:1,			/* the real DEC bit algorithms not in use */
		tp_flags:8,				/* values: */
#define TPF_CONN_DATA_OUT	TPFLAG_CONN_DATA_OUT
#define TPF_CONN_DATA_IN	TPFLAG_CONN_DATA_IN
#define TPF_DISC_DATA_IN	TPFLAG_DISC_DATA_IN
#define TPF_DISC_DATA_OUT	TPFLAG_DISC_DATA_OUT
#define TPF_XPD_PRESENT 	TPFLAG_XPD_PRESENT 
#define TPF_NLQOS_PDN	 	TPFLAG_NLQOS_PDN
#define TPF_PEER_ON_SAMENET	TPFLAG_PEER_ON_SAMENET

#define PEER_IS_LOCAL(t) \
			(((t)->tp_flags & TPF_PEER_ON_SAME_NET)==TPF_PEER_ON_SAME_NET)
#define USES_PDN(t)	\
			(((t)->tp_flags & TPF_NLQOS_PDN)==TPF_NLQOS_PDN)

		tp_unused:16;


#ifdef TP_PERF_MEAS
	/* performance stats - see tp_stat.h */
	struct tp_pmeas		*tp_p_meas;
	struct mbuf			*tp_p_mbuf;
#endif /* TP_PERF_MEAS */
	/* addressing */
	u_short				tp_domain;		/* domain (INET, ISO) */
	/* for compatibility with the *old* way and with INET, be sure that
	 * that lsuffix and fsuffix are aligned to a short addr.
	 * having them follow the u_short *suffixlen should suffice (choke)
	 */
	u_short				tp_fsuffixlen;	/* foreign suffix */
	char				tp_fsuffix[MAX_TSAP_SEL_LEN];
	u_short				tp_lsuffixlen;	/* local suffix */
	char				tp_lsuffix[MAX_TSAP_SEL_LEN];
#define SHORT_LSUFXP(tpcb) ((short *)((tpcb)->tp_lsuffix))
#define SHORT_FSUFXP(tpcb) ((short *)((tpcb)->tp_fsuffix))

	u_char 				tp_vers;		/* protocol version */
	u_char 				tp_peer_acktime; /* used to compute DT retrans time */

	struct sockbuf		tp_Xsnd;		/* for expedited data */
/*	struct sockbuf		tp_Xrcv;		/* for expedited data */
#define tp_Xrcv tp_sock->so_rcv
	SeqNum				tp_Xsndnxt;	/* next XPD seq # to send */
	SeqNum				tp_Xuna;		/* seq # of unacked XPD */
	SeqNum				tp_Xrcvnxt;	/* next XPD seq # expect to recv */

	/* AK subsequencing */
	u_short				tp_s_subseq;	/* next subseq to send */
	u_short				tp_r_subseq;	/* highest recv subseq */

	struct mbuf			*tp_pcb_mbuf;	/* back pointer to mbuf containing the protocol block */
};

u_int	tp_start_win;

#define ROUND(scaled_int) (((scaled_int) >> 8) + (((scaled_int) & 0x80) ? 1:0))

/* to round off a scaled int with an 8 bit fraction part */

#define CONG_INIT_SAMPLE(pcb) \
	pcb->tp_cong_sample.cs_received = \
    pcb->tp_cong_sample.cs_ce_set = 0; \
    pcb->tp_cong_sample.cs_size = MAX(pcb->tp_lcredit, 1) << 1;

#define CONG_UPDATE_SAMPLE(pcb, ce_bit) \
    pcb->tp_cong_sample.cs_received++; \
    if (ce_bit) { \
        pcb->tp_cong_sample.cs_ce_set++; \
    } \
    if (pcb->tp_cong_sample.cs_size <= pcb->tp_cong_sample.cs_received) { \
        if ((pcb->tp_cong_sample.cs_ce_set << 1) >=  \
                    pcb->tp_cong_sample.cs_size ) { \
            pcb->tp_win_recv -= pcb->tp_win_recv >> 3; /* multiply by .875 */ \
            pcb->tp_win_recv = MAX(1 << 8, pcb->tp_win_recv); \
        } \
        else { \
            pcb->tp_win_recv += (1 << 8); /* add one to the scaled int */ \
        } \
        pcb->tp_lcredit = ROUND(pcb->tp_win_recv); \
        CONG_INIT_SAMPLE(pcb); \
    }

#define CONG_ACK(pcb, seq) \
{ int   newacks = SEQ_SUB(pcb, seq, pcb->tp_snduna); \
	if (newacks > 0) { \
		pcb->tp_ackrcvd += newacks; \
		if (pcb->tp_ackrcvd >= MIN(pcb->tp_fcredit, pcb->tp_cong_win)) { \
			++pcb->tp_cong_win; \
			pcb->tp_ackrcvd = 0; \
		} \
	} \
}

#ifdef _KERNEL
/*
extern struct timeval 	time;
*/					/* already defined in time.h */
extern struct tp_ref 	tp_ref[];
extern struct tp_param	tp_param;
extern struct nl_protosw  nl_protosw[];
extern struct tp_pcb	*tp_listeners;
extern struct tp_pcb	*tp_intercepts;
#endif

#define	sototpcb(so) 	((struct tp_pcb *)(so->so_tpcb))
#define	sototpref(so)	((struct tp_ref *)((so)->so_tpcb->tp_ref))
#define	tpcbtoso(tp)	((struct socket *)((tp)->tp_sock))
#define	tpcbtoref(tp)	((struct tp_ref *)((tp)->tp_ref))

#endif  /* __TP_PCB__ */
