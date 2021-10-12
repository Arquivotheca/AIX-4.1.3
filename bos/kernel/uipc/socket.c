static char sccsid[] = "@(#)95	1.45.1.38  src/bos/kernel/uipc/socket.c, sysuipc, bos41J, 9525E_all 6/21/95 13:07:01";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: sbunlock
 *		soabort
 *		soaccept
 *		sobind
 *		soclose
 *		soconnect
 *		soconnect2
 *		socreate
 *		sodequeue
 *		sodisconn
 *		sodisconnect
 *		sodqfree
 *		sofree
 *		sogetaddr
 *		sogetopt
 *		sohasoutofband
 *		solisten
 *		solockpair
 *		sopriv
 *		soreceive
 *		sorflush
 *		sosbunlock
 *		sosend
 *		sosetopt
 *		soshutdown
 *		sounlock
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
/* uipc_socket.c	2.1 16:10:40 4/20/90 SecureWare, Inc. */
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
 *	Base:	
 *	Merged: uipc_socket.c	7.23 (Berkeley) 6/29/90
 *      Merged: uipc_socket.c   7.28 (Berkeley) 5/4/91
 */

#include "net/net_globals.h"
#if	MACH
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#ifdef	_AIX_FULLOSF
#include "sys/kernel.h"
#endif	/* _AIX_FULLOSF */
#include "sys/time.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#ifdef	_AIX
#include "sys/errno.h"
#include "sys/nettrace.h"
#include "sys/poll.h"
#include "sys/uio.h"
#include "sys/types.h"
#include "net/if.h"
#include "net/route.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/in_systm.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_var.h>
#endif	/* _AIX */
#ifndef	_AIX_FULLOSF
#define	min	MIN
#endif	/* _AIX_FULLOSF */

#include "net/net_malloc.h"

#ifdef _AIX_FULLOSF
#include "kern/parallel.h"
#endif /* _AIX_FULLOSF */

#if	SEC_BASE
#include <sys/security.h>
#endif

/*
 * waste not want not...this variable defines how many bytes we'll waste 
 * when allocating a cluster in sosend().  If the size we get from net_malloc()
 * minus the user data size is > waste, 
 * then we'll allocate the next smaller bucket size.
 * eg:  a 10K write would be put into one 8K and one 2K cluster, while a 15K
 * write would go into one 16K cluster...
 */
int	waste = 2048;

LOCK_ASSERTL_DECL

/*
 * Socket operation routines.
 * These routines are called by the routines in
 * sys_socket.c or from a system process, and
 * implement the semantics of socket operations by
 * switching out to the protocol specific routines.
 *
 */
/*ARGSUSED*/
socreate(dom, aso, type, proto)
	struct socket **aso;
	register int type;
	int proto;
{
	register struct protosw *prp;
	register struct socket *so;
	register int error;
	DOMAINRC_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL4T(HKWD_SOCKET | hkwd_socreate_in, dom, aso, type, proto);	
#endif	/* _AIX */
	if (proto)
		prp = pffindproto(dom, proto, type);
	else
		prp = pffindtype(dom, type);
	if (prp == 0)
		return (EPROTONOSUPPORT);
	if (prp->pr_type != type) {
		DOMAINRC_UNREF(prp->pr_domain);
		return (EPROTOTYPE);
	}
	NET_MALLOC(so, struct socket *, sizeof(*so), M_SOCKET, M_WAITOK);
	bzero((caddr_t)so, sizeof(*so));
	so->so_snd.sb_wakeone = EVENT_NULL;
	so->so_rcv.sb_wakeone = EVENT_NULL;
	so->so_type = type;
	so->so_proto = prp;
	if (prp->pr_domain->dom_funnel == 0)
		so->so_special |= SP_LOCKABLE;
#if	SEC_ARCH
	/*
	 * POSIX ACLS
	 *     ignore return value since no back pressure is possible
	 */
	SP_OBJECT_CREATE(SIP->si_tag, so->so_tag, (tag_t *) 0, SEC_OBJECT,
 		(dac_t *) 0, (mode_t) 0);
	audstub_levels(so->so_tag);
#endif	/* SEC_ARCH */
#if	NETSYNC_LOCK
	{
	struct socklocks *lp;
	NET_MALLOC(lp, struct socklocks *, sizeof(*lp), M_SOCKET, M_WAITOK);
	SOCKET_LOCKINIT(so, lp);
	++lp->refcnt;
	}
#endif
#if	_AIX_FULLOSF
	queue_init(&so->so_snd.sb_selq);
	queue_init(&so->so_rcv.sb_selq);
#endif /* _AIX_FULLOSF */
	if (so->so_proto->pr_flags & PR_INTRLEVEL)
		so->so_special |= SP_DISABLE;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	sopriv(so);	/* Set the SS_PRIV bit if create user is privileged. */
	error =
	    (*prp->pr_usrreq)(so, PRU_ATTACH,
		(struct mbuf *)0, (struct mbuf *)proto, (struct mbuf *)0);
	if (error) {
		so->so_state |= SS_NOFDREF;
		sofree(so);
	} else
		*aso = so;
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_socreate_out, so);
#endif	/* _AIX */
	return (error);
}

