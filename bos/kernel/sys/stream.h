/* @(#)38       1.7.2.17  src/bos/kernel/sys/stream.h, sysxpse, bos41J, 9521B_all 5/25/95 15:33:20 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
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

#ifndef	_SYS_STREAM_H
#define	_SYS_STREAM_H

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/cred.h>

#ifdef	_KERNEL
#include <sys/malloc.h>
#include <sys/conf.h>
#include <stdarg.h>

/*
 *	Resolution of name conflicts...
 *
 *	The method is to redefine names with "pse_"+"name".
 *	We can neither change the AT&T STREAMS spec, nor do we want to
 *	change the OS environment. So we have to intercept those occurrences
 *	using the C preprocessor. Because of this, it is imperative that
 *	STREAMS header files be included LAST in files that care.
 *	See function name resolution at end of this file.
 */

#undef	queue
#define queue   	pse_queue

#undef	queue_t
#define queue_t 	pse_queue_t

#endif	/* _KERNEL */

/* defines module or driver */
struct streamtab {
	struct qinit	* st_rdinit;	/* defines read QUEUE */
	struct qinit	* st_wrinit;	/* defines write QUEUE */
	struct qinit	* st_muxrinit;	/* for multiplexing drivers only */
	struct qinit	* st_muxwinit;	/* ditto */
};

#ifndef MSG_KERNEL_FIELDS
#define MSG_KERNEL_FIELDS
#endif

/* message block */
struct	msgb {
	struct msgb *	b_next;		/* next message on queue */
	struct msgb *	b_prev;		/* previous message on queue */
	struct msgb *	b_cont;		/* next message block of message */
	unsigned char *	b_rptr;		/* first unread data byte in buffer */
	unsigned char *	b_wptr;		/* first unwritten data byte */
	struct datab *	b_datap;	/* data block */
	unsigned char	b_band;		/* message priority */
	unsigned char	b_pad1;
	unsigned short	b_flag;		/* message flags */
	long		b_pad2;
	MSG_KERNEL_FIELDS
};
typedef	struct msgb	mblk_t;

/* mblk flags */
#define	MSGMARK		0x01	/* last byte of message is tagged */
#define	MSGNOLOOP	0x02	/* don't pass message to write-side of stream */
#define	MSGDELIM	0x04	/* message is delimited */
#define	MSGCOMPRESS	0x0100	/* compress like messages as space allows */
#define	MSGNOTIFY	0x0200	/* notify when message consumed */
#define	MSGPKT		0x0400	/* hack for ptys */

/* data descriptor */
struct	datab {
	union {
		struct datab	* freep;
		struct free_rtn	* frtnp;
	} db_f;
	unsigned char *	db_base;	/* first byte of buffer */
	unsigned char *	db_lim;		/* last byte+1 of buffer */
	unsigned char	db_ref;		/* count of messages pointing to block*/
	unsigned char	db_type;	/* message type */
	unsigned char	db_iswhat;	/* message status */
	unsigned int	db_size;	/* used internally */
	caddr_t		db_msgaddr;	/* used internally */
	long		db_filler;
};
#define	db_freep	db_f.freep
#define	db_frtnp	db_f.frtnp

typedef	struct datab	dblk_t;

/* Free return structure for esballoc */
typedef struct free_rtn {
	void	(*free_func)(char *, char *);	/* Routine to free buffer */
	char *	free_arg;			/* Parameter to free_func */
} frtn_t;

#ifndef	QUEUE_KERNEL_FIELDS
#define QUEUE_KERNEL_FIELDS
#endif

struct queue {
	struct qinit *	q_qinfo;	/* procedures and limits for queue */
	struct msgb *	q_first;	/* head of message queue */
	struct msgb *	q_last;		/* tail of message queue */
	struct queue *	q_next;		/* next QUEUE in Stream */
	struct queue *	q_link;		/* link to scheduling queue */
	caddr_t		q_ptr;		/* to private data structure */
	ulong		q_count;	/* weighted count of characters on q */
	ulong		q_flag;		/* QUEUE state */
	long		q_minpsz;	/* min packet size accepted */
	long		q_maxpsz;	/* max packet size accepted */
	ulong		q_hiwat;	/* high water mark, for flow control */
	ulong		q_lowat;	/* low water mark */
	struct qband *	q_bandp;	/* band information */
	unsigned char	q_nband;	/* number of bands */
	unsigned char	q_pad1[3];	/* reserved */
	struct queue *	q_other;	/* pointer to other Q in queue pair */
	QUEUE_KERNEL_FIELDS
};

