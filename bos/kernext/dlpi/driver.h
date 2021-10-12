/* @(#)18	1.3  src/bos/kernext/dlpi/driver.h, sysxdlpi, bos41J, 9519A_all 5/4/95 15:09:18  */
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: ATTACHED
 *		BCOPY
 *		BOUND
 *		DB
 *		INTR_LOCK
 *		INTR_UNLOCK
 *		LLCMSG
 *		NONI_LOCK
 *		NONI_UNLOCK
 *		SAPINDEX
 *		addstats
 *		decstats
 *		getmem
 *		incstats
 *		max
 *		min
 *		putmem
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
 * driver.h - private (non-exported) dlpi data structures
 */

/*
 **************************************************
 * Constants
 **************************************************
 */

#define	DLPINUM		2367	/* streams module id */
#define	DLPIHDR_LEN	(sizeof(union DL_primitives) + MAXADDR_LEN * 2)
#define	DL_LAST_PRIM	DL_GET_STATISTICS_ACK	/* max index in dl_funcs[] */

#define	DL_DADDR_OFFSET	(9*sizeof(ulong))
#define	DL_SADDR_OFFSET	(DL_DADDR_OFFSET + MAXADDR_LEN)

/*
 **************************************************
 * Macros
 **************************************************
 */

/* debugging */
#ifdef	DEBUG
#define	DB(x)		do { if (dl_Debug) { x; } } while (0)
#else
#define	DB(x)		/* null */
#endif
#define	TRC		!dl_Trace ? 0 : dl_trace
#define	ERR		dl_error

/* sigh, the only reason for extern-ing dl_handler[] and f2index[] */
#define	LLCMSG(l)	(dl_handler[f2index[l->ctl1]].msg)

#ifndef min
#define	min(a,b)	((a) <= (b) ? (a) : (b))
#define	max(a,b)	((a) >= (b) ? (a) : (b))
#endif

/* process-context memory allocation */
#define	getmem(t,n)	((t *)mi_zalloc(sizeof(t)*(n)))
#define	putmem(p)	(void)he_free(p)

/* has operation been done? */
#define	ATTACHED(dlb)	((dlb)->dlb_ndd)
#define	BOUND(dlb)	((dlb)->dlb_sap)

/* efficient bcopy */
#define BCOPY(f,t,l) do { \
	typedef struct { char x[l]; } a_t; \
	*(a_t*)(t) = *(a_t*)(f); \
} while (0)

/* convert sap into index for ppa.saps[] */
/* hack: maps netware 802.3 raw sap 0xff to sap 0 */
#define	SAPINDEX(s)	(((s) == 0xff) ? 0 : (((s) >> 1) & 0x7f))

/*
 * manipulate statistics counters
 */

#define	incstats(dlb, s) do { \
	if (dlb) { DLB *p = dlb; ++p->dlb_stats.s; } \
	++dl_stats.s; \
} while (0)
#define decstats(dlb, s) do { \
	if (dlb) { DLB *p = dlb; --p->dlb_stats.s; } \
	--dl_stats.s; \
} while (0)
#define	addstats(dlb, s, i) do { \
	if (dlb) { DLB *p = dlb; p->dlb_stats.s += i; } \
	dl_stats.s += i; \
} while (0)

#ifdef	_NOT_DEF
#define	nincstats(dlb, s)	incstats(dlb, s)
#define ndecstats(dlb, s)	decstats(dlb, s)
#define	naddstats(dlb, s, i)	addstats(dlb, s, i)
#else
#define	nincstats(dlb, s)	;
#define	ndecstats(dlb, s)	;
#define	naddstats(dlb, s, i)	;
#endif

/*
 * mutex
 *
 * noni - global non-interrupt mutex needs
 * intr - global interrupt mutex needs
 */

#define NONI_LOCK()	(void)lockl(&dl_noni_lock, LOCK_SHORT)
#define NONI_UNLOCK()	(void)unlockl(&dl_noni_lock)