sobind(so, nam)
	struct socket *so;
	struct mbuf *nam;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_sobind_in, so, nam);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	sopriv(so);
	error =
	    (*so->so_proto->pr_usrreq)(so, PRU_BIND,
		(struct mbuf *)0, nam, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

solisten(so, backlog)
	register struct socket *so;
	int backlog;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_solisten_in, so, backlog);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	sopriv(so);
	error =
	    (*so->so_proto->pr_usrreq)(so, PRU_LISTEN,
		(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
	if (error == 0) {
		if (so->so_q == 0)
			so->so_options |= SO_ACCEPTCONN;
		if (backlog < 0)
			backlog = 0;
	}
	so->so_qlimit = MIN(backlog, SOMAXCONN);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

static void
sodqfree(so)
	register struct socket *so;
{
	register struct socket *soq;

	LOCK_ASSERT("sodqfree", SOCKET_ISLOCKED(so));
	if (so->so_dqlen <= 0) {
		while (soq = so->so_dq) {
			SOCKET_LOCK(soq);
			sofree(soq);
			SOCKET_UNLOCK(soq);
		}
	}
}

void
sofree(so)
	register struct socket *so;
{
	struct socket *head = so->so_head;
	DOMAINRC_LOCK_DECL()

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sofree_in, so);	
#endif	/* _AIX */
	LOCK_ASSERT("sofree", SOCKET_ISLOCKED(so));
	if (so->so_pcb || (so->so_state & SS_NOFDREF) == 0)
		return;
	if (head) {
		/*
		 * See associated code in soclose and sodequeue.
		 */
		if (!(so->so_special & SP_CLOSING)) {
			SOCKET_LOCK(head);
			if (soqremque(so, 0) || soqremque(so, 1)) {
				if (head->so_dqlen) {
					so->so_special |= SP_CLOSING;
					soqinsque(head, so, -1);
				} else
					so->so_head = 0;
			} else
				panic("sofree dq");
			SOCKET_UNLOCK(head);
		} else {
			LOCK_ASSERT("sofree head", SOCKET_ISLOCKED(head));
			if (head->so_dqlen > 0)
				return;
			else if (soqremque(so, -1))
				so->so_head = 0;
			else
				panic("sofree dq2");
		}
	}
	if (so->so_head)	/* race in progress - loser frees */
		return;
	sbrelease(&so->so_snd);
	sorflush(so);
	DOMAINRC_UNREF(so->so_proto->pr_domain);
#if	!NETSYNC_LOCK
	NET_FREE(so, M_SOCKET);
#else
	so->so_special |= SP_FREEABLE;
#endif
}

#if	NETSYNC_LOCK
/*
 * The socklocks structure contains the locks and reference count
 *  for each socket.  When a socket is paired (ala unp_connect2),
 *  only one such structure is used for multiple sockets.
 * A note on reference counts. The socket is not explicitly reference
 *  counted due to higher level mechanisms such as the file descriptor
 *  (embodied in SS_NOFDREF). We use this to advantage since it saves a
 *  lot of bookkeeping. The socklocks structure for network-connected
 *  sockets always has a refcnt of 1, however, when two unix domain
 *  sockets are connected, they reference the same socklocks struct,
 *  and in this case refcnt grows to >= 2.
 * The sockbuf locks are currently redundant and are no-oped in a
 *  non-debug kernel (they are never taken without the socket lock).
 *  A higher degree of parallelism may be obtained with them in future.
 */

int
solockpair(so, so2)
	register struct socket *so, *so2;
{
	extern simple_lock_data_t	socklock_free_lock;
	extern struct socklocks *	free_socklocks;

	/*
	 * The lock structure for paired sockets must be the same lock
	 * in order to prevent race conditions.
	 */
	LOCK_ASSERT("solockpair so1", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("solockpair so2", SOCKET_ISLOCKED(so2));

	if (so->so_lock != so2->so_lock) {
		struct socklocks  *lp;
		int r1 = so->so_lock->refcnt;
		int r2 = so2->so_lock->refcnt;
		if (!(r1 == 1 || r2 == 1))
			return(ETOOMANYREFS);
		LOCK_ASSERT("solockpair refcnt", (r1 == 1 || r2 == 1));
		if (r1 > r2) {
			lp = so2->so_lock;
			so2->so_lock = so->so_lock;
			so2->so_rcv.sb_lock = so->so_rcv.sb_lock;
			so2->so_snd.sb_lock = so->so_snd.sb_lock;
			so->so_lock->refcnt++;
		} else {
			lp = so->so_lock;
			so->so_lock = so2->so_lock;
			so->so_rcv.sb_lock = so2->so_rcv.sb_lock;
			so->so_snd.sb_lock = so2->so_snd.sb_lock;
			so2->so_lock->refcnt++;
		}

		/*
		 * Put collapsed lock on the global freelist of lock
		 * structs, to be freed in the uipc_slowtimo()
		 * function later.
		 */

		lp->refcnt=0;
		if (so->so_special & SP_DISABLE)
			unlock_enable(lp->spl, &lp->sock_lock);
		else
			simple_unlock(&lp->sock_lock);

		simple_lock(&socklock_free_lock);
		lp->freelist = free_socklocks;
		free_socklocks = lp;
		simple_unlock(&socklock_free_lock);

	}
	return(0);
}

sounlock(so)
	struct socket *so;
{
	int special, spspecial;
	struct socket *spso;

	special = so->so_special;
	so->so_special &= ~(SP_RWAKEUP|SP_WWAKEUP);
	if (spso = so->so_lock->sp_wake) {
		LOCK_ASSERT("sounlock spso", (so->so_lock == spso->so_lock));
		so->so_lock->sp_wake = 0;
		spspecial = spso->so_special;
		spso->so_special &= ~(SP_RWAKEUP|SP_WWAKEUP);
	} else
		spspecial = 0;
	if (special & SP_FREEABLE) {

		/* Use so->so_spare to indicate that we shouldn't free this
		 * socket right now (someone needs it for e_wakeup() calls in
		 * the else clause below).  Instead, throw the socket on a
		 * free list and let uipc_slowtimo() handle it later.
		 */
		if (so->so_spare > 0) {

			extern simple_lock_data_t free_sockets_lock;
			extern struct socket      *free_sockets;

			simple_lock(&free_sockets_lock);

			/* overload so->so_tpcb (noone else uses it right
			 * now).
			 */
			so->so_tpcb = (caddr_t) free_sockets;
			free_sockets = so;
			simple_unlock(&free_sockets_lock);

			if (so->so_special & SP_DISABLE)
				unlock_enable(so->so_lock->spl,
					      &so->so_lock->sock_lock);
			else
				simple_unlock(&so->so_lock->sock_lock);
		} else {
			
			/*
			 * The socket lock implicitly protects the socklocks
			 * refcnt.  Make sure we don't drop the count without
			 * the lock! 
			 */
			struct socklocks *lp = so->so_lock;

			LOCK_ASSERT("sounlock lp", (lp != 0));
			if (lp) {
				int cnt = --lp->refcnt;
				so->so_lock = 0;
				so->so_rcv.sb_lock = so->so_snd.sb_lock = 0;
				if (so->so_special & SP_DISABLE)
					unlock_enable(lp->spl, &lp->sock_lock);
				else
					simple_unlock(&lp->sock_lock);
				if (cnt <= 0) {
					LOCK_FREE(&lp->sock_lock);
					NET_FREE(lp, M_SOCKET);
				}
			}
			if ((so->so_rcv.sb_wakeone != EVENT_NULL) || 
			    (so->so_snd.sb_wakeone != EVENT_NULL))
				panic("sounlock - freeing socket with sleepers");
			NET_FREE(so, M_SOCKET);
		}

		/*
		 * if spso was not freed, then it might have sleepers...so
		 * wake them up!
	 	 */
		if (spso != so) {
			if (spspecial & SP_RWAKEUP)
				e_wakeup(&spso->so_rcv.sb_wakeone);
			if (spspecial & SP_WWAKEUP)
				e_wakeup(&spso->so_snd.sb_wakeone);
		}
		return(1);
	} else {

		/* use so->so_spare as a semaphore indicating we can't free
		 * this socket structure right now.
		 */
		(void) fetch_and_add_h(&so->so_spare, 1);

		if (so->so_special & SP_DISABLE)
			unlock_enable(so->so_lock->spl,&so->so_lock->sock_lock);
		else
			simple_unlock(&so->so_lock->sock_lock);

		if (special & SP_RWAKEUP)
			e_wakeup(&so->so_rcv.sb_wakeone);
		if (special & SP_WWAKEUP)
			e_wakeup(&so->so_snd.sb_wakeone);
		if (spspecial & SP_RWAKEUP)
			e_wakeup(&spso->so_rcv.sb_wakeone);
		if (spspecial & SP_WWAKEUP)
			e_wakeup(&spso->so_snd.sb_wakeone);

		(void) fetch_and_add_h(&so->so_spare, -1);
	}

	return(0);
}
#endif	/* NETSYNC_LOCK */

static int
sodisconn(so)
	struct socket *so;
{
	LOCK_ASSERT("sodisconn", SOCKET_ISLOCKED(so));
	if ((so->so_state & SS_ISCONNECTED) == 0)
		return(ENOTCONN);
	if (so->so_state & SS_ISDISCONNECTING)
		return(EALREADY);
	return((*so->so_proto->pr_usrreq)(so, PRU_DISCONNECT,
	    (struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0));
}

/*
 * Close a socket on last file table reference removal.
 * Initiate disconnect if connected.
 * Free socket when disconnect complete.
 */
soclose(so)
	register struct socket *so;
{
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_soclose_in, so);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (so->so_options & SO_ACCEPTCONN) {
		register struct socket *soq;
		/*
		 * A lock hierarchy problem appears here when racing
		 * soabort by a netisr thread: so_dqlen arbitrates.
		 * Setting SP_CLOSING prevents any new accepts, and
		 * so_dqlen pushes all sofree's to so_dq, where we
		 * safely clean them up.
		 */
		so->so_special |= SP_CLOSING;
		++so->so_dqlen;
		while ((soq = so->so_q0) || (soq = so->so_q)) {
			SOCKET_UNLOCK(so);
			(void) soabort(soq);
			SOCKET_LOCK(so);
		}
		if (--so->so_dqlen <= 0 && so->so_dq)
			sodqfree(so);
	}
	if (so->so_pcb == 0)
		goto discard;
	if (so->so_state & SS_ISCONNECTED) {
		if ((so->so_state & SS_ISDISCONNECTING) == 0) {
			error = sodisconn(so);
			if (error)
				goto drop;
		}
		if (so->so_options & SO_LINGER) {
			if ((so->so_state & SS_ISDISCONNECTING) &&
			    (so->so_state & SS_NBIO))
				goto drop;
			while (so->so_state & SS_ISCONNECTED)
				if (error = sosleep(so, (caddr_t)&so->so_timeo,
				    (PZERO+1) | PCATCH, so->so_linger))
					break;
		}
	}
drop:
	if (so->so_pcb) {
		int error2 =
		    (*so->so_proto->pr_usrreq)(so, PRU_DETACH,
			(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
		if (error == 0)
			error = error2;
	}
discard:
	if (so->so_state & SS_NOFDREF)
		panic("soclose: NOFDREF");
	so->so_snd.sb_wakeup = so->so_rcv.sb_wakeup = 0;
	so->so_snd.sb_wakearg = so->so_rcv.sb_wakearg = 0;
	so->so_state |= SS_NOFDREF;
	sofree(so);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_soclose_out, so);	
#endif	/* _AIX */
	return (error);
}

soabort(so)
	struct socket *so;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_soabort_in, so);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error =
	    (*so->so_proto->pr_usrreq)(so, PRU_ABORT,
		(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

soaccept(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_soaccept_in, so, nam);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	/* Test used to be reversed. Now the responsibility of caller. */
	if (so->so_state & SS_NOFDREF)
		panic("soaccept: NOFDREF");
	sopriv(so);
	error = (*so->so_proto->pr_usrreq)(so, PRU_ACCEPT,
	    (struct mbuf *)0, nam, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_soaccept_out, error);	
#endif	/* _AIX */
	return (error);
}

soconnect(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_soconnect_in, so, nam);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (so->so_options & SO_ACCEPTCONN)
		error = EOPNOTSUPP;
	else {
		sopriv(so);
		/*
		 * If protocol is connection-based, can only connect once.
		 * Otherwise, if connected, try to disconnect first.
		 * This allows user to disconnect by connecting to, e.g.,
		 * a null address.
		 */
		if (so->so_state & (SS_ISCONNECTED|SS_ISCONNECTING) &&
		    ((so->so_proto->pr_flags & PR_CONNREQUIRED) ||
		    (error = sodisconn(so))))
			error = EISCONN;
		else
			error = (*so->so_proto->pr_usrreq)(so, PRU_CONNECT,
			    (struct mbuf *)0, nam, (struct mbuf *)0);
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

soconnect2(so1, so2)
	register struct socket *so1;
	struct socket *so2;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_soconnect2_in, so1, so2);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so1), f);

	/* 
	 * Since sopriv() unlocks the socket in AIX, we need to preserve
	 * the locking heirarchy by not using SOCKET_LOCK2()...
	 */
	SOCKET_LOCK(so1);
	sopriv(so1); 
	if ((so1)->so_lock != (so2)->so_lock)
		SOCKET_LOCK(so2);
	sopriv(so2);
	error = (*so1->so_proto->pr_usrreq)(so1, PRU_CONNECT2,
	    (struct mbuf *)0, (struct mbuf *)so2, (struct mbuf *)0);
	if ((so1)->so_lock != (so2)->so_lock)
		SOCKET_UNLOCK(so2);
	SOCKET_UNLOCK(so1);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

sodisconnect(so)
	register struct socket *so;
{
	int error;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sodisconnect_in, so);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error = sodisconn(so);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sodisconnect_out, error);	
