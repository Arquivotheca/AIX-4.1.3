static char sccsid[] = "@(#)90	1.11.1.6  src/bos/kernel/net/raw_usrreq.c, sysnet, bos411, 9428A410j 5/16/94 16:03:10";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: raw_ctlinput
 *		raw_init
 *		raw_input
 *		raw_usrreq
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
 * OSF/1 1.1 Snapshot 2
 */
/* raw_usrreq.c	1.2 14:09:34 5/15/90 SecureWare */
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
 * Copyright (c) 1980, 1986 Regents of the University of California.
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
 *	Base:	raw_usrreq.c	7.7 (Berkeley) 9/20/89
 *	Merged:	raw_usrreq.c	7.9 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"
#if	MACH
#include "sys/secdefines.h"
#endif

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"
#include "net/raw_cb.h"

LOCK_ASSERTL_DECL

/*
 * Initialize raw connection block q.
 */
void
raw_init()
{

	RAW_LOCKINIT();
	ROUTE_LOCKINIT();
	NETSTAT_LOCKINIT(&rtstat.rts_lock);
	rawcb.rcb_next = rawcb.rcb_prev = &rawcb;
}


/*
 * Raw protocol input routine.  Find the socket
 * associated with the packet(s) and move them over.  If
 * nothing exists for this packet, drop it.
 */
/*
 * Raw protocol interface.
 */
raw_input(m0, proto, src, dst)
	struct mbuf *m0;
	register struct sockproto *proto;
	struct sockaddr *src, *dst;
{
	register struct rawcb *rp;
	register struct mbuf *m = m0;
	register int sockets = 0;
	struct socket *last;
	RAW_LOCK_DECL()

	last = 0;
	RAW_LOCK();
	for (rp = rawcb.rcb_next; rp != &rawcb; rp = rp->rcb_next) {
		if (rp->rcb_proto.sp_family != proto->sp_family)
			continue;
		if (rp->rcb_proto.sp_protocol  &&
		    rp->rcb_proto.sp_protocol != proto->sp_protocol)
			continue;
		/*
		 * We assume the lower level routines have
		 * placed the address in a canonical format
		 * suitable for a structure comparison.
		 *
		 * Note that if the lengths are not the same
		 * the comparison will fail at the first byte.
		 */
#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), a1->sa_len) == 0)
		if (rp->rcb_laddr && !equal(rp->rcb_laddr, dst))
			continue;
		if (rp->rcb_faddr && !equal(rp->rcb_faddr, src))
			continue;
		if (last) {
			struct mbuf *n;
			if (n = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT)) {
				SOCKET_LOCK(last);
				SOCKBUF_LOCK(&last->so_rcv);
				if (sbappendaddr(&last->so_rcv, src,
				    n, (struct mbuf *)0) == 0) {
					SOCKBUF_UNLOCK(&last->so_rcv);
					/* should notify about lost packet */
					m_freem(n);
				} else {
					SOCKBUF_UNLOCK(&last->so_rcv);
					sorwakeup(last);
					sockets++;
				}
				SOCKET_UNLOCK(last);
			}
		}
		last = rp->rcb_socket;
	}
	if (last) {
		SOCKET_LOCK(last);
		SOCKBUF_LOCK(&last->so_rcv);
		if (sbappendaddr(&last->so_rcv, src,
		    m, (struct mbuf *)0) == 0) {
			SOCKBUF_UNLOCK(&last->so_rcv);
			m_freem(m);
		} else {
			SOCKBUF_UNLOCK(&last->so_rcv);
			sorwakeup(last);
			sockets++;
		}
		SOCKET_UNLOCK(last);
	} else
		m_freem(m);
	RAW_UNLOCK();
	return (sockets);
}

/*ARGSUSED*/
void
raw_ctlinput(cmd, arg, extra)
	int cmd;
	struct sockaddr *arg;
	caddr_t extra;
{

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	/* INCOMPLETE */
}

