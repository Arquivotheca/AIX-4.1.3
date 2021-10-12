static char sccsid[] = "@(#)96	1.27.2.20  src/bos/kernel/uipc/socket2.c, sysuipc, bos41J, 9513A_all 3/23/95 14:39:56";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: lock_sbcheck
 *		lock_socheck
 *		sbappend
 *		sbappendaddr
 *		sbappendcontrol
 *		sbappendrecord
 *		sbcompress
 *		sbdrop
 *		sbdroprecord
 *		sbflush
 *		sbinsertoob
 *		sbpoll
 *		sbrelease
 *		sbreserve
 *		sbseldequeue
 *		sbselqueue
 *		sbwait
 *		sbwakeup
 *		socantrcvmore
 *		socantsendmore
 *		soisconnected
 *		soisconnecting
 *		soisdisconnected
 *		soisdisconnecting
 *		sonewconn1
 *		sonewsock
 *		soqinsque
 *		soqremque
 *		soreserve
 *		sosbwait
 *		sosleep
 *		sowakeup
 *		unlock_sbcheck
 *		unlock_socheck
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
/* @(#)uipc_socket2.c	2.1 16:10:48 4/20/90 SecureWare */
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
 *	Base:	src/bos/kernel/uipc/socket2.c, sysuipc, bos410, aoot (Berkeley) 7/24/93
 *	Merged: uipc_socket2.c	7.15 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"
#if	_AIX_FULLOSF
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/rtc.h"

#include "net/net_malloc.h"

#if	_AIX_FULLOSF
#include "kern/parallel.h"

#if	SEC_ARCH
#include <sys/security.h>
#endif
#endif

LOCK_ASSERTL_DECL

#ifdef _AIX_FULLOSF
CONST static char pip[] = "pipe", fif[] = "fifo", sock[] = "socket";

#define sowaitmsg(so) \
	(((so)->so_special & SP_PIPE) ? pip : \
	 ((so)->so_special & SP_WATOMIC) ? fif : sock)
#endif /* _AIX_FULLOSF */

/*
 * Primitive routines for operating on sockets and socket buffers
 */

#ifdef	_AIX
u_long	sb_max 		= SB_MAX;	/* patchable via no command */
u_long	sb_max_dflt 	= SB_MAX;	/* patchable via no command */
#define        min     MIN
#define sowaitmsg(a)
#else	/* _AIX */
u_long	sb_max = SB_MAX;		/* patchable */
#endif	/* _AIX */

#ifndef	_AIX_FULLOSF
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/nettrace.h>
#endif	/* _AIX_FULLOSF */
/*
 * Procedures to manipulate state flags of socket
 * and do appropriate wakeups.  Normal sequence from the
 * active (originating) side is that soisconnecting() is
 * called during processing of connect() call,
 * resulting in an eventual call to soisconnected() if/when the
 * connection is established.  When the connection is torn down
 * soisdisconnecting() is called during processing of disconnect() call,
 * and soisdisconnected() is called when the connection to the peer
 * is totally severed.  The semantics of these routines are such that
 * connectionless protocols can call soisconnected() and soisdisconnected()
 * only, bypassing the in-progress calls when setting up a ``connection''
 * takes no time.
 *
 * From the passive side, a socket is created with
 * two queues of sockets: so_q0 for connections in progress
 * and so_q for connections already made and awaiting user acceptance.
 * As a protocol is preparing incoming connections, it creates a socket
 * structure queued on so_q0 by calling sonewconn().  When the connection
 * is established, soisconnected() is called, and transfers the
 * socket structure to so_q, making it available to accept().
 * 
 * If a socket is closed with sockets on either
 * so_q0 or so_q, these sockets are dropped.
 *
 * If higher level protocols are implemented in
 * the kernel, the wakeups done here will sometimes
 * cause software-interrupt process scheduling.
 */