#endif	/* _AIX */
	return (error);
}

/*
 * Send on a socket.
 * If send must go all at once and message is larger than
 * send buffering, then hard error.
 * Lock against other senders.
 * If must go all at once and not enough room now, then
 * inform user that this would block and do nothing.
 * Otherwise, if nonblocking, send as much as possible.
 * The data to be sent is described by "uio" if nonzero,
 * otherwise by the mbuf chain "top" (which must be null
 * if uio is not).  Data provided in mbuf chain must be small
 * enough to send all at once.
 *
 * Returns nonzero on error, timeout or signal; callers
 * must check for short counts if EINTR/ERESTART are returned.
 * Data and control buffers are freed on return.
 */
sosend(so, addr, uio, top, control, flags)
	register struct socket *so;
	struct mbuf *addr;
	struct uio *uio;
	struct mbuf *top;
	struct mbuf *control;
	int flags;
{
	struct mbuf **mp;
	register struct mbuf *m;
	register long space, len, resid;
	int clen = 0, error, dontroute, mlen, atomic;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	int sendthresh;
	struct tcpcb *tcpcb;
	u_short maxseg = 0;

	TRCHKL5T(HKWD_SOCKET | hkwd_sosend_in, so, addr, uio, top, control);	
#endif	/* _AIX */
	if (top && (uio || !(top->m_flags & M_PKTHDR)))
	panic("sosend 1");
	if (uio) {
		resid = uio->uio_resid;
		if ((uint)resid > 0x70000000)  /* sanity check for length arg */
			return(EINVAL);
	} else
		resid = top->m_pkthdr.len;

	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);

	/* Hack to turn off EOR of not supported (e.g.), for TCP */
	if (so->so_proto->pr_flags & PR_NOEOR)
		flags &= ~MSG_EOR;
	atomic = sosendallatonce(so) || top;
	if (!atomic && sowatomic(so, resid)) atomic = -1;
	dontroute =
	    (flags & MSG_DONTROUTE) && (so->so_options & SO_DONTROUTE) == 0 &&
	    (so->so_proto->pr_flags & PR_ATOMIC);
