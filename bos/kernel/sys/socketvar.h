/* @(#)74	1.12.2.16  src/bos/kernel/sys/socketvar.h, sockinc, bos41B, 412_41B_sync 11/29/94 13:02:15 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: SOCKBUF_ISLOCKED
 *		SOCKBUF_LOCK
 *		SOCKBUF_UNLOCK
 *		SOCKET_ISLOCKED
 *		SOCKET_LOCK
 *		SOCKET_LOCK2
 *		SOCKET_LOCKINIT
 *		SOCKET_UNLOCK
 *		SOCKET_UNLOCK2
 *		SOHASUAREA
 *		sballoc
 *		sbfree
 *		sbspace
 *		sodomain
 *		sonewconn
 *		soreadable
 *		sorwakeup
 *		sosendallatonce
 *		sowatomic
 *		sowriteable
 *		sowwakeup
 *		
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/* socketvar.h	2.1 16:08:08 4/20/90 SecureWare */
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
 * Copyright (c) 1982, 1986, 1990 Regents of the University of California.
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
 *	Base:	socketvar.h	7.6 (Berkeley) 9/4/89
 *	Merged: socketvar.h	7.11 (Berkeley) 6/30/90
 */

#ifndef	_SYS_SOCKETVAR_H_
#define	_SYS_SOCKETVAR_H_

#if	defined(_KERNEL) && !defined(_NET_GLOBALS_H_)
#include "net/net_globals.h"
#endif

#ifdef	_KERNEL
typedef simple_lock_t	sb_lock_t;
#else
typedef int		sb_lock_t;
#endif

#if	MACH
#include <sys/secdefines.h>
#if	SEC_ARCH
#include <sys/security.h>
#endif
#endif

/*
 * Kernel structure per socket.
 * Contains send and receive buffer queues,
 * handle on protocol and pointer to protocol
 * private data and error information.
 */