void
soisconnecting(so)
	register struct socket *so;
{

	LOCK_ASSERT("soisconnecting", SOCKET_ISLOCKED(so));
	so->so_state &= ~(SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= SS_ISCONNECTING;
}

void
soisconnected(so)
	register struct socket *so;
{
	register struct socket *head = so->so_head;

	LOCK_ASSERT("soisconnected", SOCKET_ISLOCKED(so));
	so->so_state &= ~(SS_ISCONNECTING|SS_ISDISCONNECTING|SS_ISCONFIRMING);
	so->so_state |= SS_ISCONNECTED;
	if (head) {
		SOCKET_LOCK(head);
		/*
		 * So_dqlen is safe here because both locks are held, but
		 * we don't move things around while soclose is active.
		 */
		if (!(head->so_special & SP_CLOSING) && soqremque(so, 0)) {
			soqinsque(head, so, 1);
			sorwakeup(head);
			wakeup((caddr_t)&head->so_timeo);
		}
		SOCKET_UNLOCK(head);
	} else {
		wakeup((caddr_t)&so->so_timeo);
                /*
                 * Defect 115755:
                 *
                 * In BSD, a sorwakeup() call will tell the select()
                 * process to re-poll to determine if there is something
                 * to read.  In AIX, sorwakeup() will tell the select()
                 * to return that this socket has something available
                 * to read.  Hence, to maintain strict BSD compatibility
                 * we need to check if something is in the read queue
                 * before calling sorwakeup().
                 *
                 * We should always call sowwakeup() here, especially
                 * since the connect() could be non-blocking and the
                 * user program needs to select() for status.
                 */
                if(!soreadable(so))
			so->so_rcv.sb_flags |= SB_NOSELECT;
		sorwakeup(so);
		so->so_rcv.sb_flags &= ~SB_NOSELECT;
		sowwakeup(so);
	}
}

void
soisdisconnecting(so)
	register struct socket *so;
{

	LOCK_ASSERT("soisdisconnecting",SOCKET_ISLOCKED(so));
	so->so_state &= ~SS_ISCONNECTING;
	so->so_state |= (SS_ISDISCONNECTING|SS_CANTRCVMORE|SS_CANTSENDMORE);
	wakeup((caddr_t)&so->so_timeo);
	sowwakeup(so);
	sorwakeup(so);
}

void
soisdisconnected(so)
	register struct socket *so;
{

	LOCK_ASSERT("soisdisconnected", SOCKET_ISLOCKED(so));
	so->so_state &= ~(SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= (SS_CANTRCVMORE|SS_CANTSENDMORE);
	wakeup((caddr_t)&so->so_timeo);
	sowwakeup(so);
	sorwakeup(so);
}

/*
 * When an attempt at a new connection is noted on a socket
 * which accepts connections, sonewconn is called.  If the
 * connection is possible (subject to space constraints, etc.)
 * then we allocate a new structure, properly linked into the
 * data structure of the original socket, and return this.
 * Connstatus may be 0, or SO_ISCONFIRMING, or SO_ISCONNECTED.
 *
 * Currently, sonewconn() is defined as sonewsock() in socketvar.h
 * to catch calls that are missing the (new) second parameter.
 */
struct socket *
sonewsock(head, connstatus)
	register struct socket *head;
	int connstatus;
{
	register struct socket *so;
	DOMAINRC_LOCK_DECL()
	int soqueue = connstatus ? 1 : 0;

	LOCK_ASSERT("sonewsock", SOCKET_ISLOCKED(head));
	if (head->so_special & SP_CLOSING)
		goto bad;
	if (head->so_qlen + head->so_q0len > 3 * head->so_qlimit / 2)
		goto bad;
	NET_MALLOC(so, struct socket *, sizeof(*so), M_SOCKET, M_NOWAIT);
	if (so == NULL)
		goto bad;
	bzero((caddr_t)so, sizeof(*so));
	so->so_type = head->so_type;
	so->so_options = head->so_options &~ SO_ACCEPTCONN;
	so->so_linger = head->so_linger;
	so->so_state = (head->so_state | SS_NOFDREF) & ~SS_PRIV;
	so->so_special = head->so_special & SP_INHERIT;
	so->so_proto = head->so_proto;
	so->so_timeo = head->so_timeo;
	so->so_pgid = head->so_pgid;
	so->so_rcv.sb_flags = head->so_rcv.sb_flags & SB_INHERIT;
	so->so_rcv.sb_wakeone = EVENT_NULL;
	so->so_snd.sb_flags = head->so_snd.sb_flags & SB_INHERIT;
	so->so_snd.sb_wakeone = EVENT_NULL;
#if	SEC_ARCH
	bcopy(head->so_tag, so->so_tag, sizeof so->so_tag);
#endif 	/* SEC_ARCH */
#if	NETSYNC_LOCK
	{
	struct socklocks *lp;
	NET_MALLOC(lp, struct socklocks *, sizeof (*lp), M_SOCKET, M_NOWAIT);
	if (lp == NULL) {
		NET_FREE(so, M_SOCKET);
		goto bad;
	}
	SOCKET_LOCKINIT(so, lp);
	++lp->refcnt;
	}
#endif
#if	_AIX_FULLOSF
	queue_init(&so->so_snd.sb_selq);
	queue_init(&so->so_rcv.sb_selq);
#endif
	/* Since the refcnt is !0 due to head, it's not
	 * necessary to lock the domain list for the ++refcnt. */
	DOMAINRC_REF(sodomain(so));
	SOCKET_LOCK(so);
	(void) soreserve(so, head->so_snd.sb_hiwat, head->so_rcv.sb_hiwat);
	soqinsque(head, so, soqueue);

	/* 
	 * Preserve locking heiarchy here.  Must unlock in opposite order of
	 * lock aquisition.  Since the head was locked before calling sonewsock
	 * and we must leave the new socket locked and the head unlocked on 
 	 * exit, we unlock the new one, unlock the head, 
	 * then relock the new one...
	 */
	SOCKET_UNLOCK(so);
	SOCKET_UNLOCK(head);
	SOCKET_LOCK(so);
	if ((*so->so_proto->pr_usrreq)(so, PRU_ATTACH,
	    (struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0)) {
		if (so->so_head) {
			/* 
			 * Save so->so_head in oldhead since soqremque
			 * may change or zero so_head.
			 */
			struct socket *oldhead = so->so_head;

			SOCKET_LOCK(oldhead);
			(void) soqremque(so, soqueue);
			SOCKET_UNLOCK(oldhead);
		}
		SOCKET_UNLOCK(so);
#if	NETSYNC_LOCK
		LOCK_FREE(so->so_lock);
		NET_FREE(so->so_lock, M_SOCKET);
#endif
		DOMAINRC_UNREF(sodomain(so));
		NET_FREE(so, M_SOCKET);
		return (struct socket *)0;
	}
	if (connstatus) {
		SOCKET_LOCK(head);
		sorwakeup(head);
		wakeup((caddr_t)&head->so_timeo);
		so->so_state |= connstatus;
		SOCKET_UNLOCK(head);
	}
	return (so);

bad:
	SOCKET_UNLOCK(head);
	return (struct socket *)0;
}

void
soqinsque(head, so, q)
	register struct socket *head, *so;
	int q;
{
	register struct socket **prev;

	LOCK_ASSERT("soqinsque head", SOCKET_ISLOCKED(head));
	LOCK_ASSERT("soqinsque so", SOCKET_ISLOCKED(so));
	so->so_head = head;
	if (q == 0) {
		head->so_q0len++;
		so->so_q0 = 0;
		for (prev = &(head->so_q0); *prev; )
			prev = &((*prev)->so_q0);
	} else if (q > 0) {
		head->so_qlen++;
		so->so_q = 0;
		for (prev = &(head->so_q); *prev; )
			prev = &((*prev)->so_q);
	} else {
		/* so_dqlen means something else */
		so->so_dq = 0;
		for (prev = &(head->so_dq); *prev; )
			prev = &((*prev)->so_dq);
	}
	*prev = so;
}

soqremque(so, q)
	register struct socket *so;
	int q;
{
	register struct socket *head, *prev, *next;

	head = so->so_head;
	LOCK_ASSERT("soqremque head", SOCKET_ISLOCKED(head));
	LOCK_ASSERT("soqremque so", SOCKET_ISLOCKED(so));
	prev = head;
	for (;;) {
		if (q == 0)
			next = prev->so_q0;
		else if (q > 0)
			next = prev->so_q;
		else
			next = prev->so_dq;
		if (next == so)
			break;
		if (next == 0)
			return (0);
		prev = next;
	}
	if (q == 0) {
		prev->so_q0 = next->so_q0;
		head->so_q0len--;
	} else if (q > 0) {
		prev->so_q = next->so_q;
		head->so_qlen--;
	} else {
		prev->so_dq = next->so_dq;
	}
	next->so_q0 = next->so_q = next->so_dq = 0;
	next->so_head = 0;
	return (1);
}

/*
 * Socantsendmore indicates that no more data will be sent on the
 * socket; it would normally be applied to a socket when the user
 * informs the system that no more data is to be sent, by the protocol
 * code (in case PRU_SHUTDOWN).  Socantrcvmore indicates that no more data
 * will be received, and will normally be applied to the socket by a
 * protocol when it detects that the peer will send no more data.
 * Data queued for reading in the socket may yet be read.
 */

void
socantsendmore(so)
	struct socket *so;
{

	LOCK_ASSERT("socantsendmore", SOCKET_ISLOCKED(so));
	so->so_state |= SS_CANTSENDMORE;
	sowwakeup(so);
}

void
socantrcvmore(so)
	struct socket *so;
{

	LOCK_ASSERT("socantrecvmore", SOCKET_ISLOCKED(so));
	so->so_state |= SS_CANTRCVMORE;
	sorwakeup(so);
}

/*
 * Socket select/wakeup routines.
 */

#ifdef	_AIX_FULLOSF
/*
 * Queue a process for a select on a socket buffer.
 * In the parallel environment we can't simply store the thread pointer
 * due to the race between setting it and the select sleep(). So we
 * use events. In the uniprocessor MACH environment we use the
 * current thread structure instead of the proc.
 */

#include "kern/processor.h"
#include "kern/thread.h"
#include "kern/sched_prim.h"
#endif 

void
sosleep_timeout(struct trb *trb)
{
        e_clear_wait((tid_t)trb->func_data, THREAD_TIMED_OUT);
};

/*
 * Wait for data to arrive at/drain from a socket buffer.
 * Note: normally returns an unlocked sockbuf.
 */
sosbwait(sb, so)
	struct sockbuf *sb;
	struct socket *so;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

	LOCK_ASSERT("sosbwait", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("sosbwait sb", SOCKBUF_ISLOCKED(sb));

	if (!SOHASUAREA(so))
		return EWOULDBLOCK;
	sb->sb_flags |= SB_WAIT;
	e_assert_wait(&sb->sb_wakeone, !(sb->sb_flags & SB_NOINTR));

	/* Special service for MSG_WAITALL in soreceive(). */
	if (sb->sb_flags & SB_WAITING) {
		int error;
		int pri = (sb->sb_flags & SB_NOINTR) ? PZERO : (PZERO+1)|PCATCH;
		SOCKBUF_UNLOCK(sb);
		error = sosleep(so, (caddr_t)0, pri, sb->sb_timeo);
		SOCKBUF_LOCK(sb);
		so->so_rcv.sb_flags &= ~SB_WAITING;
		return error;
	}
	sbunlock(sb);
	SOCKET_UNLOCK(so);	/* Unlock and unfunnel before sleep */
	DOMAIN_UNFUNNEL_FORCE(sodomain(so), f);

        if (sb->sb_timeo) {
        	struct trb *trb;

		trb = sb->sb_timer ? (struct trb *)sb->sb_timer : talloc();
		assert(trb != NULL);

        	TICKS_TO_TIME(trb->timeout.it_value, sb->sb_timeo);

        	trb->flags      =  T_INCINTERVAL;
        	trb->func       =  sosleep_timeout;
        	trb->eventlist  =  EVENT_NULL;
        	trb->func_data  =  current_thread()->t_tid;
        	trb->ipri       =  INTTIMER;
        	tstart(trb);
        	error = e_block_thread();
#ifndef _POWER_MP
                tstop(trb);
#else
                while (tstop(trb));
#endif
                if (sb->sb_timer == NULL)
			tfree(trb);
        } else
        	error = e_block_thread();

	if (error == THREAD_AWAKENED)
		error = 0;
	else if (error == THREAD_TIMED_OUT)
		error = ETIMEDOUT;
	else
		error = EINTR;

	DOMAIN_UNFUNNEL(f);	/* (actually, refunnel) */
	SOCKET_LOCK(so);
	return error;
}

/*
 * Sleep on an address within a socket. Executes tsleep()
 * after first (maybe) releasing socket lock and unfunnelling.
 * Restores these conditions and returns tsleep() value.
 */
sosleep(so, addr, pri, tmo)
	struct socket *so;
	caddr_t addr;
	int pri, tmo;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

	if (!SOHASUAREA(so))	/* Insurance - no PCATCH or interruptible */
		pri = PZERO;
	if (addr)
		assert_wait(addr, (pri & PCATCH) != 0);

	SOCKET_UNLOCK(so);	/* Unlock and unfunnel before sleep */
	DOMAIN_UNFUNNEL_FORCE(sodomain(so), f);

        if (tmo) {
        	struct trb *trb;

		trb = talloc();
		assert(trb != NULL);

        	TICKS_TO_TIME(trb->timeout.it_value, tmo);

        	trb->flags      =  T_INCINTERVAL;
        	trb->func       =  sosleep_timeout;
        	trb->eventlist  =  EVENT_NULL;
        	trb->func_data  =  current_thread()->t_tid;
        	trb->ipri       =  INTTIMER;
        	tstart(trb);
        	error = e_block_thread();
#ifndef _POWER_MP
                tstop(trb);
#else
                while (tstop(trb));
#endif
                tfree(trb);
        } else
        	error = e_block_thread();

	if (error == THREAD_AWAKENED)
		error = 0;
	else if (error == THREAD_TIMED_OUT)
		error = ETIMEDOUT;
	else
		error = EINTR;

	DOMAIN_UNFUNNEL(f);	/* (actually, refunnel) */
	SOCKET_LOCK(so);
	return error;
}

/*
 * Wakeup processes waiting on a socket buffer.
 * Do asynchronous notification via SIGIO
 * if the socket has the SS_ASYNC flag set.
 */
void
sowakeup(so, sb)
	register struct socket *so;
	register struct sockbuf *sb;
{
	register ushort rtnevents;

	LOCK_ASSERT("sowakeup", SOCKET_ISLOCKED(so));
	rtnevents = sb->sb_reqevents;
	if (rtnevents != 0) {
		if (rtnevents & POLLPRI)
			if (!(so->so_oobmark || (so->so_state & SS_RCVATMARK)))
				rtnevents &= ~POLLPRI; 
		if (rtnevents != 0 && !(sb->sb_flags & SB_NOSELECT)) {
			sb->sb_reqevents ^= rtnevents;
			selnotify(POLL_SOCKET,so,rtnevents);
		}	
	}
	/*
	 * process kernel I/O done hooks
	 */
	if (sb->sb_flags & SB_KIODONE)
		(*sb->sb_iodone)(so, sb->sb_ioarg, sb);

	if (sb->sb_flags & (SB_WAIT|SB_WAKEONE)) {
		sb->sb_flags &= ~SB_WAIT;
		if (sb->sb_flags & SB_WAKEONE)
			e_wakeup_one(&sb->sb_wakeone);
		else
		if ((so->so_special & SP_LOCKABLE) &&
		    (so->so_lock->sp_wake == 0 || so->so_lock->sp_wake == so)) {
			if (sb == &so->so_rcv)
				so->so_special |= SP_RWAKEUP;
			else
				so->so_special |= SP_WWAKEUP;
			so->so_lock->sp_wake = so;
		} else
			e_wakeup(&sb->sb_wakeone);
	}
	if (sb->sb_wakeup)
		(void) sbwakeup(so, sb, 1);
	if (so->so_state & SS_ASYNC) {
		if (so->so_pgid < 0)
			pgsignal(-so->so_pgid, SIGIO);
		else if (so->so_pgid > 0) 
			pidsig(so->so_pgid, SIGIO);
	}
}

/*
 * Notify alternate wakeup routine of new state. The bits are
 * for XTI at the moment and encode the following:
 *	disconn ordrel conn connconfirm data oobdata
 * When any of these are valid the high bit is set, if the
 * word is all 0, a previous failed lock attempt may be retried.
 *
 * Note: while the socket and sockbuf may be accessed from this
 * upcall (e.g. for determining sbspace, state, etc), it is not
 * possible to perform an action such as sending or receiving,
 * or to modify socket values. That must be done later from a
 * safe context.
 */
int
sbwakeup(so, sb, what)
	struct socket *so;
	struct sockbuf *sb;
	int what;
{
	int state;

	LOCK_ASSERT("sbwakeup sb", SOCKBUF_ISLOCKED(sb));

	/* Encode state */
	if (so) {
		LOCK_ASSERT("sbwakeup so", SOCKET_ISLOCKED(so));
		state = SE_STATUS;
		if (what == 0)
			state |= SE_POLL;
		if (so->so_error)
			state |= SE_ERROR;
		if (sb->sb_cc)
			state |= SE_HAVEDATA;
		if (so->so_state & SS_RCVATMARK)
			state |= SE_HAVEOOB;
		if (sbspace(sb) <= 0)
			state |= SE_DATAFULL;
		if (so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING))
			state |= SE_CONNOUT;
		if (so->so_qlen)
			state |= SE_CONNIN;
		if (so->so_state & SS_ISCONNECTED) {
			if (!(so->so_state & SS_CANTSENDMORE))
				state |= SE_SENDCONN;
			if (!(so->so_state & SS_CANTRCVMORE))
				state |= SE_RECVCONN;
		}
	} else
		state = 0;
	if (sb->sb_wakeup)
		(*sb->sb_wakeup)(sb->sb_wakearg, state);
	return state;
}

int
sbpoll(so, sb)
	struct socket *so;
	struct sockbuf *sb;
{
	int state;
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	SOCKBUF_LOCK(sb);
	state = sbwakeup(so, sb, 0);
	SOCKBUF_UNLOCK(sb);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return state;
}

/*
 * Socket buffer (struct sockbuf) utility routines.
 *
 * Each socket contains two socket buffers: one for sending data and
 * one for receiving data.  Each buffer contains a queue of mbufs,
 * information about the number of mbufs and amount of data in the
 * queue, and other fields allowing select() statements and notification
 * on data availability to be implemented.
 *
 * Data stored in a socket buffer is maintained as a list of records.
 * Each record is a list of mbufs chained together with the m_next
 * field.  Records are chained together with the m_nextpkt field. The upper
 * level routine soreceive() expects the following conventions to be
 * observed when placing information in the receive buffer:
 *
 * 1. If the protocol requires each message be preceded by the sender's
 *    name, then a record containing that name must be present before
 *    any associated data (mbuf's must be of type MT_SONAME).
 * 2. If the protocol supports the exchange of ``access rights'' (really
 *    just additional data associated with the message), and there are
 *    ``rights'' to be received, then a record containing this data
 *    should be present (mbuf's must be of type MT_RIGHTS).
 * 3. If a name or rights record exists, then it must be followed by
 *    a data record, perhaps of zero length.
 *
 * Before using a new socket structure it is first necessary to reserve
 * buffer space to the socket, by calling sbreserve().  This should commit
 * some of the available buffer space in the system buffer pool for the
 * socket (currently, it does nothing but enforce limits).  The space
 * should be released by calling sbrelease() when the socket is destroyed.
 */

soreserve(so, sndcc, rcvcc)
	register struct socket *so;
	u_long sndcc, rcvcc;
{
	int error = 0;

/*	LOCK_ASSERT("soreserve so", SOCKET_ISLOCKED(so)); */
	SOCKBUF_LOCK(&so->so_snd);
	SOCKBUF_LOCK(&so->so_rcv);
	if (sbreserve(&so->so_snd, sndcc) == 0)
		error = ENOBUFS;
	else if (sbreserve(&so->so_rcv, rcvcc) == 0) {
		sbrelease(&so->so_snd);
		error = ENOBUFS;
	} else {
		if (so->so_rcv.sb_lowat == 0)
			so->so_rcv.sb_lowat = 1;
		if (so->so_snd.sb_lowat == 0) {
			if (so->so_snd.sb_hiwat >= MCLBYTES * 2)
				so->so_snd.sb_lowat = MCLBYTES;
			else
				so->so_snd.sb_lowat = so->so_snd.sb_hiwat / 2;
		} else if (so->so_snd.sb_lowat > so->so_snd.sb_hiwat)
			so->so_snd.sb_lowat = so->so_snd.sb_hiwat;
	}
	SOCKBUF_UNLOCK(&so->so_rcv);
	SOCKBUF_UNLOCK(&so->so_snd);
	return (error);
}

/*
 * Allot mbufs to a sockbuf.
 * Attempt to scale mbmax so that mbcnt doesn't become limiting
 * if buffering efficiency is near the normal case.
 */
sbreserve(sb, cc)
	struct sockbuf *sb;
	u_long cc;
{
	LOCK_ASSERT("sbreserve", SOCKBUF_ISLOCKED(sb));
	if (cc > sb_max)
		return (0);
	sb->sb_hiwat = cc;
	sb->sb_mbmax = min(cc * 2, sb_max);
	/*
	 * If clusters are much larger than some estimate of our
	 * interfaces' MTU's (and thus packet fragment size), then
	 * scale up mbmax to account for the overhead.
	 */
	if (MCLBYTES >= 2048)
		sb->sb_mbmax *= (MCLBYTES / 2048);
	if (sb->sb_lowat > sb->sb_hiwat)
		sb->sb_lowat = sb->sb_hiwat;
	return (1);
}

/*
 * Free mbufs held by a socket, and reserved mbuf space.
 */
void
sbrelease(sb)
	struct sockbuf *sb;
{

	sbflush(sb);
	sb->sb_hiwat = sb->sb_mbmax = 0;
}

/*
 * Routines to add and remove
 * data from an mbuf queue.
 *
 * The routines sbappend() or sbappendrecord() are normally called to
 * append new mbufs to a socket buffer, after checking that adequate
 * space is available, comparing the function sbspace() with the amount
 * of data to be added.  sbappendrecord() differs from sbappend() in
 * that data supplied is treated as the beginning of a new record.
 * To place a sender's address, optional access rights, and data in a
 * socket receive buffer, sbappendaddr() should be used.  To place
 * access rights and data in a socket receive buffer, sbappendrights()
 * should be used.  In either case, the new data begins a new record.
 * Note that unlike sbappend() and sbappendrecord(), these routines check
 * for the caller that there will be enough space to store the data.
 * Each fails if there is not enough space, or if it cannot find mbufs
 * to store additional information in.
 *
 * Reliable protocols may use the socket send buffer to hold data
 * awaiting acknowledgement.  Data is normally copied from a socket
 * send buffer in a protocol with m_copy for output to a peer,
 * and then removing the data from the socket buffer with sbdrop()
 * or sbdroprecord() when the data is acknowledged by the peer.
 */

/*
 * Append mbuf chain m to the last record in the
 * socket buffer sb.  The additional space associated
 * the mbuf chain is recorded in sb.  Empty mbufs are
 * discarded and mbufs are compacted where possible.
 */
void
sbappend(sb, m)
	struct sockbuf *sb;
	struct mbuf *m;
{
	register struct mbuf *n;

	LOCK_ASSERT("sbappend", SOCKBUF_ISLOCKED(sb));
	if (m == 0)
		return;
	if (n = sb->sb_mb) {
		n = sb->sb_lastpkt;
		do {
			if (n->m_flags & M_EOR) {
				sbappendrecord(sb, m); /* XXXXXX!!!! */
				return;
			}
		} while (n->m_next && (n = n->m_next));
	}
	sbcompress(sb, m, n);
}

/*
 * As above, except the mbuf chain
 * begins a new record.
 */
void
sbappendrecord(sb, m0)
	register struct sockbuf *sb;
	register struct mbuf *m0;
{
	register struct mbuf *m;

	LOCK_ASSERT("sbappendrecord", SOCKBUF_ISLOCKED(sb));
	if (m0 == 0)
		return;
	if (m = sb->sb_mb)
		while (m->m_nextpkt)
			m = m->m_nextpkt;
	/*
	 * Put the first mbuf on the queue.
	 * Note this permits zero length records.
	 */
	sballoc(sb, m0);
	if (m)
		m->m_nextpkt = m0;
	else
		sb->sb_mb = m0;
	m = m0->m_next;
	m0->m_next = 0;
	if (m && (m0->m_flags & M_EOR)) {
		m0->m_flags &= ~M_EOR;
		m->m_flags |= M_EOR;
	}
	sbcompress(sb, m, m0);
}

/*
 * As above except that OOB data
 * is inserted at the beginning of the sockbuf,
 * but after any other OOB data.
 */
void
sbinsertoob(sb, m0)
	register struct sockbuf *sb;
	register struct mbuf *m0;
{
	register struct mbuf *m;
	register struct mbuf **mp;

	LOCK_ASSERT("sbinsertoob", SOCKBUF_ISLOCKED(sb));
	if (m0 == 0)
		return;
	for (mp = &sb->sb_mb; m = *mp; mp = &((*mp)->m_nextpkt)) {
	    again:
		switch (m->m_type) {

		case MT_OOBDATA:
			continue;		/* WANT next train */

		case MT_CONTROL:
			if (m = m->m_next)
				goto again;	/* inspect THIS train further */
		}
		break;
	}
	/*
	 * Put the first mbuf on the queue.
	 * Note this permits zero length records.
	 */
	sballoc(sb, m0);
	m0->m_nextpkt = *mp;
	*mp = m0;
	m = m0->m_next;
	m0->m_next = 0;
	if (m && (m0->m_flags & M_EOR)) {
		m0->m_flags &= ~M_EOR;
		m->m_flags |= M_EOR;
	}
	sbcompress(sb, m, m0);
}

/*
 * Append address and data, and optionally, control (ancillary) data
 * to the receive queue of a socket.  If present,
 * m0 must include a packet header with total length.
 * Returns 0 if no space in sockbuf or insufficient mbufs.
 */
sbappendaddr(sb, asa, m0, control)
	register struct sockbuf *sb;
	struct sockaddr *asa;
	struct mbuf *m0, *control;
{
	register struct mbuf *m, *n;
	int space = asa->sa_len;

	LOCK_ASSERT("sbappendaddr", SOCKBUF_ISLOCKED(sb));
if (m0 && (m0->m_flags & M_PKTHDR) == 0)
panic("sbappendaddr");
	if (m0)
		space += m0->m_pkthdr.len;
	for (n = control; n; n = n->m_next) {
		space += n->m_len;
		if (n->m_next == 0)	/* keep pointer to last control buf */
			break;
	}
	if (space > sbspace(sb))
		return (0);
	if (asa->sa_len > MLEN)
		return (0);
	MGET(m, M_DONTWAIT, MT_SONAME);
	if (m == 0)
		return (0);
#ifdef	_AIX
	if (m0 && m0->m_len == 0 && (m0->m_flags & M_PKTHDR == 0))
		m0 = m_free(m0);
#endif	/* _AIX */
	m->m_len = asa->sa_len;
	bcopy((caddr_t)asa, mtod(m, caddr_t), asa->sa_len);
	if (n)
		n->m_next = m0;		/* concatenate data to control */
	else
		control = m0;
	m->m_next = control;
	for (n = m; n; n = n->m_next)
		sballoc(sb, n);
#ifdef	_AIX
       if (n = sb->sb_mb)  {
                if (!n->m_nextpkt)
                        n->m_nextpkt = m;
                else
                        sb->sb_lastpkt->m_nextpkt = m;
        }
        else
                sb->sb_mb = m;

   	sb->sb_lastpkt = m;
#else	/* _AIX */
	if (n = sb->sb_mb) {
		while (n->m_nextpkt)
			n = n->m_nextpkt;
		n->m_nextpkt = m;
	} else
		sb->sb_mb = m;
#endif	/* _AIX */
	return (1);
}

sbappendcontrol(sb, m0, control)
	struct sockbuf *sb;
	struct mbuf *control, *m0;
{
	register struct mbuf *m, *n;
	int space = 0;

	LOCK_ASSERT("sbappendcontrol", SOCKBUF_ISLOCKED(sb));
	if (control == 0)
		panic("sbappendcontrol");
	for (m = control; ; m = m->m_next) {
		space += m->m_len;
		if (m->m_next == 0)
			break;
	}
	n = m;			/* save pointer to last control buffer */
	for (m = m0; m; m = m->m_next)
		space += m->m_len;
	if (space > sbspace(sb))
		return (0);
	n->m_next = m0;			/* concatenate data to control */
	for (m = control; m; m = m->m_next)
		sballoc(sb, m);
	if (n = sb->sb_mb) {
		while (n->m_nextpkt)
			n = n->m_nextpkt;
		n->m_nextpkt = control;
	} else
		sb->sb_mb = control;
	return (1);
}

/*
 * Compress mbuf chain m into the socket
 * buffer sb following mbuf n.  If n
 * is null, the buffer is presumed empty.
 */
void
sbcompress(sb, m, n)
	register struct sockbuf *sb;
	register struct mbuf *m, *n;
{
	register int eor = 0;
	register struct mbuf *o;

	LOCK_ASSERT("sbcompress", SOCKBUF_ISLOCKED(sb));
	while (m) {
		eor |= m->m_flags & M_EOR;
		if (m->m_len == 0 &&
		    (eor == 0 ||
		     (((o = m->m_next) || (o = n)) &&
		      o->m_type == m->m_type))) {
			m = m_free(m);
			continue;
		}
		/*
		 * If compression candidate exists, weigh the bcopy
		 * cost against the buffering efficiency and the
		 * sockbuf capacity. Do an inline M_TRAILINGSPACE
		 * since we don't care about MCLREFERENCED here.
		 */
		if (n && (n->m_flags & M_EOR) == 0 &&
		    n->m_type == m->m_type &&
		    m->m_len <= ((n->m_flags & M_EXT) ?
		       n->m_ext.ext_buf + n->m_ext.ext_size : &n->m_dat[MLEN]) -
		       (n->m_data + n->m_len) &&
		    (m->m_len <= 4 * MLEN ||
		     (int)(sb->sb_mbmax - sb->sb_mbcnt) <= MCLBYTES)) {
			bcopy(mtod(m, caddr_t), mtod(n, caddr_t) + n->m_len,
			    (unsigned)m->m_len);
			n->m_len += m->m_len;
			sb->sb_cc += m->m_len;
			m = m_free(m);
			continue;
		}
		if (n)
			n->m_next = m;
		else
			sb->sb_mb = m;
		sb->sb_lastpkt = m;
		sballoc(sb, m);
		n = m;
		m->m_flags &= ~M_EOR;
		m = m->m_next;
		n->m_next = 0;
	}
	if (eor) {
		if (n)
			n->m_flags |= eor;
#if	MACH_ASSERT
		else
			printf("semi-panic: sbcompress\n");
#endif
	}
}

/*
 * Free all mbufs in a sockbuf.
 * Check that all resources are reclaimed.
 */
void
sbflush(sb)
	register struct sockbuf *sb;
{

	if (sb->sb_flags & SB_LOCK)
		panic("sbflush");
	SOCKBUF_LOCK(sb);
	while (sb->sb_mbcnt)
		sbdrop(sb, (int)sb->sb_cc);
	if (sb->sb_cc || sb->sb_mb)
		panic("sbflush 2");
	SOCKBUF_UNLOCK(sb);
}

/*
 * Drop data from (the front of) a sockbuf.
 */
void
sbdrop(sb, len)
	register struct sockbuf *sb;
	register int len;
{
	register struct mbuf *m, *mn;
	struct mbuf *next;

	LOCK_ASSERT("sbdrop", SOCKBUF_ISLOCKED(sb));
	next = (m = sb->sb_mb) ? m->m_nextpkt : 0;
	while (len > 0) {
		if (m == 0) {
			if (next == 0)
				panic("sbdrop");
			m = next;
			next = m->m_nextpkt;
			continue;
		}
		if (m->m_len > len) {
			m->m_len -= len;
			m->m_data += len;
			sb->sb_cc -= len;
			break;
		}
		len -= m->m_len;
		sbfree(sb, m);
		MFREE(m, mn);
		m = mn;
	}
	while (m && m->m_len == 0) {
		sbfree(sb, m);
		MFREE(m, mn);
		m = mn;
	}
	if (m) {
		sb->sb_mb = m;
		m->m_nextpkt = next;
	} else
		sb->sb_mb = next;
}

/*
 * Drop a record off the front of a sockbuf
 * and move the next record to the front.
 */
void
sbdroprecord(sb)
	register struct sockbuf *sb;
{
	register struct mbuf *m, *mn;

	LOCK_ASSERT("sbdroprecord", SOCKBUF_ISLOCKED(sb));
	m = sb->sb_mb;
	if (m) {
		sb->sb_mb = m->m_nextpkt;
		do {
			sbfree(sb, m);
			MFREE(m, mn);
		} while (m = mn);
	}
}

#if	NETSYNC_LOCK
/* Debugging aids: don't belong here */

int socket_lockhang = 1;

lock_socheck(so)
struct socket *so;
{
	if (!so) {
		printf("\tLocking null so\n");
		return 0;
	}
	if (!so->so_lock) {
		printf("\tLocking socket 0x%x with null lock\n", so);
		return 0;
	}
	if (!so->so_snd.sb_lock || !so->so_rcv.sb_lock)
		printf("\tLocking socket 0x%x with null sockbuf lock(s)\n", so);
	if (so->so_special & SP_FREEABLE)
		printf("\tLocking freeable socket 0x%x!\n", so);
#if	NETSYNC_SPL
	if (SOCKET_ISLOCKED(so)) {
		if (so->so_special & SP_LOCKABLE)
			printf("\tLocking locked socket 0x%x\n", so);
		return socket_lockhang;
	}
#endif
	return 1;
}
unlock_socheck(so)
struct socket *so;
{
	if (!so) {
		printf("\tUnlocking null so\n");
		return 0;
	}
	if (!so->so_lock) {
		printf("\tUnlocking socket 0x%x with null lock(s)\n", so);
		return 0;
	}
	if (so->so_snd.sb_lock && so->so_rcv.sb_lock) {
#if	SOCKBUF_LOCKTEST
		/* sockbuf locks are null if not */
		int a = SOCKBUF_ISLOCKED(&so->so_snd);
		int b = SOCKBUF_ISLOCKED(&so->so_rcv);
		if (a || b) {
			char *snd, *plus, *rcv;
			snd = plus = rcv = "";
			if (a) snd = "snd";
			if (b) rcv = "rcv";
			if (a && b) plus = "+";
			printf("\tUnlocking socket 0x%x with locked so->so_%s%s%s\n",
				so, snd, plus, rcv);
		}
#endif
	} else
		printf("\tUnlocking socket 0x%x with null sockbuf lock(s)\n", so);
	if (!SOCKET_ISLOCKED(so)) {
		printf("\tUnlocking unlocked socket 0x%x\n", so);
		return 0;
	}
	return 1;
}
/* Use sb to back up to socket and check locks there? Later if so. */
lock_sbcheck(sb)
struct sockbuf *sb;
{
	if (!sb->sb_lock) {
		printf("\tLocking sockbuf 0x%x with null lock\n", sb);
		return 0;
	}
#if	NETSYNC_SPL
	if (SOCKBUF_ISLOCKED(sb)) {
		printf("\tLocking locked sockbuf 0x%x\n", sb);
		return socket_lockhang;
	}
#endif
	return 1;
}
unlock_sbcheck(sb)
struct sockbuf *sb;
{
	if (!sb->sb_lock) {
		printf("\tUnlocking sockbuf 0x%x with null lock\n", sb);
		return 0;
	}
	if (!SOCKBUF_ISLOCKED(sb)) {
		printf("\tUnlocking unlocked sockbuf 0x%x\n", sb);
		return 0;
	}
	return 1;
}

#ifdef	S_LCK
char _net_lock_format_[] = "\t%s %s %d\n";
char _net_simple_lock_[] = "simple_lock";
char _net_simple_unlock_[] = "simple_unlock";
char _net_lock_write_[] = "lock_write";
char _net_lock_read_[] = "lock_read";
char _net_lock_write_to_read_[] = "lock_write_to_read";
char _net_lock_done_[] = "lock_done";
char _net_lock_recursive_[] = "lock_recursive";
#endif
#endif

#ifdef	_AIX
/*
 * Wait for data to arrive at/drain from a socket buffer.
 */

sbwait(sb)
	struct sockbuf *sb;
{
	/* Should be using sosbwait */
	panic("sbwait");
}

/*
 * When an attempt at a new connection is noted on a socket
 * which accepts connections, sonewconn is called.  If the
 * connection is possible (subject to space constraints, etc.)
 * then we allocate a new structure, propoerly linked into the
 * data structure of the original socket, and return this.
 * Connstatus may be 0, or SO_ISCONFIRMING, or SO_ISCONNECTED.
 *
 * Currently, sonewconn() is defined as sonewconn1() in socketvar.h
 * to catch calls that are missing the (new) second parameter.
 */
struct socket *
sonewconn1(head, connstatus)
	register struct socket *head;
	int connstatus;
{
	return(sonewsock(head, connstatus));
}
#endif
