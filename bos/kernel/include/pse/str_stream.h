/* @(#)04       1.37  src/bos/kernel/include/pse/str_stream.h, sysxpse, bos41J, 9521B_all 5/25/95 15:33:02 */
/*
 * COMPONENT_NAME: SYSXPSE - Streams Framework 
 * 
 * ORIGINS: 63, 71, 83
 *
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988-1991  Mentat Inc. */

#ifndef	_STR_STREAM_H
#define	_STR_STREAM_H

#include <pse/str_system.h>
#include <sys/sleep.h>

#include <sys/uio.h>
#include <sys/encap.h>
#include <pse/str_lock.h>
#include <sys/lock_alloc.h>
#ifdef SQH_LOCK_COUNT
short sqh_lock_count = 0;
#else
extern short sqh_lock_count;
#endif

#include <net/net_malloc.h>

/*
 * Note: <sys/stream.h> is included later, since we need to complete
 * the configuration first.
 */

#include <pse/str_debug.h>

#if	STREAMS_DEBUG
#define staticf
#else
#define staticf	static
#endif

#define INTR_STACK		0x3000
#define INITDONE		0x8000
#define cdevsw  devsw

#define	nilp(t)			((t *)0)
#define	nil(t)			((t)0)
#define reg			register
#define LONG_SIGN_BIT		((long) ~((~((unsigned long)0)) >> 1))
#define	MS_TO_TICKS(ms)		(((ms)*hz)/1000)
#define	TICKS_TO_MS(ticks)	(((ticks)*1000)/hz)

/*
 * The Synch Queues arbitrate all access to Streams queues.
 * A synch queue is a synch queue head if the SQ_IS_HEAD bit
 * is set in sqh_flags, else it's a sync queue element. The
 * first three elements of the SQH and the SQ must be identical.
 */

struct sqh_s {
	struct sqe_s *		sqh_next;
	struct sqe_s *		sqh_prev;
	long			sqh_flags;
	struct sqh_s *		sqh_parent;
	struct sqe_s *		sqh_owner;
	int			sqh_refcnt;
	decl_simple_lock_data(EMPTY_FIELD, sqh_lock)
};
typedef	struct sqh_s	SQH;
typedef	struct sqh_s *	SQHP;

typedef	void	(*sq_entry_t)(void *, void *);

struct sqe_s {
	struct sqe_s *		sq_next;
	struct sqe_s *		sq_prev;
	long			sq_flags;
	struct sqh_s *		sq_target;
	struct pse_queue *	sq_queue;
	sq_entry_t		sq_entry;
	void *			sq_arg0;
	void *			sq_arg1;
	int			sq_sleeper;
	int			sq_count;
};
typedef struct sqe_s	SQ;
typedef struct sqe_s *	SQP;

/* Special SQH used when synchronizing multiple SQH's. */
extern	SQH		mult_sqh;
/* Global SQH used by globally synchronized modules */
extern	SQH		sq_sqh;

/* SQ flags */
#define	SQ_IS_HEAD	0x00000001
#define	SQ_INUSE	0x00000002
#define	SQ_HOLD		0x00000004
#define	SQ_QUEUED	0x00000008
#define	SQ_IS_TIMEOUT	0x00000010
#define	SQ_IS_FUNNEL	0x00000400

typedef struct t_list {
	struct	t_list	* next;
	tid_t		t_id;
} tidlist;

/*
 *      A generic doubly-linked list (queue).
 */

struct queue_entry {
	struct queue_entry	*next;		/* next element */
	struct queue_entry	*prev;		/* previous element */
};

/*
 * The following macros define some additional parts of certain data
 * structures, which are needed by the stream head in order to provide
 * synchronization.
 *
 * Note that these macros are undefined for normal modules and drivers.
 * This gives them a wrong impression about the size of those structures.
 * But that should not matter, since - according to the STREAMS spec -
 * they should not make such assumptions anyway. The idea is that
 * module code also becomes independent of the configuration, and binary
 * compatibility is guaranteed.
 *
 * Note: even modules which include this file for debugging purposes, should
 * not refer to these fields, since they do not necessarily form a standard
 * to which OSF/1 will adhere in the future.
 */