struct socket {
	short	so_type;		/* generic type, see socket.h */
	short	so_options;		/* from socket call, see socket.h */
	short	so_linger;		/* time to linger while closing */
	short	so_state;		/* internal state flags SS_*, below */
	caddr_t	so_pcb;			/* protocol control block */
	struct	protosw *so_proto;	/* protocol handle */
	struct	socklocks *so_lock;	/* socket structure lock(s) */
/*
 * Variables for connection queueing.
 * Socket where accepts occur is so_head in all subsidiary sockets.
 * If so_head is 0, socket is not related to an accept.
 * For head socket so_q0 queues partially completed connections,
 * while so_q is a queue of connections ready to be accepted.
 * If a connection is aborted and it has so_head set, then
 * it has to be pulled out of either so_q0 or so_q.
 * We allow connections to queue up based on current queue lengths
 * and limit on number of queued connections for this socket.
 */
	struct	socket *so_head;	/* back pointer to accept socket */
	struct	socket *so_q0;		/* queue of partial connections */
	struct	socket *so_q;		/* queue of incoming connections */
	struct	socket *so_dq;		/* queue of defunct connections */
	short	so_q0len;		/* partials on so_q0 */
	short	so_qlen;		/* number of connections on so_q */
	short	so_qlimit;		/* max number queued connections */
	short	so_dqlen;		/* listener dequeues in progress */
	short	so_timeo;		/* connection timeout */
	u_short	so_error;		/* error affecting connection */
	short	so_special;		/* special state flags SP_*, below */
	short	so_spare;		/* available: pad for pid_t */
	pid_t	so_pgid;		/* pgid for signals */
	u_long	so_oobmark;		/* chars to oob mark */
/*
 * Variables for socket buffering.
 */
	struct	sockbuf {
		u_long	sb_cc;		/* actual chars in buffer */
		u_long	sb_hiwat;	/* max actual char count */
		u_long	sb_mbcnt;	/* chars of mbufs used */
		u_long	sb_mbmax;	/* max chars of mbufs to use */
		long	sb_lowat;	/* low water mark */
		struct	mbuf *sb_mb;	/* the mbuf chain */
		union {			/* process selecting read/write */
			struct proc *sb_selproc;	/* UNIX */
			struct sbselque {		/* MACH */
				struct sbselque *next, *prev;
			} sb_selque;
		} sb_select;
                ushort  sb_reqevents;   /* current pending events */
                int     (*sb_iodone)(); /* I/O done function */
                caddr_t sb_ioarg;       /* arg for sb_iodone */
		struct  mbuf *sb_lastpkt; /* last packet of mbuf chain */
		int 	sb_wakeone;	/* SB_WAKEONE sleep event */
		caddr_t	sb_timer;	/* pre-allocated trb */
		short	sb_flags;	/* flags, see below */
		ushort	sb_timeo;	/* timeout for read/write */
#ifdef	_KERNEL
		void	(*sb_wakeup)(caddr_t, int);
#else
		void	(*sb_wakeup)();	/* upcall instead of sowakeup */
#endif
		caddr_t	sb_wakearg;	/* (*sb_wakeup)(sb_wakearg, state) */
		sb_lock_t sb_lock;	/* sockbuf lock (in socklocks) */
	} so_rcv, so_snd;
#define sb_sel		sb_select.sb_selproc
#define sb_selq		sb_select.sb_selque
#define	SB_MAX		(64*1024)	/* default for max chars in sockbuf */
#define	SB_LOCK		0x01		/* lock on data queue */
#define	SB_WANT		0x02		/* someone is waiting to lock */
#define	SB_WAIT		0x04		/* someone is waiting for data/space */
#define	SB_SEL		0x08		/* someone is selecting */
#define	SB_ASYNC	0x10		/* ASYNC I/O, need signals */
#define	SB_COLL		0x20		/* collision selecting (UNIX) */
#define	SB_NOINTR	0x40		/* operations not interruptible */
#define	SB_WAKEONE	0x80		/* wakeup only one on notify */
#define SB_WAITING      0x100           /* MSG_WAITALL receive in progress */
#define	SB_INHERIT	(SB_NOINTR|SB_WAKEONE)
#ifdef  _AIX
#define SB_KIODONE	0x400		/* socket kernel iodone */
#define SB_NOTIFY	(SB_WAIT|SB_SEL|SB_ASYNC|SB_WAKEONE|SB_KIODONE)
#else /* _AIX */
#define SB_NOTIFY	(SB_WAIT|SB_SEL|SB_ASYNC|SB_WAKEONE)
#endif  /* _AIX */
#define SB_NOSELECT	0x800

	caddr_t	so_tpcb;		/* Wisc. protocol control block XXX */
};

/*
 * Socket state bits.
 */
#define	SS_NOFDREF		0x001	/* no file table ref any more */
#define	SS_ISCONNECTED		0x002	/* socket connected to a peer */
#define	SS_ISCONNECTING		0x004	/* in process of connecting to peer */
#define	SS_ISDISCONNECTING	0x008	/* in process of disconnecting */
#define	SS_CANTSENDMORE		0x010	/* can't send more data to peer */
#define	SS_CANTRCVMORE		0x020	/* can't receive more data from peer */
#define	SS_RCVATMARK		0x040	/* at mark on input */

#define	SS_PRIV			0x080	/* privileged for broadcast, raw... */
#define	SS_NBIO			0x100	/* non-blocking ops */
#define	SS_ASYNC		0x200	/* async i/o notify */
#define	SS_ISCONFIRMING		0x400	/* deciding to accept connection req */


/*
 * Special socket state bits.
 */