typedef	struct	queue	queue_t;

/* queue_t flag defines */
#define	QREADR			0x1	/* This queue is a read queue */
#define	QNOENB			0x2	/* Don't enable in putq */
#define	QFULL			0x4	/* The queue is full */
#define	QWANTR			0x8	/* The queue should be scheduled
					   in the next putq */
#define	QWANTW			0x10	/* The stream should be back enabled
					   when this queue drains */
#define	QUSE			0x20	/* The queue is allocated and ready
					   for use */
#define	QENAB			0x40	/* The queue is scheduled (on the
					   run queue) */
#define	QBACK			0x80	/* The queue has been back enabled */
#define	QOLD			0x100	/* Module supports old style opens
					   and closes */
#define	QHLIST			0x200	/* The Stream head is doing something
					   with queue (not used by OSF/1) */
#define	QFUN			0x400	/* The queue is funneled */
#define QSAFE			0x02000	/* Flag for "safe" callbacks needed */
#define	QWELDED			0x04000	/* Flag for welded queues */

#define	QNOTTOSPEC		0x08000 /* Service routines not coded to spec.
					Must be scheduled to run. */ 

struct qband {
	struct qband *	qb_next;	/* next band for this queue */
	ulong		qb_count;	/* weighted character count in band */ 
	struct msgb *	qb_first;	/* head of message queue */
	struct msgb *	qb_last;	/* tail of message queue */
	ulong		qb_hiwat;	/* high water mark */
	ulong		qb_lowat;	/* low water mark */
	ulong		qb_flag;	/* state */
	long		qb_pad1;	/* reserved */
};

typedef struct qband	qband_t;

typedef struct ucred    cred_t;
#define NODEV   (dev_t)(-1)

/* qband_t flag defines */
#define	QB_FULL		0x1	/* The band is full */
#define	QB_WANTW	0x2	/* Back enable when this queue/band drains */
#define	QB_BACK		0x4	/* The queue has been back enabled */

/* Cast to these typedefs when qi_qopen|qi_qclose called. */
typedef	int	(*stropen_V3)(queue_t *, dev_t, int, int);
typedef	int	(*strclose_V3)(queue_t *);
typedef	int	(*stropen_V4)(queue_t *, dev_t *, int, int, cred_t *);
typedef	int	(*strclose_V4)(queue_t *, int, cred_t *);

struct	qinit {
	int	(*qi_putp)(queue_t *, mblk_t *);
	int	(*qi_srvp)(queue_t *);
	int	(*qi_qopen)();
	int	(*qi_qclose)();
	int	(*qi_qadmin)(void);
	struct module_info * qi_minfo;
	struct module_stat * qi_mstat;
};

struct	module_info {
	unsigned short	mi_idnum;	/* module ID number */
	char * 		mi_idname;	/* module name */
	long		mi_minpsz;	/* min packet size, for developer use */
	long		mi_maxpsz;	/* max packet size, for developer use */
	ulong		mi_hiwat;	/* hi-water mark, for flow control */
	ulong		mi_lowat;	/* lo-water mark, for flow control */
};

/* Used in M_IOCTL mblks to muxes (ioc_cmd I_LINK) */
struct	linkblk {
	queue_t	* l_qtop;	/* lowest level write queue of upper stream */
	queue_t	* l_qbot;	/* highest level write queue of lower stream */
	int	l_index;	/* system-unique index for lower stream */
	long	l_pad[5];
};

/*
 * Structure of M_IOCTL, M_COPYIN, M_COPYOUT and M_IOCDATA messages
 *
 * Note that these structures must be interchangeable!
 * That means: sizes must be identical, and the first fields,
 * as well as the final private pointer must match.
 */

typedef union {
	long		l;
	ulong		ul;
	caddr_t		cp;
	mblk_t *	mp;
} ioc_pad;

struct	iocblk {
	int		ioc_cmd;	/* ioctl command type */
	cred_t *	ioc_cr;		/* pointer to full credentials */
	uint		ioc_id;		/* ioctl id */
	ioc_pad		ioc_cnt;	/* count of bytes in data field */
	int		ioc_error;	/* error code */
	int		ioc_rval;	/* return value */
	mblk_t *	ioc_private;	/* module's private state info */
	long		ioc_filler[4];
};
#define	ioc_count	ioc_cnt.ul
#define	ioc_uid		ioc_cr->cr_uid
#define	ioc_gid		ioc_cr->cr_gid