/*
 * Extra fields for queue structure:
 *	struct queue *	q_ffcp;		Forward flow control pointer
 *	struct queue *	q_bfcp;		Backward flow control pointer
 *	SQH		q_sqh;		Sync queue head for laterals
 *	SQP		q_runq_sq;	Sync queue element for runq
 *	simple_lock_data_t q_qlock;	short term lock for (de)queueing
 *
 *	struct queue_entry q_act;	... currently active queues
 *	tidlist		q_tids;		and thread identifier,
 *					for identification
 *
 * These present only if SEC_BASE defined
 *	struct msgb *	b_attr;		pointer to security attributes
 *
 * Extra fields for message structure:
 *	SQ		b_sq;		Sync queue element for lateral
 *
 * The q_runq_sq is a pointer to permit sizeof (queue_t) to be <128 bytes
 * in the production configuration. Increase with care.
 */

#if SEC_BASE

#define MSG_KERNEL_SEC_FIELDS			\
	struct msgb *		b_attr;

#else

#define MSG_KERNEL_SEC_FIELDS

#endif

#define	QUEUE_KERNEL_FIELDS			\
	struct pse_queue *	q_ffcp;		\
	struct pse_queue *	q_bfcp;		\
	SQH			q_sqh;		\
	SQP			q_runq_sq;	\
	struct queue_entry 	q_act;		\
	tidlist			q_tids;		\
	int			(*q_is_interesting) (mblk_t *); \
	int			q_int_count;	\
	decl_simple_lock_data(EMPTY_FIELD, q_qlock) \
	struct	wantio *	q_wantio;

#define	MSG_KERNEL_FIELDS 			\
	SQ			b_sq;		\
	MSG_KERNEL_SEC_FIELDS

/*
 * OK, we can include <sys/stream.h> *now*, after having defined those
 * extra pieces.
 */

#include <sys/strconf.h>
#include <sys/stream.h>
#include <sys/sad.h>

#undef	tsleep
#undef	e_sleep
#undef	e_sleepl
#undef	e_mpsleep
#undef  e_block_thread
#undef	sleep
#undef	sleepx
#define tsleep(a,b,c,d)  sleepx((a),(b),0)

/*
 * Streams framework does not want to undef the followings, so it can use 
 * these symbols as defined in stream.h. The framework will define STR_DEFINES 
 * and so for others who includes this file, the undef's will be done.
 */

#ifndef STR_DEFINES
#undef	timeout
#undef	untimeout
#endif

typedef	struct msgb *	MBLKP;
typedef	struct msgb **	MBLKPP;

#define MSGEXT 0x08 /* external association. Define should be in stream.h */

/* Extra PSE mblk type */
#define       M_MI            64
/* Subfields for M_MI messages */
#define       M_MI_READ_RESET 1
#define       M_MI_READ_SEEK  2
#define       M_MI_READ_END   4

/* Extra for FLUSHALL, FLUSHDATA */
#define	FLUSH_CAN_CLOSE		0x8000
#define	FLUSH_NOTIFYONLY	0x4000

#define	DEF_CLOSE_WAIT	15	/* seconds to wait per non-empty module
				 * or driver during close. */
#define	STH_HASH_TBL_SIZE 32	/* Now defined only here. */

#include <sys/stropts.h>	/* for struct bandinfo, etc. */