#define	INTR_LOCK_DECL	int _snox
#define	INTR_LOCK()	_snox = disable_lock(PL_IMP, &dl_intr_lock)
#define	INTR_UNLOCK()	unlock_enable(_snox, &dl_intr_lock)

/*
 **************************************************
 * Types
 **************************************************
 */

typedef int (*pfi_t)();
typedef void (*pfv_t)();
typedef	struct ndd NDD;

/*
 * handler_t - LLC frame type information
 */

typedef struct {
	pfv_t	func;		/* frame handler */
	char	*msg;		/* trace message */
	int	flags;		/* frame handler characteristics */
} handler_t;

/*
 * timerq_t - llc timers
 */

typedef struct timerq_s {
	int	tag;	/* magic number so can verify message */
	struct dlb_s *dlb; /* stream identifier */
	mblk_t	*mp;	/* do not use! */
	int	fired;	/* has this timer fired yet? */
	pfv_t	func;	/* called when timer fires */
	ulong	arg[4];	/* 4 is magic: makes for a 64 byte allocb request */
} timerq_t;

/*
 * conn_t - connection records
 */

typedef struct conn_s {
	struct conn_s *next;		/* next connection record */
	struct dlb_s *dlb;		/* stream for this connection */
	struct bind_s *bind;		/* bind record for this connection */
	uchar	remaddr[PHYSLEN];	/* remote mac address */
	uchar	remsap;			/* remote ssap sap */
	char	listen;			/* listening stream if true */
	int	poll;			/* remembered poll/final */
} conn_t;

/*
 * prov_t - configured providers
 */

typedef struct prov_s {
	struct prov_s *p_next;		/* next configured provider */
	struct ppa_s *p_ppa;		/* list of ppa's for this provider */
	char	p_nddname[FMNAMESZ+1];	/* provider's ndd name */
	dev_t	p_major;		/* configured major */
	int	p_isether;		/* boolean: ethernet? (hack) */
	int	p_ref;			/* reference counter */
	int	p_drd;			/* need DRD for this provider? */
} prov_t;

/*
 * ppa_t - per interface control info
 */

typedef struct ppa_s {
	struct ppa_s *next;		/* next ppa for this interface */
	int	tag;			/* structure id */
	ndd_t	*ndd;			/* saved ndd from ns_alloc() */
	int	ppa;			/* name of this ppa */
	int	ref;			/* reference counter */
	conn_t	*saps[128][2];		/* incoming sap demuxing info */
} ppa_t;

/*
 * drdreq_t - DRD need-route request
 */

typedef struct drdreq_s {
	struct drdreq_s *next;		/* next pending request; no order */
	int tag;			/* identifier */
	mblk_t *mp;			/* mp holding this request */
	struct dlb_s *dlb;		/* requesting stream */
	pfv_t func;			/* callback function */
	void *arg;			/* generic argument to callback */
	uchar *addr;			/* pointer to remote address */
	uchar *seg;			/* pointer to routing segment */
	int seglen;			/* segment length */
} drdreq_t;

/*
 **************************************************
 * Per Stream Structures
 **************************************************
 */

/*
 * sapu_t - anatomy of a SAP
 */

typedef union sapu {
	struct {
		uchar	dsap;
		uchar	ssap;
		ushort	type;
	} su_dst;		/* AIX 3.2.0 style: deprecated */
	ulong	su_sap;		/* The Real Thing */
} sapu_t;

/*
 * bind_t - N:1 with a stream, these are bound to types
 */

typedef struct bind_s {
	struct bind_s *next;	/* other records bound on this stream */
	struct ns_8022 ns;	/* what we registered */
	int	class;		/* how bound */
} bind_t;

/*
 * multi_t - registered multicast list
 */

typedef struct multi_s {
	struct multi_s *next;
	uchar	addr[PHYSLEN];
} multi_t;

/*
 * codls_t - connection oriented variables
 */