#ifdef	SOCKET_RESOURCES
	if (SOHASUAREA(so)) {
		U_HANDY_LOCK();
		u.u_ru.ru_msgsnd++;
		U_HANDY_UNLOCK();
	}
#endif
	if (control)
		clen = control->m_len;
#define	snderr(errno)	{ error = errno; goto release; }

restart:
	if (error = sosblock(&so->so_snd, so))
		goto out;
	do {
		if (so->so_state & SS_CANTSENDMORE)
			snderr(EPIPE);
		if (so->so_error)
			snderr(so->so_error);
		if ((so->so_state & SS_ISCONNECTED) == 0) {
			if (so->so_proto->pr_flags & PR_CONNREQUIRED) {
				if ((so->so_state & SS_ISCONFIRMING) == 0 &&
				    !(resid == 0 && clen != 0))
					snderr(ENOTCONN);
			} else if (addr == 0)
				snderr(EDESTADDRREQ);
		}
		space = sbspace(&so->so_snd);
		if (flags & MSG_OOB)
			space += 1024;
		if (space < resid + clen &&
		    (atomic || space < so->so_snd.sb_lowat || space < clen)) {
			if (atomic > 0 && (resid > so->so_snd.sb_hiwat ||
			    clen > so->so_snd.sb_hiwat))
				snderr(EMSGSIZE);
			if ((so->so_state & SS_NBIO) || (flags & MSG_NONBLOCK))
				snderr(EWOULDBLOCK);
			if (error = sosbwait(&so->so_snd, so))
				goto out;
			goto restart;
		}
		SOCKBUF_UNLOCK(&so->so_snd);
#ifdef _AIX

		/*
		 * If the protocol is TCP and there is a maxseg and
		 * TCP_NODELAY is NOT set, then try pipelining...
		 */
                if ((so->so_proto->pr_domain->dom_family == AF_INET) &&
                    (so->so_type == SOCK_STREAM) &&
                    (so->so_pcb != NULL) ) {
                        tcpcb = ((struct inpcb *)so->so_pcb)->inp_ppcb;
                        if (tcpcb != NULL && !(tcpcb->t_flags & TF_NODELAY))
                                maxseg = tcpcb->t_maxseg;
                }

#endif /* _AIX */
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		mp = &top;
		space -= clen;
		do {
		    if (uio == NULL) {
			/*
			 * Data is prepackaged in "top".
			 */
			resid = 0;
			if (flags & MSG_EOR)
				top->m_flags |= M_EOR;
		    } else {
#ifdef _AIX
                        if (maxseg)
                                sendthresh = 2 * maxseg;
                        else
                                sendthresh = resid;
#endif /* _AIX */

			do {

#define bucket_size(size) (1 << (BUCKETINDX((size))))
			if (resid > MHLEN) {
				if (resid >= MAXALLOCSAVE)
					m = m_getclustm(M_WAIT, MT_DATA, 
						MAXALLOCSAVE);
				else if ((bucket_size(resid) - resid) > waste)
					m = m_getclustm(M_WAIT, MT_DATA,
						(1 << (BUCKETINDX(resid)-1)));
				else
					m = m_getclustm(M_WAIT, MT_DATA, resid);
				if (top == 0) {
					m->m_flags |= M_PKTHDR; /* XXX */
					m->m_pkthdr.len = 0;
					m->m_pkthdr.rcvif = (struct ifnet *)0;
				}
				mlen = m->m_ext.ext_size;
				if (atomic > 0 && top == 0) {
					mlen -= max_hdr;
					m->m_data += max_hdr;
				}
				len = min(mlen, resid);
			} else {
				if (top == 0) {
					MGETHDR(m, M_WAIT, MT_DATA);
					mlen = MHLEN;
					m->m_pkthdr.len = 0;
					m->m_pkthdr.rcvif = (struct ifnet *)0;
				} else {
					MGET(m, M_WAIT, MT_DATA);
					mlen = MLEN;
				}
				len = min(mlen, resid);
				/*
				 * For datagram protocols, leave room
				 * for protocol headers in first mbuf.
				 */
				if (atomic > 0 && top == 0 && len < mlen)
					MH_ALIGN(m, len);
			}
			space -= len;
#ifdef	_AIX_FULLOSF
			error = uiomove(mtod(m, caddr_t), len, uio);
#else	/* _AIX_FULLOSF */
			if (len > 0)
				error = uiomove(mtod(m, caddr_t), len, 
					UIO_WRITE, uio);
#endif	/* _AIX_FULLOSF */
#ifdef	_AIX
			sendthresh -= len;
#endif	/* _AIX */
			resid = uio->uio_resid;
			m->m_len = len;
			*mp = m;
			top->m_pkthdr.len += len;
			if (error) {
				DOMAIN_FUNNEL(sodomain(so), f);
				SOCKET_LOCK(so);
				SOCKBUF_LOCK(&so->so_snd);
				goto release;
			}
			mp = &m->m_next;
			if (resid <= 0) {
				if (flags & MSG_EOR)
					top->m_flags |= M_EOR;
				break;
			}
#ifdef	_AIX
		        } while (space > 0 && (atomic || sendthresh > 0) );
#else	/* _AIX */
		        } while (space > 0 && atomic);
#endif	/* _AIX */
		    }
		    DOMAIN_FUNNEL(sodomain(so), f);
		    SOCKET_LOCK(so);
		    if (dontroute)
			    so->so_options |= SO_DONTROUTE;
		    error = (*so->so_proto->pr_usrreq)(so,
			(flags & MSG_OOB) ? PRU_SENDOOB : PRU_SEND,
			top, addr, control);
		    if (dontroute)
			    so->so_options &= ~SO_DONTROUTE;
		    /*
		     * If we're done, then bail out of double while loop.
		     * This saves an extra unlock, then lock on the last
		     * iteration of the loops...
		     */
		    if (!error && !resid) {
			control = 0;
			top = 0;
			goto release;
		    }
		    SOCKET_UNLOCK(so);
		    DOMAIN_UNFUNNEL(f);
		    clen = 0;
		    control = 0;
		    top = 0;
		    mp = &top;
		    if (error) {
			DOMAIN_FUNNEL(sodomain(so), f);
			SOCKET_LOCK(so);
			SOCKBUF_LOCK(&so->so_snd);
			goto release;
		    }
		} while (resid && space > 0);
		DOMAIN_FUNNEL(sodomain(so), f);
		SOCKET_LOCK(so);
		SOCKBUF_LOCK(&so->so_snd);
	} while (resid);

release:
	sbunlock(&so->so_snd);
out:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (top)
		m_freem(top);
	if (control)
		m_freem(control);
	TRCHKL1T(HKWD_SOCKET | hkwd_sosend_out, error);
	return (error);
}