typedef struct ioctl_osr_s {
	struct iovec ioc_oia[3];
	mblk_t *ioc_data;
	/* Ioctl data buffer - large enough for largest copyin/out */
	union streams_ioctl_buffer {
		char			c[FMNAMESZ+1];
		int			i;
		long			l;
		caddr_t			p;
		struct stat		sb;
		struct bandinfo		b;
		struct str_list		s1;
		struct strfdinsert	s2;
#ifdef SEC_BASE
		struct strfdinsert_attr	s2a;
#endif
		struct strioctl		s3;
#ifdef SEC_BASE
		struct strioctl_attr	s3a;
#endif
		struct strpmsg		s4;
#ifdef SEC_BASE
		struct strpmsg_attr	s4a;
#endif
		struct strpeek		s5;
#ifdef SEC_BASE
		struct strpeek_attr	s5a;
#endif
		struct strrecvfd	s6;
#ifdef SEC_BASE
		struct strrecvfd_attr	s6a;
		/* there is no struct strsendfd */
		struct strsendfd_attr	s7a;
#endif
	} ioc_buf;
} IOCTL_OSR;

#define	osr_ioctl_arg0		osr_osru.ioctl_osr.ioc_oia[0].iov_len
#define	osr_ioctl_arg0p		osr_osru.ioctl_osr.ioc_oia[0].iov_base
#define	osr_ioctl_arg0_len	osr_osru.ioctl_osr.ioc_oia[0].iov_len
#define	osr_ioctl_arg1		osr_osru.ioctl_osr.ioc_oia[1].iov_len
#define	osr_ioctl_arg1p		osr_osru.ioctl_osr.ioc_oia[1].iov_base
#define	osr_ioctl_arg1_len	osr_osru.ioctl_osr.ioc_oia[1].iov_len
#define	osr_ioctl_arg2		osr_osru.ioctl_osr.ioc_oia[2].iov_len
#define	osr_ioctl_arg2p		osr_osru.ioctl_osr.ioc_oia[2].iov_base
#define	osr_ioctl_arg2_len	osr_osru.ioctl_osr.ioc_oia[2].iov_len
#define	osr_ioctl_data		osr_osru.ioctl_osr.ioc_data

typedef struct open_osr_s {
	dev_t	open_dev;
	int	open_dindex;
	int	open_fflag;
} OPEN_OSR;

#define	osr_open_dev		osr_osru.open_osr.open_dev
#define	osr_open_dindex		osr_osru.open_osr.open_dindex
#define	osr_open_fflag		osr_osru.open_osr.open_fflag

typedef struct rw_osr_s {
	struct uio *rw_uio;
	int	rw_total;
	int	rw_count;
	int	rw_offset;
	enum uio_rw	uio_rw;
} RD_OSR;

#define	osr_rw_uio	osr_osru.rw_osr.rw_uio
#define	osr_rw_total	osr_osru.rw_osr.rw_total
#define	osr_rw_count	osr_osru.rw_osr.rw_count
#define	osr_rw_offset	osr_osru.rw_osr.rw_offset
#define	osr_rw_rw	osr_osru.rw_osr.uio_rw

struct osrq_s {
	struct osr_s *		osrq_first;
	struct osr_s *		osrq_last;	/* (Undefined when osrq_first is nil!) */
	decl_simple_lock_data(EMPTY_FIELD, osrq_lock)
	int			osrq_sleeper;
};

typedef	struct osrq_s	OSRQ;
typedef	struct osrq_s *	OSRQP;

struct osr_s {
	SQ		osr_sq;	  /* this OSR's synch queue element */
	struct osrq_s *	osr_osrq; /* pointer to OSRQ where this OSR is queued */
	struct osr_s *	osr_next; /* next OSR in the same OSRQ */
	struct sth_s *	osr_sth;
	int		osr_ret_val;
	int		(*osr_handler)(struct osr_s *);
	ulong		osr_closeout;
	ulong		osr_flags;
	int		osr_bufcall_id;
	int		osr_awakened;
	cred_t  	*osr_creds;
	union {
		IOCTL_OSR	ioctl_osr;
		OPEN_OSR	open_osr;
		RD_OSR		rw_osr;
	} osr_osru;
	int		osr_sleeper;
};

typedef	struct osr_s	OSR;
typedef struct osr_s *	OSRP;
typedef	struct osr_s **	OSRPP;