#define SP_PIPE			0x0001	/* socket is (unnamed) pipe */
#define SP_WATOMIC		0x0002	/* pipe write atomicity */
#define SP_NOUAREA		0x0004	/* no u-area available (XTI - XXX) */
#define SP_LOCKABLE		0x0008	/* socket of parallel domain */
#define SP_CLOSING		0x0010	/* closing a listening socket */
#define SP_RWAKEUP		0x0020	/* sorwakeup pending release of lock */
#define SP_WWAKEUP		0x0040	/* sowwakeup pending release of lock */
#define SP_EXTPRIV              0x0080  /* SS_PRIV managed externally */
#define	SP_DISABLE		0x0800	/* use disable_lock for serialization */
#define SP_FREEABLE		0x8000	/* free socket on unlock */
#define SP_INHERIT		(SP_PIPE|SP_WATOMIC|SP_NOUAREA|SP_LOCKABLE|SP_DISABLE)

#ifdef	_KERNEL

/*
 * Macros for sockets, socket locking and socket buffering.
 */

struct  socklocks {
	simple_lock_data_t	sock_lock;
	simple_lock_data_t	snd_lock;
	simple_lock_data_t	rcv_lock;
	int			refcnt;
	struct socket 		*sp_wake;
	struct socket 		*so;		/* back pointer */
	int			spl;
	struct socklocks	*freelist;
};

#define SOCKET_LOCKINIT(so, lp)  { \
	(so)->so_lock = (lp); \
	(so)->so_snd.sb_lock = &((lp)->snd_lock); \
	(so)->so_rcv.sb_lock = &((lp)->rcv_lock); \
	(lp)->refcnt = 0; \
	(lp)->sp_wake = 0; \
	(lp)->so = so; \
	(lp)->freelist = 0; \
	lock_alloc(&((lp)->sock_lock), LOCK_ALLOC_PIN, SOCKET_LOCK_FAMILY, so);\
	simple_lock_init(&((lp)->sock_lock));  \
}

/* The socket is considered locked for assertions if not lockable */
#define SOCKET_ISLOCKED(so)  1

/* Lock order of unpaired sockets is critical to avoid deadlock. */
#define SOCKET_LOCK2(so1, so2) { \
	if ((so1) < (so2)) { \
		SOCKET_LOCK(so1); \
		if ((so1)->so_lock != (so2)->so_lock) \
			SOCKET_LOCK(so2); \
	} else { \
		SOCKET_LOCK(so2); \
		if ((so1)->so_lock != (so2)->so_lock) \
			SOCKET_LOCK(so1); \
	} \
}
#define SOCKET_UNLOCK2(so1, so2) { \
	if ((so1) < (so2)) { \
		if ((so1)->so_lock != (so2)->so_lock) \
			SOCKET_UNLOCK(so2); \
		SOCKET_UNLOCK(so1); \
	} else { \
		if ((so1)->so_lock != (so2)->so_lock) \
			SOCKET_UNLOCK(so1); \
		SOCKET_UNLOCK(so2); \
	} \
}

#define SOCKET_LOCK(so)		{	\
if ((so)->so_special & SP_DISABLE)	\
	(so)->so_lock->spl = disable_lock(PL_IMP, &((so)->so_lock->sock_lock));\
else {	\
	simple_lock_data_t *lw = &((so)->so_lock->sock_lock); \
\
	simple_lock(lw); \
	while (lw != &((so)->so_lock->sock_lock)) { \
		simple_unlock(lw); \
		lw = &((so)->so_lock->sock_lock); \
		simple_lock(lw); \
	}  \
} \
}
#define SOCKET_UNLOCK(so)	(void)sounlock(so)
/* Sockbuf locks are disabled in non-debug kernel */
#define SOCKBUF_LOCKTEST	0
#define SOCKBUF_LOCK(sb)	/*simple_lock((sb)->sb_lock)*/
#define SOCKBUF_UNLOCK(sb)	/*simple_unlock((sb)->sb_lock)*/
#define SOCKBUF_ISLOCKED(sb)	1 /*lock_islocked((sb)->sb_lock)*/

