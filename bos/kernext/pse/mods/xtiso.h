/* @(#)58     1.2.2.7  src/bos/kernext/pse/mods/xtiso.h, sysxpse, bos412, 9444A412a 10/27/94 15:04:46 */
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 18 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 *   ALL RIGHTS RESERVED
 *
 */


#ifndef _XTISO_H
#define _XTISO_H

#define	nilp(t)	((t *)0)
#define	nil(t)	((t)0)
typedef	char	* IDP;
typedef	int	(*pfi_t)();

/*
 * Protocol Information Structure
 */
typedef struct { 
	int	 xp_configured;		/* protocol configured		*/
	int	 xp_dom;		/* socket domain		*/
	int	 xp_type;		/* socket type  		*/
	int	 xp_proto;		/* socket protocol		*/
	int	 xp_servtype;		/* service type supported	*/
	long	 xp_tsdulen;		/* max TSDU size		*/
	long	 xp_etsdulen;		/* max ETSDU size		*/
	long	 xp_disconlen;		/* max disconnection data size	*/
	long	 xp_connectlen;		/* max connection data size 	*/
	long	 xp_addrlen;		/* max protocol addr size	*/
	long	 xp_optlen;		/* max protocol option size 	*/
	long	 xp_tidulen;		/* max trans protocol i/f size  */
} xtiproto_t;

/* 
 * Connection Indication Structure
 */
struct xtiseq { 
	long    seq_no;			/* sequence number		*/
	int     seq_used;		/* used or not?			*/
	struct  socket *seq_so;		/* associated socket		*/
};

/* Statically defined */
#define XTI_MAXQLEN	5		/* Remainder queued in socket	*/

struct xti_def_opts {
	unsigned long		xti_rcvbuf;
	unsigned long		xti_rcvlowat;
	unsigned long		xti_sndbuf;
	unsigned long		xti_sndlowat;
	struct linger		xti_linger; 
	char			xti_debug;
	/* IP specific - union with other protocols later */
	char			ip_broadcast;
	char			ip_dontroute;
	char			ip_reuseaddr;
	struct mbuf		*ip_options;
	unsigned char		ip_tos;
	unsigned char		ip_ttl;
	char			udp_checksum;
	char			tcp_nodelay;
	unsigned long		tcp_maxseg;
	struct t_kpalive	tcp_keepalive; 
};

/*
 * XTI-Socket Control Block
 */
struct xticb {
	int	xti_state;		/* state of transport provider	*/
	long	xti_flags;		/* internal flags, see below	*/
	dev_t	xti_minor;		/* minor device # 		*/

	struct	socket   *xti_so;	/* corresponding socket		*/
	int	xti_sostate;		/* safe state of socket		*/
	int	xti_soerror;		/* safe socket error		*/
	long	xti_sorcount;		/* safe read-count of socket	*/
	long	xti_sowspace;		/* safe write-space of socket	*/
	int	xti_sosnap;		/* async state of socket	*/

	struct	socket   *xti_lso;	/* "saved" listening socket	*/
	struct	xtiseq    xti_seq[XTI_MAXQLEN];		
					/* outstanding conn ind seq #s	*/
	short	xti_qlen;		/* # of conn ind. allowed	*/
        short	xti_cindno;		/* # of outstanding conn ind	*/

	int	xti_seqcnt;		/* sequence counter 		*/
        int	xti_pendind;		/* pending indication		*/
	long	xti_tsdu;
	long	xti_etsdu;

	queue_t	*xti_rq;		/* stream read queue ptr	*/
	queue_t	*xti_wq;		/* stream write queue ptr	*/

	mblk_t	*xti_wdata;		/* pending write-side data	*/
	mblk_t	*xti_wexdata;		/* pending write-side exdata	*/
	struct	mbuf *xti_wnam;		/* destination addr of unitdata	*/
	struct	mbuf *xti_rdata;	/* pending read-side (ex)data	*/
	int	xti_rflags;		/* pending read-side flags	*/

	int	(*xti_pendcall)();	/* pending routine to call	*/
	int	xti_bufcallid;		/* internal bufcall id		*/
	int	xti_tlierr;		/* xti error recovery		*/
	int	xti_unixerr;		/* unix error recovery		*/
	int	xti_errtype;		/* error type recovery		*/

	struct	xtisocfg *xti_cfg;	/* link back to cfg head	*/
#define	xti_proto xti_cfg->xti_cfgproto	/* xtiproto for xticb		*/

	struct	xti_def_opts xti_def_opts; /* options managed from app  */
	struct	mbuf *xti_nam;		/* save original bind address   */
	char	*xti_opt;		/* save options to return	*/
	int	xti_optlen;		/* saved option length		*/

	Simple_lock  xtiso_lock;        /* MP lock                      */
};

/*
 * XTI configuration block.
 */
struct xtisocfg {
	struct	xtisocfg	*xti_cfgnext;	/* more config structs	*/
	dev_t 			xti_cfgmajor;	/* major device # 	*/
	int			xti_cfgnopen;	/* # of current opens	*/
	xtiproto_t		xti_cfgproto;	/* protocol information	*/
	lock_t			xti_cfglock;	/* for serialization */
};

/*
 * Socket types
 */
#define XTI_NEWSOCK     1		/* New Socket, open		*/
#define XTI_NOSOCK	2		/* Socket disabled, no close 	*/
#define XTI_CLOSESOCK	3		/* Close indicator 		*/

/* 
 * Internal XTI Flags
 */