/*
 * Values for osr_flags
 *
 * F_OSR_NEED_MULT_SQH	 - the OSR must allocate mult_sqh first
 * F_OSR_HAVE_MULT_SQH	 - this OSR is holding the mult_sqh
 * F_OSR_NDELAY		 - this OSR wants non-delay mode
 * F_OSR_NONBLOCK	 - this OSR wants non-block mode
 * F_OSR_RTTY_CHECK 	 - OSR needs read-style bg check if tty
 * F_OSR_WTTY_CHECK 	 - OSR needs write-style bg check if tty
 * F_OSR_ITTY_CHECK 	 - OSR needs ioctl-style bg check if tty
 * F_OSR_BLOCK_TTY	 - BG proc. attempting access to ctl. tty: block it
 * F_OSR_AUDIT_READ	 - SEC_BASE ioctl: generate read audit record
 * F_OSR_AUDIT_WRITE	 - SEC_BASE ioctl: generate write audit record
 */

#define F_OSR_NEED_MULT_SQH	0x00000001
#define F_OSR_HAVE_MULT_SQH	0x00000002
#define F_OSR_NDELAY		0x00000004
#define F_OSR_NONBLOCK		0x00000008
#define F_OSR_NBIO		(F_OSR_NDELAY|F_OSR_NONBLOCK)
#define F_OSR_RTTY_CHECK	0x00000010
#define F_OSR_WTTY_CHECK	0x00000020
#define F_OSR_ITTY_CHECK	0x00000040
#define F_OSR_TTYBITS	(F_OSR_RTTY_CHECK|F_OSR_WTTY_CHECK|F_OSR_ITTY_CHECK)
#define F_OSR_BLOCK_TTY		0x00000080
#define F_OSR_AUDIT_READ	0x00000100
#define F_OSR_AUDIT_WRITE	0x00000200
#define F_OSR_KCOPY		0x00010000
#define F_OSR_TTY_IOCTL		0x00020000

struct sthq_s {
	struct sthq_s *next, *prev;
};

struct poll_s {
	struct sthq_s	ps_link;
	struct sth_s *	ps_sth;
	ushort		ps_events;
	chan_t		ps_chan;
};

typedef struct poll_s		POLLS;
typedef struct poll_s *		POLLSP;
typedef struct sthq_s		POLLQ;
typedef struct sthq_s *		POLLQP;
#define sth_poll_active(sth)	\
	((sth)->sth_pollq.next != &(sth)->sth_pollq)

struct sigs_s {
	struct sthq_s	ss_q;
	struct sthq_s	ss_pq;
	struct sth_s *	ss_sth;
	pid_t		ss_pid;
        ulong     	ss_mask;
        ulong     	ss_async;
};

typedef struct sigs_s		SIGS;
typedef struct sigs_s *		SIGSP;
typedef struct sthq_s		SIGSQ;
#define sth_sigs_active(sth)	\
	((sth)->sth_sigsq.next != &(sth)->sth_sigsq)
#define ss_link		ss_q.next
#define ss_proclink	ss_pq.next

/*
 * Stream head tty control structure.
 * Allocated at stream head tty open.
 */
typedef	struct	shtty_s	{
	pid_t	shtty_pgrp;		/* process grou identificator	*/
	pid_t	shtty_sid;		/* session identificator	*/
	pid_t	shtty_tsm;		/* terminal state manager id	*/
	ulong	shtty_flags;		/* general streamhead tty flags	*/
					/* for the erminal state.	*/
};

/*
 * Stream head tty definitions for shtty_flags.
 */
#define	F_SHTTY_TRUST	0x0001		/* terminal trusted or not	*/
#define	F_SHTTY_SAK	0x0002		/* sak sequence enabled or not	*/
#define	F_SHTTY_KEP	0x0004		/* keyboard emulation program	*/
					/* emulated or not		*/
#define	F_SHTTY_CONS	0x0008		/* console redirected or not	*/