/*ARGSUSED*/
raw_usrreq(so, req, m, nam, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register struct rawcb *rp = sotorawcb(so);
	register int error = 0;
	int s, len;
	RAW_LOCK_DECL()
       

	if (req == PRU_CONTROL)
		return (EOPNOTSUPP);
#if	!SEC_ARCH
	if (control && control->m_len) {
		error = EOPNOTSUPP;
		goto release;
	}
#endif
	if (rp == 0) {
		error = EINVAL;
		goto release;
	}

	/*
	 * We must take the RAW_LOCK first to avoid deadlock with
	 * raw_input(). It is safe to drop the socket lock in the
	 * usrreq entry due to the file table reference from above.
	 */
	SOCKET_UNLOCK(so);
	RAW_LOCK();
	SOCKET_LOCK(so);

	switch (req) {

	/*
	 * Allocate a raw control block and fill in the
	 * necessary info to allow packets to be routed to
	 * the appropriate raw interface routine.
	 */
	case PRU_ATTACH:
		if ((so->so_state & SS_PRIV) == 0) {
			error = EACCES;
			break;
		}
		error = raw_attach(so, (int)nam);
		break;

	/*
	 * Destroy state just before socket deallocation.
	 * Flush data or not depending on the options.
	 */
	case PRU_DETACH:
		if (rp == 0) {
			error = ENOTCONN;
			break;
		}
		raw_detach(rp);
		break;

#ifdef notdef
	/*
	 * If a socket isn't bound to a single address,
	 * the raw input routine will hand it anything
	 * within that protocol family (assuming there's
	 * nothing else around it should go to). 
	 */
	case PRU_CONNECT:
		if (rp->rcb_faddr) {
			error = EISCONN;
			break;
		}
		nam = m_copym(nam, 0, M_COPYALL, M_WAIT);
		rp->rcb_faddr = mtod(nam, struct sockaddr *);
		soisconnected(so);
		break;

	case PRU_BIND:
		if (rp->rcb_laddr) {
			error = EINVAL;			/* XXX */
			break;
		}
		error = raw_bind(so, nam);
		break;
#endif

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		break;		/* NOT goto release; */

	case PRU_DISCONNECT:
		if (rp->rcb_faddr == 0) {
			error = ENOTCONN;
			break;
		}
		raw_disconnect(rp);
		soisdisconnected(so);
		break;

	/*
	 * Mark the connection as being incapable of further input.
	 */
	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	/*
	 * Ship a packet out.  The appropriate raw output
	 * routine handles any massaging necessary.
	 */
	case PRU_SEND:
		if (nam) {
			if (rp->rcb_faddr) {
			    error = EISCONN;
			    break;
			}
			rp->rcb_faddr = mtod(nam, struct sockaddr *);
		} else if (rp->rcb_faddr == 0) {
			error = ENOTCONN;
			break;
		}

		SOCKET_UNLOCK(so);
		RAW_UNLOCK();
		SOCKET_LOCK(so);
		error = (*so->so_proto->pr_output)(m, so);

		m = NULL;
		if (nam)
		    rp->rcb_faddr = 0;
		goto release;

	case PRU_ABORT:
		raw_disconnect(rp);
		soisdisconnected(so);
		sofree(so);
		break;

	case PRU_SENSE:
		/*
		 * stat: don't bother with a blocksize.
		 */
		SOCKET_UNLOCK(so);
		RAW_UNLOCK();
		SOCKET_LOCK(so);
		return (0);

	/*
	 * Not supported.
	 */
	case PRU_RCVOOB:
	case PRU_RCVD:
		SOCKET_UNLOCK(so);
		RAW_UNLOCK();
		SOCKET_LOCK(so);
		return(EOPNOTSUPP);

	case PRU_LISTEN:
	case PRU_ACCEPT:
	case PRU_SENDOOB:
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		if (rp->rcb_laddr == 0) {
			error = EINVAL;
			break;
		}
		len = rp->rcb_laddr->sa_len;
		bcopy((caddr_t)rp->rcb_laddr, mtod(nam, caddr_t), (unsigned)len);
		nam->m_len = len;
		break;

	case PRU_PEERADDR:
		if (rp->rcb_faddr == 0) {
			error = ENOTCONN;
			break;
		}
		len = rp->rcb_faddr->sa_len;
		bcopy((caddr_t)rp->rcb_faddr, mtod(nam, caddr_t), (unsigned)len);
		nam->m_len = len;
		break;

	default:
		panic("raw_usrreq");
	}
	SOCKET_UNLOCK(so);
	RAW_UNLOCK();
	SOCKET_LOCK(so);
release:
	if (control != NULL)
		m_freem(control);
	if (m != NULL)
		m_freem(m);
	return (error);
}