#define XTI_ACTIVE	0x0001		/* if set, this xticb is in use */
#define XTI_FATAL	0x0002		/* fatal condition occurred     */
#define XTI_DISCONMAIN	0x0004		/* disconnect the main conn.    */
#define XTI_PRIV	0x0008		/* created by privileged user    */
#define XTI_MOREDATA	0x0010		/* more data expected from TU   */
#define XTI_MOREEXDATA	0x0020		/* more exdata expected from TU	*/
#define XTI_FLOW	0x0040		/* flow control on output       */
#define XTI_TIMEOUT	0x0200		/* bufcall active on write side */
#define XTI_ISCLOSING	0x0400		/* close in progress  		*/
#define XTI_OPTINIT	0x1000		/* options initialized		*/
#define XTI_OPTMODIFY	0x2000		/* options modified		*/
#define XTI_NEEDSTARTUP	0x4000		/* restart after shutdown	*/
#define XTI_ORDREL	0x8000		/* if set, soshutdown is not needed */

/* 
 * State for connection indications
 */
#define	XTIS_AVAILABLE	0		/* Available			*/
#define XTIS_AWAITING	1		/* Awaiting acceptance		*/
#define XTIS_ACTIVE	2		/* Connected/accepted		*/
#define XTIS_LOST	3		/* Connection lost before accept*/

/*
 * Disconnect reason codes
 */
#define XTID_TPINIT		1  /* Initiated by transport provider    */
#define XTID_REMWITHDRAW	2  /* Connect req withdrawn by remote    */
#define XTID_REMREJECT  	3  /* Connect req rejected by remote     */
#define XTID_REMINIT		4  /* Disconnect req Initiated by remote */

/*
 * UDERR IND error types
 */
#define XTIU_BADOPTSZ		1  /* Initiated by transport provider    */
#define XTIU_BADMSGSZ		2  /* Connect req withdrawn by remote    */
#define XTIU_NOOPT		1  /* UDP Option not supported           */

#define XTI_INFO_ID	5010	/* module/driver ID */

#define XTI_INIT_HI	4096	/* hi-water mark, flow control on queue */
#define XTI_INIT_LO	1024	/* lo-water mark, flow control on queue */

/* Glue for union T_primitives for now */
#define	tbindack	bind_ack
#define	tbindreq	bind_req
#define	tconncon	conn_con
#define	tconnind	conn_ind
#define	tconnreq	conn_req
#define	tconnres	conn_res
#define	tdataind	data_ind
#define	tdatareq	data_req
#define	tdisconind	discon_ind
#define	tdisconreq	discon_req
#define	texdataind	exdata_ind
#define	texdatareq	exdata_req
#define	terrorack	error_ack
#define	tinfoack	info_ack
#define	tinforeq	info_req
#define	tokack		ok_ack
#define	toptmgmtack	optmgmt_ack
#define	toptmgmtreq	optmgmt_req
#define	tordrelind	ordrel_ind
#define	tordrelreq	ordrel_req
#define	tunbindreq	unbind_req
#define	tuderrorind	uderror_ind
#define	tunitdataind	unitdata_ind
#define	tunitdatareq	unitdata_req
#define	taddrreq	addr_req
#define	taddrack	addr_ack

/*
 * Debugging
 */

#ifdef	STREAMS_DEBUG
#define XTIDEBUG	1	/* Not quite, but handy */
#else
#undef XTIDEBUG
#endif

#if	XTIDEBUG

/*
 * Internal DEBUG flags
 */
#define XTIF_ALL	(~0)
#define XTIF_CONFIGURE	0x00000001	/* configuration  		*/
#define XTIF_OPEN	0x00000002	/* open routine  		*/
#define XTIF_WPUT	0x00000004	/* write put routine 		*/
#define XTIF_MISC	0x00000008	/* miscellaneous trace 		*/
#define XTIF_WSRV	0x00000010	/* write service routine	*/
#define XTIF_OUTPUT	0x00000020	/* xti output routine		*/
#define XTIF_INPUT	0x00000040	/* xti input routine		*/
#define XTIF_EVENTS	0x00000080	/* socket event tracing		*/
#define XTIF_INFO	0x00000100	/* info request			*/
#define XTIF_BINDING	0x00000200	/* binding-related tracing	*/
#define XTIF_CONNECT	0x00000400	/* connect/disconnect trace	*/
#define XTIF_DATA	0x00000800	/* data xfer trace		*/
#define XTIF_SEND	0x00001000	/* xti send routine trace	*/
#define XTIF_RECV	0x00002000	/* xti receive routine trace 	*/
#define XTIF_CLOSE	0x00004000	/* close-related trace		*/
#define XTIF_ERRORS	0x00008000	/* stream error trace		*/
#define XTIF_SOCKET	0x00010000	/* socket misc			*/
#define XTIF_CHECKPOINT	0x00020000	/* routine entry trace		*/
#define XTIF_SEND_FLOW	0x00040000	/* send flow control trace	*/
#define XTIF_STRLOG	0x00080000	/* enable strlog trace		*/
#define XTIF_BREAK	0x40000000	/* enable debugger on "panic"	*/
#define XTIF_PANIC	0x80000000	/* enable "real" panic		*/

extern int xtiDEBUG;

#define PUTNEXT(q,mp)	xti_putnext((q),(mp))
#define XTITRACE(f,v)	do { if (xtiDEBUG & (f)) { v } } while (0)
#define CHECKPOINT(v)	XTITRACE(XTIF_CHECKPOINT, \
			 printf( "\n xtiso: ========>> entered %s()...\n",(v));)

#else	/* !XTIDEBUG */

#define PUTNEXT(q,mp)	putnext((q),(mp))
#define XTITRACE(f,v)
#define CHECKPOINT(v)

#endif	/* !XTIDEBUG */

#endif /* _XTISO_H */