/* Stream head control structure */
/* Warning - production configuration almost 256 bytes - increase with care. */
struct sth_s {
	queue_t *	sth_rq;		/* Stream head read queue */
	queue_t *	sth_wq;		/* Stream head write queue */
	dev_t		sth_dev;	/* Device number of stream */
	ulong		sth_read_mode;	/* RNORM, etc. */
	ulong		sth_write_mode;	/* SNDZERO if 0 byte messages are ok */
	int		sth_close_wait_timeout;	/* ms to wait during close */
	uchar		sth_read_error;
	uchar		sth_write_error;
	uchar		sth_prev_band;	/* last band written to sth */
	uchar		sth_pad;
	ulong		sth_flags;
	int		sth_ioc_id;	/* id of outstanding M_IOCTL */
	mblk_t *	sth_ioc_mp;	/* M_IOCACK or M_IOCNAK reply */
	OSRQ		sth_ioctl_osrq;	/* Ioctl request queue */
	OSRQ		sth_read_osrq;	/* Read request queue */
	OSRQ		sth_write_osrq;	/* Write request queue */
	ulong		sth_wroff;	/* Write offset to prepend to M_DATA */
	int		sth_muxid;	/* linked id of lower mux */
	struct sth_s *	sth_mux_link;	/* link to next muxed lower stream */
	struct sth_s *	sth_mux_top;	/* controlling mux list */
	struct sth_s *	sth_pmux_top;	/* mux list for persistent links */
	struct shtty_s * shttyp;	/* pointer to tty informations	*/
	struct sth_s *	sth_next;	/* Link on sth_open_streams list */
	POLLQ		sth_pollq;	/* list of active polls/selects */
	SIGSQ		sth_sigsq;	/* list of I_SETSIGs */
	int		sth_push_cnt;	/* Number of modules pushed */
	POLLS		sth_polls;	/* Poll per sth */
	OSR		sth_osr;	/* Osr per sth */
	ulong		sth_ext_flags;	/* Flags looked at by threads
					 * that don't own the Stream head. */
	int		sth_open_count;	/* number of simultaneous open */
	int             sth_open_ev;    /* open event */
};

typedef	struct sth_s	STH;
typedef struct sth_s *	STHP;
typedef struct sth_s **	STHPP;

/*
 * Values for sth_read_mode: see stropts.h
 * {RNORM, RFILL, RMSGD, RMSGN, ...}
 */

/*
 * Values for sth_flags
 *
 * F_STH_READ_ERROR	- M_ERROR with read error received, fail all read calls
 * F_STH_WRITE_ERROR	- M_ERROR with write error received, fail all writes
 * F_STH_HANGUP		- M_HANGUP received, no more data
 * F_STH_NDELON		- do TTY semantics for ONDELAY handling
 * F_STH_ISATTY		- this stream acts a terminal
 * F_STH_MREADON	- generate M_READ messages
 * F_STH_TOSTOP		- disallow background writes (for job control)
 * F_STH_PIPE		- stream is one end of a pipe or fifo
 * F_STH_WPIPE		- stream is the "write" side of a pipe
 * F_STH_FIFO		- stream is a fifo
 * F_STH_LINKED		- stream has one or more lower streams linked
 *
 * F_STH_CLOSED		- stream has been closed, and should be freed
 * F_STH_CLOSING	- actively on the way down
 * F_STH_MSDRAIN	- ask to send a M_PCRSE to ptydd when data drained
 *
 * RWHL_ERROR_FLAGS	- error flags which prevent input and output ops
 * RWL_ERROR_FLAGS	- combination of flags which prevent output ops
 * RL_ERROR_FLAGS	- combination of flags which prevent input ops
 * WHL_ERROR_FLAGS	- combination of flags which prevent output ops
 */