typedef struct codls_s {
	int	s_flag;		/* saw sabme while sending sabme */
	int	retry;		/* (n2) retry counter */
	int	ir_ct;		/* (n3) i-frame received counter */
	int	vb;		/* (vb) busy status */
	int	vs;		/* next frame to send (sent NS) */
	int	vr;		/* next frame to receive (sent NR/recv'd NS) */
	int	va;		/* last frame acked (recv'd NR) */
	int	poll;		/* (vp) poll status */
	int	final;		/* (vp) final status */
	int	localreset;	/* locally initiated reset? */

	timerq_t *tq1;		/* T1,P timer */
	timerq_t *tq2;		/* T2,REJ timer */
	timerq_t *tqi;		/* TI timer */

	mblk_t	*qhead;		/* head of messages for rexmit */
	mblk_t	*qtail;		/* tail of messages for rexmit */

	struct mbuf *mhead;	/* incoming frames */
	struct mbuf *mtail;
} codls_t;

/* codls.busy values */
#define	FB_FLOW		0x0001		/* write flow-controlled */
#define	FB_LOCAL	0x0002		/* local busy: send RNRs */
#define	FB_REMOTE	0x0004		/* remote busy: send only S-frames */
#define	FB_POLL		0x0008		/* sent poll: want final */
#define	FB_REJECT	0x0010		/* sent REJ */
#define	FB_REJWAIT	0x0020		/* REJ timer active */

/*
 * dlb_t - per stream state record
 */

typedef struct dlb_s {
	int	dlb_tag;		/* structure ID */
	int	dlb_minor;		/* minor number */
	queue_t	*dlb_rq;		/* our queue */
	queue_t	*dlb_wq;		/* convenient */
	prov_t	*dlb_prov;		/* our provider */

	/* attach variables */
	ppa_t	*dlb_ppa;		/* our ppa */
	int	dlb_mactype;		/* infoack mactype */
	int	dlb_drd;		/* DRD used for this provider? */

	/* bind variables */
	sapu_t	dlb_sapu;		/* first bound sap */
	int	dlb_mode;		/* DL_CLDLS or DL_CODLS */
	int	dlb_conind;		/* max value for npend */
	int	dlb_xidtest;		/* how to handle XID, TEST */
	bind_t	*dlb_bound;		/* list of bound types */

	/* constructed address info (from bind) */
	uchar	dlb_addr[MAXADDR_LEN];	/* infoack address */
	int	dlb_addrlen;
	long	dlb_saplen;
	llcsnap_t dlb_llc;		/* optimization: default unitdata llc */

	/* multicast, promisc variables */
	multi_t	*dlb_multi;		/* list of multicast addresses */
	ulong	dlb_promisc;		/* promiscuous request */

	/* connection variables */
	conn_t	*dlb_conn;		/* connection record, if connected */
	uchar	dlb_remaddr[MAXADDR_LEN]; /* remote address */
	int	dlb_remaddrlen;		/* and its length */
	uchar	dlb_seg[MAXROUTE_LEN];	/* routing segments to remote */
	int	dlb_seglen;		/* length of routine in dlb_seg[] */
	int	dlb_npend;		/* number of pending connections */
	conn_t	*dlb_pend;		/* array of pending connections */
	codls_t	dlb_codls;		/* connection variables */

	/* misc variables */
	int	dlb_state;		/* current DLPI state */
	int	dlb_flags;		/* misc control flags */
	int	dlb_priv;		/* boolean: privileged stream? */
	int	dlb_netware;		/* boolean: netware stream? */
	ulong	dlb_pkt_format;		/* user requested packet format */
	int	(*dlb_output)();	/* output resolve routine */
	void	(*dlb_input)();		/* intput resolve routine */
	mblk_t	*dlb_failmp;		/* "in case of emergency..." */
	llctune_t dlb_tune;		/* tunable parameters */
	stats_t	dlb_stats;		/* per-stream stats */
} dlb_t, DLB;