struct copyreq {
	int		cq_cmd;		/* command type == ioc_cmd */
	cred_t *	cq_cr;		/* pointer to full credentials */
	uint		cq_id;		/* ioctl id == ioc_id */
	ioc_pad		cq_ad;		/* address to copy data to/from */
	uint		cq_size;	/* number of bytes to copy */
	int		cq_flag;	/* reserved */
	mblk_t *	cq_private;	/* module's private state info */
	long		cq_filler[4];
};
#define	cq_addr	cq_ad.cp
#define	cq_uid	cq_cr->cr_uid
#define	cq_gid	cq_cr->cr_gid

/* structure contained in M_IOCDATA message block */
struct copyresp {
	int		cp_cmd;		/* command type == ioc_cmd */
	cred_t	*	cp_cr;		/* pointer to full credentials */
	uint		cp_id;		/* ioctl id == ioc_id */
	ioc_pad		cp_rv;		/* 0 = success */
	uint		cp_pad1;	/* reserved */
	int		cp_pad2;	/* reserved */
	mblk_t *	cp_private;	/* module's private state info */
	long		cp_filler[4];
};
#define	cp_rval	cp_rv.l
#define	cp_uid	cp_cr->cr_uid
#define	cp_gid	cp_cr->cr_gid

#define TRANSPARENT	(-32768)	/* special value for ioc_count! */

/* Message types */
#define	QNORM		0
#define	M_DATA		0x00	/* Ordinary data */
#define	M_PROTO		0x01	/* Internal control info and data */
#define	M_BREAK		0x08	/* Request a driver to send a break */
#define	M_PASSFP	0x09	/* Used to pass a file pointer */
#define	M_SIG		0x0b	/* Requests a signal to be sent */
#define	M_DELAY		0x0c	/* Request a real-time delay */
#define	M_CTL		0x0d	/* For inter-module communication */
#define	M_IOCTL		0x0e	/* Used internally for I_STR requests */
#define	M_SETOPTS	0x10	/* Alters characteristics of Stream head */
#define	M_RSE		0x11	/* Reserved for internal use */

/* Priority messages types */
#define	QPCTL		0x80
#define	M_IOCACK	0x81	/* Positive ack of previous M_IOCTL */
#define	M_IOCNAK	0x82	/* Previous M_IOCTL failed */
#define	M_PCPROTO	0x83	/* Same as M_PROTO except for priority*/
#define	M_PCSIG		0x84	/* Priority signal */
#define	M_READ          0x85	/* Reports client read at stream head */
#define	M_FLUSH		0x86	/* Requests modules to flush queues */
#define	M_STOP		0x87	/* Request drivers to stop output */
#define	M_START		0x88	/* Request drivers to start output */
#define	M_HANGUP	0x89	/* Driver can no longer produce data */
#define	M_ERROR		0x8a	/* Reports downstream error condition */
#define M_COPYIN	0x8b	/* Module's request to perform copyin */
#define M_COPYOUT	0x8c	/* Module's request to perform copyout */
#define M_IOCDATA	0x8d	/* Response to M_COPYIN, M_COPYOUT */
#define	M_PCRSE		0x8e	/* Reserved for internal use */
#define	M_STOPI		0x8f	/* Request drivers to stop input */
#define	M_STARTI	0x90	/* Request drivers to start input */
#define	M_HPDATA        0x93	/* OSF-private: high priority data */
/* AIX SPECIFIC follow */
#define	M_NOTIFY	0x94	/* Reports result of read at stream head */
#define M_LETSPLAY      0x95  /* Sent up by driver */
#define M_DONTPLAY    0x96    /* Sent down by stream head */
#define M_BACKWASH    0x97    /* Sent down by stream head and modules */
#define M_BACKDONE    0x98    /* End of M_BACKWASH messages */


#define	FLUSHALL	0x0001
#define	FLUSHDATA	0x0000

#define	NOERROR		(uchar)-1	/* used in M_ERROR messages */

/* structure contained in an M_SETOPTS message block */
struct	stroptions {
	ulong		so_flags;	/* options to set */
	short		so_readopt;	/* read option */
	ushort		so_wroff;	/* write offset */
	long		so_minpsz;	/* minimum read packet size */
	long		so_maxpsz;	/* maximum read packet size */
	ulong		so_hiwat;	/* read queue high-water mark */
	ulong		so_lowat;	/* read queue low-water mark */
	unsigned char	so_band;	/* band for water marks */
};