#define F_STH_READ_ERROR	0x0001
#define	F_STH_WRITE_ERROR	0x0002
#define	F_STH_HANGUP		0x0004 
#define F_STH_NDELON		0x0008
#define	F_STH_ISATTY		0x0010
#define F_STH_MREADON           0x0020
#define	F_STH_TOSTOP		0x0040
#define	F_STH_PIPE		0x0080
#define	F_STH_WPIPE		0x0100
#define	F_STH_FIFO		0x0200
#define	F_STH_LINKED		0x0400
#define F_STH_CTTY              0x0800

#define	F_STH_CLOSED		0x4000
#define	F_STH_CLOSING		0x8000
#define	F_STH_MSDRAIN		0x10000

#define	MSDRAIN			0x0800	/* another hack for ptys */

#define	RWHL_ERROR_FLAGS	(F_STH_READ_ERROR | F_STH_WRITE_ERROR \
				| F_STH_HANGUP | F_STH_LINKED | F_STH_CLOSED)
#define	RWL_ERROR_FLAGS		(F_STH_READ_ERROR | F_STH_WRITE_ERROR \
				| F_STH_LINKED | F_STH_CLOSED)
#define	RL_ERROR_FLAGS		(F_STH_READ_ERROR | F_STH_LINKED | F_STH_CLOSED)
#define	WHL_ERROR_FLAGS		(F_STH_WRITE_ERROR | F_STH_HANGUP \
				| F_STH_LINKED | F_STH_CLOSED)
/*
 * Values for sth_ext_flags.  These are flags which must sometimes be
 * inspected in a context where the Stream head has not been acquired.
 * Atomic primitives must be used to manipulate these flags.
 *
 * F_STH_OSR_INUSE	- per stream OSR is in use
 *
 */
#define	F_STH_OSR_INUSE		0x0001
#define	F_STH_POLL_INUSE	0x0002
#define	F_STH_PUSHING		0x0004
#define F_STH_PUSHED            0x0008

#define	sth_id(sth)		(((ulong)(sth)) & ~LONG_SIGN_BIT)

#define	STREAM_END(q)		(!(q)->q_next \
				|| ((q)->q_next->q_flag & QREADR))

/* Stream head qinfo structure */
extern	struct streamtab	sthinfo;

/* Maximum number of qbands any queue can have */
#define	MAX_QBAND		255

/* Minimum number of seconds close can wait for a stream to drain */
#define	MIN_CLOSE_WAIT_TIMEOUT	0

/* Minimum number of bytes allocated for a M_PROTO block */
#define	MIN_CTL_BUF		64

#define	READ_MODE_MASK		(RNORM | RMSGD | RMSGN | RFILL)
#define	READ_PROTO_MASK		(RPROTNORM | RPROTDAT | RPROTDIS | RPROTCOMPRESS)
#define	READ_OPTIONS_MASK	(READ_MODE_MASK | READ_PROTO_MASK)

/*
 * module and device switches
 *
 * d_sqh	pointer to synchronization queue for module-wide
 *			synchronization. Also, in the case of external
 *			synchronization, this element points to it.
 * d_str	pointer to streamtab
 * d_funnel_str pointer to true streamtab, when d_str point to funnelized one
 * d_sq_level	synchronization level
 * d_name	name
 * d_flags	flags derived from configuration parameters
 * d_refcnt	non-0 if opened and/or pushed
 * d_major	major number of this device if device
 */

struct modsw {
	struct modsw *	d_next;
	struct modsw *	d_prev;
	char		d_name[FMNAMESZ+1];
	char		d_flags;
	SQHP		d_sqh;
	struct streamtab * d_str;
	struct streamtab * d_funnel_str;
	int		d_sq_level;
	int		d_refcnt;
	int		d_major;
};

/* Supports old-style (V.3) open/close parameters */
#define	F_MODSW_OLD_OPEN	0x1
/* Module requires safe timeout/bufcall callbacks - XXX */
#define	F_MODSW_QSAFETY		0x2
/* non MP Safe drivers need funneling */
#define F_MODSW_MPSAFE		0x4
/* non safe service routines need scheduling */
#define F_MODSW_NOTTOSPEC	0x8

struct file_cookie {
	int		fc_magic;
	struct file *	fc_fp;
	int		fc_flags;
};