/*
 * Implement receive operations on a socket.
 * We depend on the way that records are added to the sockbuf
 * by sbappend*.  In particular, each record (mbufs linked through m_next)
 * must begin with an address if the protocol so specifies,
 * followed by an optional mbuf or mbufs containing ancillary data,
 * and then zero or more mbufs of data.
 * In order to avoid blocking network interrupts for the entire time here,
 * we splx() while doing the actual copy to user space.
 * Although the sockbuf is locked, new data may still be appended,
 * and thus we must maintain consistency of the sockbuf during that time.
#if	NETSYNC_LOCK
 * Note "sockbuf locked" means only SB_LOCK set to synchronize with other
 * processes. The actual SOCKBUF_LOCK is released.
#endif
 * 
 * The caller may receive the data as a single mbuf chain by supplying
 * an mbuf **mp0 for use in returning the chain.  The uio is then used
 * only for the count in uio_resid.
 */
soreceive(so, paddr, uio, mp0, controlp, flagsp)
	register struct socket *so;
	struct mbuf **paddr;
	struct uio *uio;
	struct mbuf **mp0;
	struct mbuf **controlp;
	int *flagsp;
{
	register struct mbuf *m, **mp;
	register int flags, len, error, offset;
	struct protosw *pr = so->so_proto;
	struct mbuf *nextrecord, *freehead = 0;
	int moff, type;
	DOMAIN_FUNNEL_DECL(f)

/*
 * DELAY_MFREE - macro to delay freeing mbufs until after the socket lock
 * is released.  Reduces contentin on socket lock.
 */
#define	DELAY_MFREE(m, sb, freehead) \
{ \
	register struct mbuf *tmp; \
	sb = m->m_next; \
	tmp = freehead; \
	freehead = m; \
	freehead->m_next = tmp; \
	freehead->m_nextpkt = (struct mbuf *)NULL; \
}

#ifdef	_AIX
	TRCHKL5T(HKWD_SOCKET | hkwd_soreceive_in, so, paddr, uio, mp0,controlp);
#endif	/* _AIX */
	mp = mp0;
	if (paddr)
		*paddr = 0;
	if (controlp)
		*controlp = 0;
	if (flagsp)
		flags = *flagsp &~ MSG_EOR;
	else 
		flags = 0;
	if (flags & MSG_OOB) {
		m = m_get(M_WAIT, MT_DATA);
		DOMAIN_FUNNEL(sodomain(so), f);
		SOCKET_LOCK(so);
		error = (*pr->pr_usrreq)(so, PRU_RCVOOB,
		    m, (struct mbuf *)(flags & MSG_PEEK), (struct mbuf *)0);
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		if (error == 0) do {
#ifdef	_AIX_FULLOSF
			error = uiomove(mtod(m, caddr_t),
			    (int) min(uio->uio_resid, m->m_len), uio);
#else	/* _AIX_FULLOSF */
			error = uiomove(mtod(m, caddr_t),
			    (int) min(uio->uio_resid, m->m_len), UIO_READ, uio);
#endif	/* _AIX_FULLOSF */
			m = m_free(m);
		} while (uio->uio_resid && error == 0 && m);
		if (m)
			m_freem(m);
		return (error);
	}
	if (mp)
		*mp = (struct mbuf *)0;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (so->so_state & SS_ISCONFIRMING && uio->uio_resid)
		(*pr->pr_usrreq)(so, PRU_RCVD, (struct mbuf *)0,
		    (struct mbuf *)0, (struct mbuf *)0);

restart:
	if (error = sosblock(&so->so_rcv, so))
		goto out;
	m = so->so_rcv.sb_mb;
	/*
	 * If we have less data than requested, block awaiting more
	 * (subject to any timeout) if:
	 *   1. the current count is less than the low water mark, or
	 *   2. MSG_WAITALL is set, and it is possible to do the entire
	 *	receive operation at once if we block (resid <= hiwat).
	 * If MSG_WAITALL is set but resid is larger than the receive buffer,
	 * we have to do the receive in sections, and thus risk returning
	 * a short count if a timeout or signal occurs after we start.
	 */
	while (m == 0 || so->so_rcv.sb_cc < uio->uio_resid && 
	    (so->so_rcv.sb_cc < so->so_rcv.sb_lowat ||
	    ((flags & MSG_WAITALL) && uio->uio_resid <= so->so_rcv.sb_hiwat)) &&
	    m->m_nextpkt == 0) {
#ifdef DIAGNOSTIC
		if (m == 0 && so->so_rcv.sb_cc)
			panic("receive 1");
#endif
		if (so->so_error) {
			if (m)
				break;
			error = so->so_error;
			if ((flags & MSG_PEEK) == 0)
				so->so_error = 0;
			goto release;
		}
		if (so->so_state & SS_CANTRCVMORE) {
			if (m)
				break;
			goto release;
		}
		for (; m; m = m->m_next)
			if (m->m_type == MT_OOBDATA  || (m->m_flags & M_EOR)) {
				m = so->so_rcv.sb_mb;
				goto dontblock;
			}
		if ((so->so_state & (SS_ISCONNECTED|SS_ISCONNECTING)) == 0 &&
		    (so->so_proto->pr_flags & PR_CONNREQUIRED)) {
			error = ENOTCONN;
			goto release;
		}
		if (uio->uio_resid == 0)
			goto release;
		if ((so->so_state & SS_NBIO) || (flags & MSG_NONBLOCK)) {
			error = EWOULDBLOCK;
			goto release;
		}
		if (error = sosbwait(&so->so_rcv, so))
			goto out;
		goto restart;
	}
dontblock:
#ifdef	SOCKET_RESOURCES
	if (SOHASUAREA(so)) {
		U_HANDY_LOCK();
		u.u_ru.ru_msgrcv++;
		U_HANDY_UNLOCK();
	}
#endif
#ifdef DIAGNOSTIC
	if (m->m_type == 0)
	panic("receive 3a");
#endif
	nextrecord = m->m_nextpkt;
	if (pr->pr_flags & PR_ADDR) {
#ifdef DIAGNOSTIC
		if (m->m_type != MT_SONAME)
			panic("receive 1a");
#endif
		if (flags & MSG_PEEK) {
			if (paddr)
				*paddr = m_copym(m, 0, m->m_len, M_WAIT);
			m = m->m_next;
		} else {
			sbfree(&so->so_rcv, m);
			if (paddr) {
				*paddr = m;
				so->so_rcv.sb_mb = m->m_next;
				m->m_next = 0;
				m = so->so_rcv.sb_mb;
			} else {
				DELAY_MFREE(m, so->so_rcv.sb_mb, freehead);
				m = so->so_rcv.sb_mb;
			}
		}
	}
	while (m && m->m_type == MT_CONTROL && error == 0) {
		if (flags & MSG_PEEK) {
			if (controlp)
				*controlp = m_copym(m, 0, m->m_len, M_WAIT);
			m = m->m_next;
		} else {
			sbfree(&so->so_rcv, m);
			if (controlp) {
				if (pr->pr_domain->dom_externalize &&
				    mtod(m, struct cmsghdr *)->cmsg_type ==
				    SCM_RIGHTS)
				   error = (*pr->pr_domain->dom_externalize)(m);
				*controlp = m;
				so->so_rcv.sb_mb = m->m_next;
				m->m_next = 0;
				m = so->so_rcv.sb_mb;
			} else {
				if (pr->pr_domain->dom_dispose &&
				    mtod(m, struct cmsghdr *)->cmsg_type ==
				    SCM_RIGHTS)
					(*pr->pr_domain->dom_dispose)(m);
				DELAY_MFREE(m, so->so_rcv.sb_mb, freehead);
				m = so->so_rcv.sb_mb;
			}
		}
		if (controlp)
			controlp = &(*controlp)->m_next;
	}
	if (m) {
		if ((flags & MSG_PEEK) == 0)
			m->m_nextpkt = nextrecord;
		type = m->m_type;
		if (type == MT_OOBDATA)
			flags |= MSG_OOB;
	}
	moff = 0;
	offset = 0;
	while (m && uio->uio_resid > 0 && error == 0) {
		if (m->m_type == MT_OOBDATA) {
			if (type != MT_OOBDATA)
				break;
		} else if (type == MT_OOBDATA)
			break;
#ifdef DIAGNOSTIC
		else if (m->m_type != MT_DATA && m->m_type != MT_HEADER)
			panic("receive 3");
#endif
		so->so_state &= ~SS_RCVATMARK;
		len = uio->uio_resid;
		if (so->so_oobmark && len > so->so_oobmark - offset)
			len = so->so_oobmark - offset;
		if (len > m->m_len - moff)
			len = m->m_len - moff;
		/*
		 * If mp is set, just pass back the mbufs.
		 * Otherwise copy them out via the uio, then free.
		 * Sockbuf must be consistent here (points to current mbuf,
		 * it points to next record) when we drop priority;
		 * we must note any additions to the sockbuf when we
		 * block interrupts again.
		 */
		if (mp == 0) {
			SOCKBUF_UNLOCK(&so->so_rcv);
			SOCKET_UNLOCK(so);
			DOMAIN_UNFUNNEL(f);
#ifdef	_AIX_FULLOSF
			error = uiomove(mtod(m, caddr_t) + moff, (int)len, uio);
#else	/* _AIX_FULLOSF */
			error = uiomove(mtod(m, caddr_t) + moff, 
					(int)len, UIO_READ, uio);
#endif	/* _AIX_FULLOSF */
			DOMAIN_FUNNEL(sodomain(so), f);
			SOCKET_LOCK(so);
			SOCKBUF_LOCK(&so->so_rcv);
		} else
			uio->uio_resid -= len;
		if (len == m->m_len - moff) {
			if (m->m_flags & M_EOR)
				flags |= MSG_EOR;
			if (flags & MSG_PEEK) {
				m = m->m_next;
				moff = 0;
			} else {
				nextrecord = m->m_nextpkt;
				sbfree(&so->so_rcv, m);
				if (mp) {
					*mp = m;
					mp = &m->m_next;
					so->so_rcv.sb_mb = m = m->m_next;
					*mp = (struct mbuf *)0;
				} else {
					DELAY_MFREE(m, so->so_rcv.sb_mb, 
						freehead);
					m = so->so_rcv.sb_mb;
				}
				if (m)
					m->m_nextpkt = nextrecord;
			}
		} else {
			if (flags & MSG_PEEK)
				moff += len;
			else {
				if (mp)
					*mp = m_copym(m, 0, len, M_WAIT);
				m->m_data += len;
				m->m_len -= len;
				so->so_rcv.sb_cc -= len;
			}
		}
		if (so->so_oobmark) {
			if ((flags & MSG_PEEK) == 0) {
				so->so_oobmark -= len;
				if (so->so_oobmark == 0) {
					so->so_state |= SS_RCVATMARK;
					break;
				}
			} else {
				offset += len;
				if (so->so_oobmark == offset)
					break;
			}
		}
		if (flags & MSG_EOR)
			break;
		/*
		 * If the MSG_WAITALL flag is set (for non-atomic socket),
		 * we must not quit until "uio->uio_resid == 0" or an error
		 * termination.  If a signal/timeout occurs, return
		 * with a short count but without error.
		 * Keep sockbuf locked against other readers.
		 */
		while (flags & MSG_WAITALL && m == 0 && uio->uio_resid > 0 &&
		    !sosendallatonce(so)) {
			if (so->so_error || so->so_state & SS_CANTRCVMORE)
				break;
			if ((flags & MSG_PEEK) == 0 &&
			    pr->pr_flags & PR_WANTRCVD && so->so_pcb) {
				SOCKBUF_UNLOCK(&so->so_rcv);
				(*pr->pr_usrreq)(so, PRU_RCVD, (struct mbuf *)0,
				    (struct mbuf *)flags, (struct mbuf *)0);
				SOCKBUF_LOCK(&so->so_rcv);
			}
			so->so_rcv.sb_flags |= SB_WAITING;
			if (sosbwait(&so->so_rcv, so))
				goto release;
			if (m = so->so_rcv.sb_mb)
				nextrecord = m->m_nextpkt;
		}
	}
	if ((flags & MSG_PEEK) == 0) {
		if (m == 0)
			so->so_rcv.sb_mb = nextrecord;
		else if (pr->pr_flags & PR_ATOMIC) {
			flags |= MSG_TRUNC;
			(void) sbdroprecord(&so->so_rcv);
		}
		if (pr->pr_flags & PR_WANTRCVD && so->so_pcb) {
			SOCKBUF_UNLOCK(&so->so_rcv);
			(*pr->pr_usrreq)(so, PRU_RCVD, (struct mbuf *)0,
			    (struct mbuf *)flags, (struct mbuf *)0);
			SOCKBUF_LOCK(&so->so_rcv);
		}
	}
	if (flagsp)
		*flagsp |= flags;
release:
	sbunlock(&so->so_rcv);
out:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (freehead)
		m_freem(freehead);
	return (error);
#undef	DELAY_MFREE
}