/* definitions for so_flags field */
#define	SO_ALL		(~0)	/* Update all options */
#define	SO_READOPT	0x0001	/* Set the read mode */
#define	SO_WROFF	0x0002	/* Insert an offset in write M_DATA mblks */
#define	SO_MINPSZ	0x0004	/* Change the min packet size on sth rq */
#define	SO_MAXPSZ	0x0008	/* Change the max packet size on sth rq */
#define	SO_HIWAT	0x0010	/* Change the high water mark on sth rq */
#define	SO_LOWAT	0x0020	/* Change the low water mark */
#define	SO_MREADON      0x0040  /* Request M_READ messages */
#define	SO_MREADOFF     0x0080  /* Don't gen M_READ messages */
#define	SO_ISTTY	0x0100	/* Act as controlling tty */
#define SO_ISNTTY	0x0200	/* Don't act as controlling tty */
#define SO_NDELON	0x0400	/* Apply TTY semantics on ONDELAY */
#define SO_NDELOFF	0x0800	/* Apply STREAMS semantics on ONDELAY */
#define	SO_TOSTOP	0x1000	/* Stop on background writes */
#define	SO_TONSTOP	0x2000	/* Don't stop on background writes */
#define	SO_BAND		0x4000	/* Water marks are for a band */
 
/* Buffer Allocation Priority */
#define	BPRI_LO		1
#define	BPRI_MED	2
#define	BPRI_HI		3
#define	BPRI_WAITOK	255

/** Test whether message is a data message */
#define	datamsg(type)	((type) == M_DATA || (type) == M_PROTO || \
			 (type) == M_PCPROTO || (type) == M_DELAY)

/** Get pointer to the mate queue */
#define	OTHERQ(q)	((q)->q_other)

/** Get pointer to the read queue, assumes 'q' is a write queue ptr */
#define	RD(q)		OTHERQ(q)

/** Get pointer to the write queue, assumes 'q' is a read queue ptr */
#define	WR(q)		OTHERQ(q)

#define	OPENFAIL	(-1)
#define	CLONEOPEN	0x2
#define	MODOPEN		0x1

#define	NSTRPUSH	9
#define	STRMSGSZ	0
#define	STRCTLSZ	1024

/* Unused - compat for V.4/V3.2 spec */
#define STRLOFRAC	80
#define	STRMEDFRAC	90
#define	STRTHRESH	0

/* "Infinity" for messages */
#define	INFPSZ		(-1)

/* Maximum length of module name. */
#define FMNAMESZ	8

/* Enumeration values for strqget and strqset */
typedef enum qfields {
	QHIWAT	= 0,
	QLOWAT	= 1,
	QMAXPSZ	= 2,
	QMINPSZ	= 3,
	QCOUNT	= 4,
	QFIRST	= 5,
	QLAST	= 6,
	QFLAG	= 7,
	QBAD	= 8
} qfields_t;

struct wantiosw {
	int (*w_read)(queue_t *, struct uio *, int);
	int (*w_write)(queue_t *, struct uio *, int);
	int (*w_select)(queue_t *, ushort, ushort *, void *);
	int (*w_reserved1)(); /* Future use. */
	int (*w_reserved2)(); /* Future use. */
	int (*w_reserved3)(); /* Future use. */
};
struct wantio {
	Simple_lock w_lock;
	int w_flags;
	struct wantiosw	*wantiosw;
};
 
/* w_flags bits */
#define W_SIGIO   0x0001
#define W_LOCKME  0x0002 /* Not implemented yet. Wantio modules responsible
			  * for serialization. */

/* The V.4 Programmer's guide says these are available...  */
#ifndef max
#define max(x1,x2)	((x1) >= (x2) ? (x1) : (x2))
#endif
#ifndef min
#define min(x1,x2)	((x1) <= (x2) ? (x1) : (x2))
#endif

#ifdef	_KERNEL

/* svr4 compatible macros */
#define getmajor        major
#define getminor        minor
#define getemajor       major
#define geteminor       minor
#define makedevice      makedev

/*
 * Name resolution continued from above.
 */

/* The V.4 guide has a non-portable and wrong prototype for bufcall. */
#undef	bufcall
typedef	void *		bufcall_arg_t;
typedef	void		(*bufcall_fcn_t)(bufcall_arg_t);
extern	int		pse_bufcall(uint, int, bufcall_fcn_t,bufcall_arg_t);
#define	bufcall(a,b,c,d)	\
	pse_bufcall((a), (b), (bufcall_fcn_t)(c), (bufcall_arg_t)(d))