#define FILE_COOKIE_MAGIC	0x6e616773

struct mh_s {
	mblk_t	mh_mblk;
	dblk_t	mh_dblk;
	frtn_t	mh_frtn;
};

typedef	struct mh_s	MH;
typedef	struct mh_s *	MHP;

struct open_args {
	int		(*a_func)();	/* casted to V3/V4 signature */
	queue_t *	a_queue;
	dev_t *		a_devp;
	int		a_fflag;
	int		a_sflag;
	cred_t *	a_creds;
};

/*
 * Inlined synch operations.
 */

#define	sqh_init(sqh)	do {				\
	(sqh)->sqh_next = (sqh)->sqh_prev = (SQP)(sqh);	\
	(sqh)->sqh_flags = SQ_IS_HEAD;			\
	lock_alloc((&(sqh)->sqh_lock), LOCK_ALLOC_PIN, PSE_SQH_LOCK, sqh_lock_count++);\
	simple_lock_init(&(sqh)->sqh_lock);		\
	(sqh)->sqh_parent = (sqh);			\
	(sqh)->sqh_owner = nilp(SQ);			\
	(sqh)->sqh_refcnt = 0;				\
} while (0)

#define sqh_term(sqh)  lock_free(&(sqh)->sqh_lock)

#define	sq_init(sq)	do {				\
	bzero((sq), sizeof *(sq));			\
/* new field to satisfy sleep routines interface */     \
	(sq)->sq_sleeper = EVENT_NULL;			\
	/* Other fields set later */			\
} while (0)

#define	csq_acquire(sqh, sq) do {			\
	SQHP _psqh = (sqh)->sqh_parent;			\
	DISABLE_LOCK_DECL				\
	LOCK_QUEUE(_psqh);				\
	if (!(_psqh->sqh_flags & SQ_INUSE)) {		\
		_psqh->sqh_flags |= SQ_INUSE;		\
		_psqh->sqh_owner = (sq);		\
		_psqh->sqh_refcnt = 1;			\
		UNLOCK_QUEUE(_psqh);			\
	} else {					\
		UNLOCK_QUEUE(_psqh);			\
		if (_psqh->sqh_owner == (sq))		\
			_psqh->sqh_refcnt++;		\
		else					\
			_csq_acquire((sqh), (sq));	\
	}						\
} while (0)

#if	MACH_ASSERT

extern const char csqrel1[], csqrel2[];

#define	csq_release(sqh) do {				\
	if (--(sqh)->sqh_parent->sqh_refcnt <= 0) {	\
		SQHP _psqh = (sqh)->sqh_parent;		\
		DISABLE_LOCK_DECL			\
		if (_psqh->sqh_refcnt < 0)		\
			panic(csqrel1);			\
		LOCK_QUEUE(_psqh);			\
		if (_psqh->sqh_owner == 0 		\
		||  !(_psqh->sqh_flags & SQ_INUSE))	\
			panic(csqrel2);			\
		_psqh->sqh_flags &= ~SQ_INUSE;		\
		_psqh->sqh_owner = 0;			\
		UNLOCK_QUEUE(_psqh);			\
		if (_psqh->sqh_next != (SQP)_psqh)	\
			csq_turnover(_psqh);		\
	}						\
} while (0)

#else

#define	csq_release(sqh) do {				\
	if (--(sqh)->sqh_parent->sqh_refcnt <= 0) {	\
		SQHP _psqh = (sqh)->sqh_parent;		\
		DISABLE_LOCK_DECL			\
		LOCK_QUEUE(_psqh);			\
		_psqh->sqh_flags &= ~SQ_INUSE;		\
		_psqh->sqh_owner = 0;			\
		UNLOCK_QUEUE(_psqh);			\
		if (_psqh->sqh_next != (SQP)_psqh)	\
			csq_turnover(_psqh);		\
	}						\
} while (0)

#endif

#endif	/* _STR_STREAM_H */