/* dlb.flags */
#define	FD_QENABLE	0x0001		/* need to qenable wq */
#define	FD_REXMIT	0x0002		/* need to retransmit */

/* dlb pseudo-fields (derived types) ... */
#define	dlb_sap 	dlb_sapu.su_sap
#define	dlb_type	dlb_sapu.su_dst.type
#define	dlb_dsap	dlb_llc.dsap
#define	dlb_ssap	dlb_llc.ssap

/* ... from ppa */
#define	dlb_isether	dlb_prov->p_isether
#define	dlb_ndd		dlb_ppa->ndd
#define	dlb_physlen	dlb_ppa->ndd->ndd_addrlen

/* ... from codls */
#define	dlb_s_flag	dlb_codls.s_flag
#define	dlb_retry	dlb_codls.retry
#define	dlb_ir_ct	dlb_codls.ir_ct
#define	dlb_vb		dlb_codls.vb
#define	dlb_vs		dlb_codls.vs
#define	dlb_vr		dlb_codls.vr
#define	dlb_va		dlb_codls.va
#define	dlb_poll	dlb_codls.poll
#define	dlb_final	dlb_codls.final
#define	dlb_localreset	dlb_codls.localreset
#define	dlb_tq1		dlb_codls.tq1
#define	dlb_tq2		dlb_codls.tq2
#define	dlb_tqi		dlb_codls.tqi
#define	dlb_qhead	dlb_codls.qhead
#define	dlb_qtail	dlb_codls.qtail
#define	dlb_mhead	dlb_codls.mhead
#define	dlb_mtail	dlb_codls.mtail

/* ... from tune */
#define	dlb_t1		dlb_tune.t1
#define	dlb_t2		dlb_tune.t2
#define	dlb_ti		dlb_tune.ti
#define	dlb_n1		dlb_tune.n1
#define	dlb_n2		dlb_tune.n2
#define	dlb_n3		dlb_tune.n3
#define	dlb_k		dlb_tune.k

/*
 **************************************************
 * Debugging Aids: structure identifiers
 **************************************************
 */

#define	TAG_TOCB	0x544f4342	/* "TOCB" - timerq_t */
#define	TAG_PPA		0x50504120	/* "PPA " - ppa_t */
#define	TAG_CONN	0x434f4e4e	/* "CONN" - conn_t */
#define	TAG_DLB		0x444c4220	/* "DLB " - dlb_t */
#define	TAG_DLPI	0x444c5049	/* "DLPI" - dlpivar_t */
#define	TAG_DRD		0x44524400	/* "DRD " - drdreq_t, isr_data */

/*
 **************************************************
 * Prototypes and Globals
 **************************************************
 */

/* dlack.c */
extern mblk_t *errack(), *okack(), *uderrack();
extern void dl_deq();

/* misc.c */
extern void dl_flush();
extern mblk_t *dl_gethdr();
extern llc_t *setdsap();
extern conn_t *findpend(), *findconn(), *findstream();

/* llcmisc.c */
extern void local_busy(), local_okay();
extern void setup_timers(), stop_timers();
extern void init_session(), term_session(), abort_session();

/* convert.c */
extern mblk_t *mbuf_to_mblk();
extern struct mbuf *mblk_to_mbuf();

/* timer.c */
extern timerq_t *tq_alloc();
extern void tq_free(), tq_fire();
extern void tq_start(), tq_restart(), tq_stop();

/* tx.c */
extern struct mbuf *mkframe();
extern int tx_frame(), tx_frame2();
extern void tx_rsp();

/* drd.c */
extern void drd();
extern void drd_bind(), drd_unbind();
extern void drd_flush();

/* globals.c */
extern int dl_noni_lock;
extern Simple_lock dl_intr_lock;
extern stats_t dl_stats;
extern uchar dl_broadcast[6];
extern void (*dl_funcs[])();
extern int dl_Debug, dl_Trace;
extern handler_t dl_handler[];
extern char f2index[];
