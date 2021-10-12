/* @(#)47	1.5  src/bos/usr/include/netiso/cons_pcb.h, sockinc, bos411, 9428A410j 3/5/94 12:41:00 */

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
/* $Header: cons_pcb.h,v 4.2 88/06/29 14:59:08 hagens Exp $ */
/* $Source: /usr/argo/sys/netiso/RCS/cons_pcb.h,v $ */
/*	(#)cons_pcb.h	7.3 (Berkeley) 8/29/89 */

/*
 * protocol control block for the connection oriented network service
 */

/*
 * legit port #s for cons "transport" are 0..23 for su users only, and
 * 1024..1099 for public users
 */
#define X25_SBSIZE 	512
#define	X25_PORT_RESERVED 24
#define	X25_PORT_USERMAX 1099
#define X25_FACIL_LEN_MAX  109
#define X25_PARTIAL_PKT_LEN_MAX (MLEN - sizeof(struct cons_pcb))

#ifndef ARGO_DEBUG
#define X25_TTL 600 /* 5 min */
#else ARGO_DEBUG
#define X25_TTL 120 /* change to 1 min - 120 */
#endif /* ARGO_DEBUG */

struct cons_pcb {
	struct isopcb 	_co_isopcb;
#define co_next	_co_isopcb.isop_next
/* prev used for netstat only */
#define co_prev	_co_isopcb.isop_prev
#define co_head	_co_isopcb.isop_head
#define co_laddr _co_isopcb.isop_laddr
#define co_faddr _co_isopcb.isop_faddr

#define co_sladdr _co_isopcb.isop_sladdr
#define co_sfaddr _co_isopcb.isop_sfaddr

#define co_lport(copcb) \
		TSEL((copcb)->_co_isopcb.isop_laddr)
#define co_fport(copcb) \
		TSEL((copcb)->_co_isopcb.isop_faddr)
#define co_SHORT_lport(copcb) \
		TSEL_SHORT((copcb)->_co_isopcb.isop_laddr)
#define co_SHORT_fport(copcb) \
		TSEL_SHORT((copcb)->_co_isopcb.isop_faddr)

#define co_lportlen(copcb)	\
		 ((copcb)->_co_isopcb.isop_laddr->siso_tlen)
#define co_fportlen(copcb)	\
		 ((copcb)->_co_isopcb.isop_faddr->siso_tlen)

#define co_laddr_x121(copcb) \
			SOCKADDR_TO_X121((copcb)->co_laddr)
#define co_faddr_x121(copcb) \
			SOCKADDR_TO_X121((copcb)->co_faddr)

#define co_route _co_isopcb.isop_route
#define co_socket _co_isopcb.isop_socket
#define	co_chanmask _co_isopcb.isop_chanmask
#define	co_negchanmask _co_isopcb.isop_negchanmask
#define	co_x25crud _co_isopcb.isop_x25crud
#define	co_x25crud_len _co_isopcb.isop_x25crud_len
	u_short 		co_state; 
	u_char 			co_flags; 
	u_short			co_ttl; /* time to live timer */
	u_short			co_init_ttl; /* initial value of ttl  */
	int 			co_channel; /* logical channel */
					    /* channel is divided into
					     * unit in first half word
					     * and session_id in 2nd half
					     */
	struct ifnet *co_ifp; /* interface */
	struct protosw *co_proto; 

	struct ifqueue 	co_pending; /* queue data to send when connection
						completes*/
#define MAX_DTE_LEN 0x7 /* 17 bcd digits */
	struct dte_addr	co_peer_dte;
	struct	cons_pcb *co_myself; /* DEBUGGING AID */
};

#define touch(copcb) copcb->co_ttl = copcb->co_init_ttl

/*
 * X.25 Packet types 
 */
#define XPKT_DATA		1
#define XPKT_INTERRUPT	2
#define XPKT_FLOWCONTROL 3 /* not delivered? */

/*
 * pcb xtates
 */

#define	CLOSED		0x0
#define	LISTENING	0x1
#define	CLOSING		0x2
/* USABLE STATES MUST BE LAST */
#define	CONNECTING	0x3
#define	ACKWAIT		0x4
#define	OPEN		0x5
#define MIN_USABLE_STATE CONNECTING

#define	cons_NSTATES		0x6


/* type */
#define CONSF_OCRE	0x40 /* created on OUTPUT */
#define CONSF_ICRE	0x20 /* created on INPUT */
#define CONSF_unused	0x10 /* not used */
#define CONSF_unused2	0x08 /* not used */
#define CONSF_DGM		0x04 /* for dgm use only */
#define CONSF_XTS		0x02 /* for cons-as-transport-service */
#define CONSF_LOOPBACK	0x01 /* loopback was on when connection commenced */

#define X_NOCHANNEL 0x80


struct cons_stat {
	u_int co_intr;	/* input from eicon board */
	u_int co_restart; /* ecn_restart() request issued to board */
	u_int co_slowtimo; /* times slowtimo called */
	u_int co_timedout; /* connections closed by slowtimo */
	u_int co_ack; /* ECN_ACK indication came from eicon board */
	u_int co_receive; /* ECN_RECEIVE indication came from eicon board */
	u_int co_send; /* ECN_SEND request issued to board */
	u_int co_reset_in; /* ECN_RESET indication came from eicon board */
	u_int co_reset_out; /* ECN_RESET issued to the eicon board */
	u_int co_clear_in; /* ECN_CLEAR indication came from eicon board */
	u_int co_clear_out; /* ECN_CLEAR request issued to board */
	u_int co_refuse; /* ECN_REFUSE indication came from eicon board */
	u_int co_accept; /* ECN_ACCEPT indication came from eicon board */
	u_int co_connect; /* ECN_CONNECT indication came from eicon board */
	u_int co_call; /* ECN_CALL request issued to board */
	u_int co_Rdrops; /* bad pkt came from ll */
	u_int co_Xdrops; /* can't keep up */

	u_int	co_intrpt_pkts_in; /* interrupt packets in */
	u_int co_avg_qlen;
	u_int co_avg_qdrop;
	u_int co_active;

	u_int co_noresources;
	u_int co_parse_facil_err;
	u_int co_addr_proto_consist_err;
	u_int co_no_copcb;
} cons_stat;


struct ifqueue consintrq; 

/* reasons for clear are in a data mbuf chained to a clear ecn_request */
struct e_clear_data 				{
	u_char ecd_cause;
	u_char ecd_diagnostic;
};

#ifdef _KERNEL
#define IncStat(XYZ) cons_stat.XYZ++
#endif /* _KERNEL */