/* Bits for network events to sb_wakeup */
#define SE_ERROR	0x0001	/* so_error non-0 */
#define SE_HAVEDATA	0x0002	/* data in send or recv q */
#define SE_HAVEOOB	0x0004	/* oob data in recv q */
#define SE_DATAFULL	0x0008	/* send or recv q is full */
#define SE_CONNOUT	0x0010	/* outgoing connect complete (connect) */
#define SE_CONNIN	0x0020	/* incoming connect complete (listen)  */
#define SE_SENDCONN	0x0040	/* connected for send */
#define SE_RECVCONN	0x0080	/* connected for recv */
#define SE_POLL		0x4000	/* wakeup is synchronous poll */
#define SE_STATUS	0x8000	/* above status bits valid */

/* does socket have a valid u-area associated? (for stats, security, etc) */
#define SOHASUAREA(so)	(!((so)->so_special & SP_NOUAREA))

/* what is the domain associated with this socket? */
#define sodomain(so)	((so)->so_proto->pr_domain)

/*
 * How much space is there in a socket buffer (so->so_snd or so->so_rcv)?
 * This is problematical if the fields are unsigned, as the space might
 * still be negative (cc > hiwat or mbcnt > mbmax).  Should detect
 * overflow and return 0.  Should use "lmin" but it doesn't exist now.
 */
#ifdef	_AIX_FULLOSF
#define	sbspace(sb) \
    ((long) imin((int)((sb)->sb_hiwat - (sb)->sb_cc), \
	 (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)))
#else	/* _AIX_FULLOSF */
#define	sbspace(sb) \
    (MIN((int)((sb)->sb_hiwat - (sb)->sb_cc), \
	 (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)))
#endif	/* _AIX_FULLOSF */

/* do we have to send all at once on a socket? */
#define	sosendallatonce(so) \
    ((so)->so_proto->pr_flags & PR_ATOMIC)

/*
 * Is write of len bytes to be atomic (for pipes)? Since sb_hiwat
 * is adjusted by uipc_usrreq, it's not usable for this purpose,
 * so we look straight at syslimits.h with the danger of
 * PIPE_BUF > PIPSIZ.
 */
#define sowatomic(so, len) \
    (((so)->so_special & SP_WATOMIC) && ((len) <= PIPE_BUF))

/* can we read something from so? */
#define	soreadable(so) \
    ((so)->so_rcv.sb_cc >= (so)->so_rcv.sb_lowat || \
	((so)->so_state & SS_CANTRCVMORE) || \
	(so)->so_qlen || (so)->so_error)

/* can we write something to so? */
#define	sowriteable(so) \
    (sbspace(&(so)->so_snd) >= (so)->so_snd.sb_lowat && \
	(((so)->so_state&SS_ISCONNECTED) || \
	  ((so)->so_proto->pr_flags&PR_CONNREQUIRED)==0) || \
     ((so)->so_state & SS_CANTSENDMORE) || \
     (so)->so_error)

/* adjust counters in sb reflecting allocation of m */
#define	sballoc(sb, m) { \
	(sb)->sb_cc += (m)->m_len; \
	(sb)->sb_mbcnt += MSIZE; \
	if ((m)->m_flags & M_EXT) \
		(sb)->sb_mbcnt += (m)->m_ext.ext_size; \
}

/* adjust counters in sb reflecting freeing of m */
#define	sbfree(sb, m) { \
	(sb)->sb_cc -= (m)->m_len; \
	(sb)->sb_mbcnt -= MSIZE; \
	if ((m)->m_flags & M_EXT) \
		(sb)->sb_mbcnt -= (m)->m_ext.ext_size; \
}

#define	sorwakeup(so)	sowakeup((so), &(so)->so_rcv)
#define	sowwakeup(so)	sowakeup((so), &(so)->so_snd)

/* to catch callers missing new second argument to sonewconn: */
#define sonewconn(head, connstatus)	sonewsock((head), (connstatus))
extern u_long	sb_max;
#endif	/* _KERNEL */

#endif