#undef	timeout
typedef	caddr_t		timeout_arg_t;
typedef	void		(*timeout_fcn_t)(timeout_arg_t);
extern	int		pse_timeout(timeout_fcn_t, timeout_arg_t, int);
#define timeout(a,b,c)	pse_timeout((timeout_fcn_t)(a),(timeout_arg_t)(b),c)

#undef	untimeout
extern	void		pse_untimeout(int);
#define untimeout	pse_untimeout

#undef timeoutcf
extern	int		pse_timeout_cf(int);
#define timeoutcf	pse_timeout_cf

#undef  sleepx
#define sleepx(a, b, c) pse_sleepx((a),(b),(c))

#undef  e_sleep
#define e_sleep(a, b)   pse_sleep((a),(b))

#undef  e_sleepl
#define e_sleepl(a, b, c)	pse_sleepl((a),(b),(c))

#undef  e_sleep_thread
#define e_sleep_thread(a, b, c)	pse_sleep_thread((a),(b),(c))

#undef  e_block_thread
#define e_block_thread()	pse_block_thread()

extern  int             pse_sleep(int *, int);
extern  int             pse_sleepl(int *, int *, int);
extern  int             pse_sleepx(caddr_t, int, int);
extern  int             pse_sleep_thread(int *, void *, int);
extern	int		pse_block_thread();

/*
 * Other Streams utilities.
 */
extern	int		adjmsg(mblk_t *, int);
extern	mblk_t *	allocb(int, uint);
extern	mblk_t *	allocbi(int, int, void (*)(char *, char *), char *, uchar *);
extern	queue_t *	backq(queue_t *);
extern	int		bcanput(queue_t *, unsigned char);
extern	int		canenable(queue_t *);
extern	int		canput(queue_t *);
extern	mblk_t *	copyb(mblk_t *);
extern	mblk_t *	copymsg(mblk_t *);
extern	mblk_t *	dupb(mblk_t *);
extern	mblk_t *	dupmsg(mblk_t *);
extern	void		enableok(queue_t *);
extern	mblk_t *	esballoc(unsigned char *, int, int, frtn_t *);
extern	void		flushband(queue_t *, unsigned char, int);
extern	void		flushq(queue_t *, int);
extern	void		freeb(mblk_t *);
extern	void		freemsg(mblk_t *);
extern	int		(*getadmin(ushort))(void);
extern	ushort		getmid(char *);
extern	mblk_t *	getq(queue_t *);
extern	int		insq(queue_t *, mblk_t *, mblk_t *);
extern	void		linkb(mblk_t *, mblk_t *);
extern	void		mpsprintf(mblk_t *, char *, ...);
extern	int		msgdsize(mblk_t *);
extern	void		noenable(queue_t *);
extern	int		pullupmsg(mblk_t *, int);
extern	int		putbq(queue_t *, mblk_t *);
extern	int		putctl(queue_t *, int);
extern	int		putctl1(queue_t *, int, int);
extern	int		putctl2(queue_t *, int, int, int);
extern	void		puthere(queue_t *, mblk_t *);
extern	void		putnext(queue_t *, mblk_t *);
extern	int		putq(queue_t *, mblk_t *);
extern	int		qenable(queue_t *);
extern	void		qreply(queue_t *, mblk_t *);
extern	int		qsize(queue_t *);
extern	mblk_t *	rmvb(mblk_t *, mblk_t *);
extern	void		rmvq(queue_t *, mblk_t *);
extern	int		strlog(short, short, char, unsigned short, char *, ...);
extern	int		strqget(queue_t *, qfields_t, unsigned char, long *);
extern	int		strqset(queue_t *, qfields_t, unsigned char, long);
extern	int		testb(int, uint);
extern	void		unbufcall(int);
extern	mblk_t *	unlinkb(mblk_t *);
extern  int             wantio(queue_t *q, struct wantio *w);
typedef void *		weld_arg_t;
typedef void		(*weld_fcn_t)(weld_arg_t);
extern	int		weldq(queue_t *, queue_t *, queue_t *, queue_t *,
				weld_fcn_t, weld_arg_t, queue_t *);
extern	int		unweldq(queue_t *, queue_t *, queue_t *, queue_t *,
				weld_fcn_t, weld_arg_t, queue_t *);

#endif	/* _KERNEL */
#endif	/* _SYS_STREAM_H */