soshutdown(so, how)
	register struct socket *so;
	register int how;
{
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL2T(HKWD_SOCKET | hkwd_soshutdown_in, so, how);	
#endif	/* _AIX */

	switch (how++) {
	case 0: case 1: case 2:
		break;
	default:
		return (EINVAL);
	}
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (how & 1)
		sorflush(so);
	if (how & 2)
		error = (*so->so_proto->pr_usrreq)(so, PRU_SHUTDOWN,
		    (struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

void
sorflush(so)
	register struct socket *so;
{
	register struct sockbuf *sb = &so->so_rcv;
	register struct protosw *pr = so->so_proto;
	struct sockbuf asb;
	DOMAIN_FUNNEL_DECL(f)

	LOCK_ASSERT("sorflush", SOCKET_ISLOCKED(so));

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sorflush_in, so);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	sb->sb_flags |= SB_NOINTR;
	(void) sosblock(sb, so);
	SOCKBUF_UNLOCK(sb);		/* want SB_LOCK but !locked */
	socantrcvmore(so);
	SOCKBUF_LOCK(sb);
	sbunlock(sb);
	asb = *sb;
	/* We cannot just bzero the sockbuf, it would destroy our
	 * locks and/or select queue. So, we do it the silly way. */
	sb->sb_cc	= 0;
	sb->sb_hiwat	= 0;
	sb->sb_mbcnt	= 0;
	sb->sb_mbmax	= 0;
	sb->sb_lowat	= 0;
	sb->sb_mb	= 0;
	sb->sb_flags	= 0;
	sb->sb_timeo	= 0;
	DOMAIN_UNFUNNEL(f);
	if (pr->pr_flags & PR_RIGHTS && pr->pr_domain->dom_dispose)
		(*pr->pr_domain->dom_dispose)(asb.sb_mb);
	sbrelease(&asb);
}

void
sopriv(so)
	struct socket *so;
{
	int junk;
	int priv;
	LOCK_ASSERT("sopriv", SOCKET_ISLOCKED(so));

	if (SOHASUAREA(so) && !(so->so_special & SP_EXTPRIV)) {
		SOCKET_UNLOCK(so);
		priv = suser(&junk); /* This can fault */
		SOCKET_LOCK(so);
		if (priv)
			so->so_state |= SS_PRIV;
		else
			so->so_state &= ~SS_PRIV;
	}
}

sosetopt(so, level, optname, m0)
	register struct socket *so;
	int level, optname;
	struct mbuf *m0;
{
	int error = 0;
	register struct mbuf *m = m0;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL4T(HKWD_SOCKET | hkwd_sosetopt_in, so, level, optname, m0);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	sopriv(so);
	if (level != SOL_SOCKET) {
		if (so->so_proto && so->so_proto->pr_ctloutput) {
			error = (*so->so_proto->pr_ctloutput)
				(PRCO_SETOPT, so, level, optname, &m0);
			m = 0;
		} else
			error = ENOPROTOOPT;
	} else {
		switch (optname) {

		case SO_LINGER:
			if (m == NULL || m->m_len != sizeof (struct linger)) {
				error = EINVAL;
				goto bad;
			}
			so->so_linger = mtod(m, struct linger *)->l_linger;
			/* fall thru... */

		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_USELOOPBACK:
		case SO_BROADCAST:
		case SO_REUSEADDR:
		case SO_OOBINLINE:
		case SO_CKSUMRECV:
			if (m == NULL || m->m_len < sizeof (int)) {
				error = EINVAL;
				goto bad;
			}
			if (*mtod(m, int *))
				so->so_options |= optname;
			else
				so->so_options &= ~optname;
			break;

		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
			if (m == NULL || m->m_len < sizeof (int)) {
				error = EINVAL;
				goto bad;
			}
			switch (optname) {

			case SO_SNDBUF:
				SOCKBUF_LOCK(&so->so_snd);
				if (sbreserve(&so->so_snd,
				    (u_long) *mtod(m, int *)) == 0)
					error = ENOBUFS;
				SOCKBUF_UNLOCK(&so->so_snd);
				if (error)
					goto bad;
				break;

			case SO_RCVBUF:
				SOCKBUF_LOCK(&so->so_rcv);
				if (sbreserve(&so->so_rcv,
				    (u_long) *mtod(m, int *)) == 0)
					error = ENOBUFS;
				SOCKBUF_UNLOCK(&so->so_rcv);
				if (error)
					goto bad;
				break;

			case SO_SNDLOWAT:
				so->so_snd.sb_lowat = *mtod(m, int *);
				break;
			case SO_RCVLOWAT:
				so->so_rcv.sb_lowat = *mtod(m, int *);
				break;
			}
			break;

		case SO_SNDTIMEO:
		case SO_RCVTIMEO:
		    {
			struct timeval *tv;
			short val;

			if (m == NULL || m->m_len < sizeof (*tv)) {
				error = EINVAL;
				goto bad;
			}
			tv = mtod(m, struct timeval *);
#ifdef _AIX_FULLOSF
			if (tv->tv_sec > SHRT_MAX / hz - hz) {
#else
			if (tv->tv_sec > MAX_SECS_TO_uS ) {
#endif /* _AIX_FULLOSF */
				error = EDOM;
				goto bad;
			}
#ifdef	_AIX_FULLOSF
			val = tv->tv_sec * hz + tv->tv_usec / tick;
#else	/* _AIX_FULLOSF */
			val = tv->tv_sec * uS_PER_SECOND + tv->tv_usec ;
#endif	/* _AIX_FULLOSF */

			switch (optname) {

			case SO_SNDTIMEO:
				so->so_snd.sb_timeo = val;
				break;
			case SO_RCVTIMEO:
				so->so_rcv.sb_timeo = val;
				break;
			}
			break;
		    }

		default:
			error = ENOPROTOOPT;
			break;
		}
	}
bad:
	if (m)
		(void) m_free(m);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sosetopt_out, error);	
#endif	/* _AIX */
	return (error);
}

sogetopt(so, level, optname, mp)
	register struct socket *so;
	int level, optname;
	struct mbuf **mp;
{
	register struct mbuf *m;
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

#ifdef	_AIX
	TRCHKL4T(HKWD_SOCKET | hkwd_sogetopt_in, so, level, optname, mp);	
#endif	/* _AIX */
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	sopriv(so);
	if (level != SOL_SOCKET) {
		if (so->so_proto && so->so_proto->pr_ctloutput)
			error = (*so->so_proto->pr_ctloutput)
				(PRCO_GETOPT, so, level, optname, mp);
		else
			error = ENOPROTOOPT;
	} else {
		m = m_get(M_WAIT, MT_SOOPTS);
		m->m_len = sizeof (int);

		switch (optname) {

		case SO_LINGER:
			m->m_len = sizeof (struct linger);
			mtod(m, struct linger *)->l_onoff =
				so->so_options & SO_LINGER;
			mtod(m, struct linger *)->l_linger = so->so_linger;
			break;

		case SO_USELOOPBACK:
		case SO_DONTROUTE:
		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_REUSEADDR:
		case SO_BROADCAST:
		case SO_OOBINLINE:
		case SO_CKSUMRECV:
			*mtod(m, int *) = so->so_options & optname;
			break;

		case SO_TYPE:
			*mtod(m, int *) = so->so_type;
			break;

		case SO_ERROR:
			*mtod(m, int *) = so->so_error;
			so->so_error = 0;
			break;

		case SO_SNDBUF:
			*mtod(m, int *) = so->so_snd.sb_hiwat;
			break;

		case SO_RCVBUF:
			*mtod(m, int *) = so->so_rcv.sb_hiwat;
			break;

		case SO_SNDLOWAT:
			*mtod(m, int *) = so->so_snd.sb_lowat;
			break;

		case SO_RCVLOWAT:
			*mtod(m, int *) = so->so_rcv.sb_lowat;
			break;

		case SO_SNDTIMEO:
		case SO_RCVTIMEO:
		    {
			int val = (optname == SO_SNDTIMEO ?
			     so->so_snd.sb_timeo : so->so_rcv.sb_timeo);

			m->m_len = sizeof(struct timeval);
#ifdef _AIX_FULLOSF
			mtod(m, struct timeval *)->tv_sec = val / hz;
			mtod(m, struct timeval *)->tv_usec =
			    (val % hz) / tick;
#else	/* _AIX_FULLOSF */
			mtod(m,struct timeval *)->tv_sec = val / uS_PER_SECOND;
			mtod(m,struct timeval *)->tv_usec =val % uS_PER_SECOND;
#endif	/* _AIX_FULLOSF */
			break;
		    }

		default:
			(void)m_free(m);
			error = ENOPROTOOPT;
			goto bad;
		}
		*mp = m;
	}
bad:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
#ifdef	_AIX
	TRCHKL0T(HKWD_SOCKET | hkwd_sogetopt_out);	
#endif	/* _AIX */
	return (error);
}

void
sohasoutofband(so)
	register struct socket *so;
{
	struct proc *p;
#ifndef	_AIX_FULLOSF
	struct sockbuf *sb;
	register ushort rtnevents;
#endif	/* _AIX_FULLOSF */

	LOCK_ASSERT("sohasoutofband so", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("sohasoutofband sb", SOCKBUF_ISLOCKED(&so->so_rcv));

#ifdef	_AIX
	TRCHKL1T(HKWD_SOCKET | hkwd_sohasoutofband_in, so);	
#endif	/* _AIX */

	if (so->so_pgid < 0)
		gsignal(-so->so_pgid, SIGURG);
	else if (so->so_pgid > 0 && (p = pfind(so->so_pgid)) != 0) {
#if	NETISR_THREAD
		psignal_inthread(p, SIGURG);
#else
		psignal(p, SIGURG);
#endif
		P_UNREF(p);
	}

	if (so->so_rcv.sb_reqevents)
	{
		sb = &(so->so_rcv);
		rtnevents = sb->sb_reqevents;
		if (rtnevents & POLLPRI)
			if (!(so->so_oobmark ||
			      (so->so_state & SS_RCVATMARK)))
				rtnevents &= ~POLLPRI; 
		if (rtnevents != 0)
			{
				sb->sb_reqevents ^= rtnevents;
				selnotify(POLL_SOCKET,so,rtnevents);
			}	
	}
#ifdef	_AIX
	TRCHKL0T(HKWD_SOCKET | hkwd_sohasoutofband_out);	
#endif	/* _AIX */
}

/*
 * "Accept" the first queued connection.
 */
sodequeue(head, so, nam, compat_43)
	struct socket *head, **so;
	struct mbuf **nam;
{
	int error = 0;
	struct mbuf *m;
	struct socket *aso;
	DOMAIN_FUNNEL_DECL(f)

	if (nam)
		*nam = 0;
	*so = 0;
	DOMAIN_FUNNEL(sodomain(head), f);
	SOCKET_LOCK(head);
	if ((head->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto bad;
	}
again:
	if (head->so_qlen == 0) {
		error = ENOTCONN;
		goto bad;
	}
	if (head->so_error) {
		error = head->so_error;
		head->so_error = 0;
		goto bad;
	}
	/*
	 * Other threads may race this accept when we unlock "head" in
	 * order to follow proper lock hierarchy. We dequeue the _first_
	 * on so_q, and protect it (from sofree or other deq's) with
	 * head->so_dqlen. If we lose such a race, the thread that comes
	 * in last calls sofree(). Note sofree checks NOFDREF, etc. No
	 * race can occur if !NETSYNC_LOCK.
	 */
	if (head->so_special & SP_CLOSING) {
		error = ECONNABORTED;	/* paranoia */
		goto bad;
	}
	aso = head->so_q;
	++head->so_dqlen;
	SOCKET_UNLOCK(head);
	SOCKET_LOCK(aso);
	SOCKET_LOCK(head);
	if (aso != head->so_q) {		/* Didn't win race */
		SOCKET_UNLOCK(head);
		SOCKET_UNLOCK(aso);
		SOCKET_LOCK(head);
		aso = 0;
	}
	if (--head->so_dqlen <= 0 && head->so_dq) /* Last does any cleanup */
		sodqfree(head);
	if (aso == 0)				/* Back to starting block */
		goto again;

	if (soqremque(aso, 1) == 0)
		panic("sodequeue");
	aso->so_state &= ~SS_NOFDREF;
	SOCKET_UNLOCK(head);
	SOCKET_UNLOCK(aso);
	DOMAIN_UNFUNNEL(f);
	m = m_getclr(M_WAIT, MT_SONAME);
	(void) soaccept(aso, m);
	*so = aso;
	if (nam) {
		if (compat_43)
			sockaddr_old(m);
		*nam = m;
	} else
		m_freem(m);
	return error;

bad:
	SOCKET_UNLOCK(head);
	DOMAIN_UNFUNNEL(f);
	return error;
}

sogetaddr(so, nam, which, compat_43)
	struct socket *so;
	struct mbuf **nam;
{
	int error;
	struct mbuf *m = 0;
	DOMAIN_FUNNEL_DECL(f)

	*nam = 0;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (which && (so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		error = ENOTCONN;
		goto bad;
	}
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		error = ENOBUFS;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so,
				which ? PRU_PEERADDR : PRU_SOCKADDR,
				(struct mbuf *)0, m, (struct mbuf *)0);
	if (error == 0) {
		if (compat_43)
			sockaddr_old(m);
		*nam = m;
	}

bad:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (error && m)
		m_freem(m);
	return error;
}

/*
 * Sockbuf lock/unlock.
 */

sosblock(sb, so)
	register struct sockbuf *sb;
	struct socket *so;
{
	int error, pri;

	LOCK_ASSERT("sosblock", SOCKET_ISLOCKED(so));

	SOCKBUF_LOCK(sb);
	while (sb->sb_flags & SB_LOCK) {
		pri = (sb->sb_flags & SB_NOINTR) ? PZERO : (PZERO+1)|PCATCH;
		sb->sb_flags |= SB_WANT;
		if (!SOHASUAREA(so)) {	/* After SB_WANT for wakeup later */
			SOCKBUF_UNLOCK(sb);
			return EWOULDBLOCK;
		}
		assert_wait((caddr_t)&sb->sb_flags, !(sb->sb_flags&SB_NOINTR));
		SOCKBUF_UNLOCK(sb);
		error = sosleep(so, (caddr_t)0, pri, 0);
		if (error)
			return error;
		SOCKBUF_LOCK(sb);
	}
	sb->sb_flags |= SB_LOCK;
	return 0;
}

void
sbunlock(sb)
	register struct sockbuf *sb;
{
	LOCK_ASSERT("sbunlock", SOCKBUF_ISLOCKED(sb));

	sb->sb_flags &= ~SB_LOCK;
	if (sb->sb_flags & (SB_WANT|SB_WAKEONE)) {
		sb->sb_flags &= ~SB_WANT;
		if (sb->sb_flags & SB_WAKEONE)
			wakeup_one((caddr_t)&sb->sb_flags);
		else
			wakeup((caddr_t)&sb->sb_flags);
		if (sb->sb_wakeup)
			(void) sbwakeup((struct socket *)0, sb, 1);
	}
	SOCKBUF_UNLOCK(sb);
}
