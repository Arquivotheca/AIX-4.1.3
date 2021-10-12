static char sccsid[] = "@(#)99	1.32.1.28  src/bos/kernel/uipc/usrreq.c, sysuipc, bos41J, 9525E_all 6/21/95 13:07:03";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: 
 *		uipc_init
 *		uipc_usrreq
 *		unp_abort
 *		unp_attach
 *		unp_bind
 *		unp_connect
 *		unp_connect2
 *		unp_detach
 *		unp_discard
 *		unp_disconnect
 *		unp_dispose
 *		unp_drain
 *		unp_drop
 *		unp_externalize
 *		unp_internalize
 *		unp_mark
 *		unp_scan
 *		unp_shutdown
 *		unp_usrclosed
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
/* uipc_usrreq.c	2.1 16:10:44 4/20/90 SecureWare, Inc. */
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	Base:	uipc_usrreq.c	7.13 (Berkeley) 10/19/89
 * 	Merged: uipc_usrreq.c	7.20 (Berkeley) 6/28/90
 * 	Merged: uipc_usrreq.c   7.26 (Berkeley) 6/3/91
 */

#include "net/net_globals.h"
#if	_AIX_FULLOSF
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/stat.h"
#include "sys/time.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/unpcb.h"
#include "sys/un.h"

#include "net/net_malloc.h"

#ifdef	_AIX
#include "sys/var.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/low.h"
#include <sys/fs_locks.h>
#endif	/* _AIX */

#if	SEC_ARCH
#include <sys/security.h>
#include <sys/secpolicy.h>
#include <sys/ioctl.h>

extern caddr_t findrights();
#endif

#ifdef	_AIX
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/uio.h>
#endif	/* _AIX */

LOCK_ASSERTL_DECL

/*
 * Unix communications domain.
 *
 * TODO:
 *	SEQPACKET, RDM
 *	rethink name space problems
 *	need a proper out-of-band
 */
struct unpcb	unb;			/* global list used by netstat */
CONST struct	sockaddr sun_noname = { sizeof(sun_noname), AF_UNIX };
ino_t	unp_vno;			/* prototype for fake vnode numbers */
#if	NETSYNC_LOCK
simple_lock_data_t	global_unpconn_lock;
simple_lock_data_t	unp_misc_lock;
#endif

simple_lock_data_t	socklock_free_lock;
struct socklocks *	free_socklocks;

simple_lock_data_t	free_sockets_lock;
struct socket *         free_sockets;

void
uipc_init()
{
	unb.unp_queue.next = unb.unp_queue.prev = &unb.unp_queue;
	UNPCONN_LOCKINIT();
	UNPMISC_LOCKINIT();

	/* Initialize socklocks free list and lock. */
	free_socklocks = 0;
	lock_alloc(&socklock_free_lock, LOCK_ALLOC_PIN, UNPMISC_LOCK_FAMILY,
		   4);
	simple_lock_init(&socklock_free_lock);

	/* Initialize socket free list and lock. */
	free_sockets = 0;
	lock_alloc(&free_sockets_lock, LOCK_ALLOC_PIN, UNPMISC_LOCK_FAMILY, 4);
	simple_lock_init(&free_sockets_lock);
	
}

/*ARGSUSED*/
uipc_usrreq(so, req, m, nam, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	struct unpcb *unp = sotounpcb(so);
	register struct socket *so2;
	register int error = 0;

	if (req == PRU_CONTROL) {
#if	SEC_ARCH
		/* XXX m == scalar, nam == int * */
		if ((int) m == SIOCGPEERPRIV) {
			if (unp->unp_conn == (struct unpcb *) 0)
				return ENOTCONN;
			so2 = unp->unp_conn->unp_socket;
			/* Don't lock so2 for this */
			*(int *) nam = (so2->so_state & SS_PRIV) != 0;
			return 0;
		}
#endif
		return (EOPNOTSUPP);
	}
	if (req != PRU_SEND && control && control->m_len) {
		error = EOPNOTSUPP;
		goto release;
	}
	if (unp == 0 && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}

	LOCK_ASSERT("uipc_usrreq", SOCKET_ISLOCKED(so));

	switch (req) {

	case PRU_ATTACH:
		if (unp) {
			error = EISCONN;
			break;
		}
		error = unp_attach(so);
		break;

	case PRU_DETACH:
		unp_detach(unp);
		break;

	case PRU_BIND:
		SOCKET_UNLOCK(so);		/* namei may be interrupted */
		/* if no nam, then came in a backdoor (xtiso); feign success */
		error = nam ? unp_bind(unp, nam) : 0;
		SOCKET_LOCK(so);
		break;

	case PRU_LISTEN:
		if (unp->unp_vnode == 0)
			error = EINVAL;
#ifdef	_AIX
		if (so->so_type == SOCK_DGRAM)
			error = EOPNOTSUPP;
#endif	/* _AIX */
		break;

	case PRU_CONNECT:
		error = unp_connect(so, nam);
		break;

	case PRU_CONNECT2:
		error = unp_connect2(so, (struct socket *)nam);
		break;

	case PRU_DISCONNECT:
		unp_disconnect(unp);
		break;

	case PRU_ACCEPT:
		/*
		 * Pass back name of connected socket,
		 * if it was bound and we are still connected
		 * (our peer may have closed already!).
		 */
		if (unp->unp_conn && unp->unp_conn->unp_addr) {
			nam->m_len = unp->unp_conn->unp_addr->m_len;
			bcopy(mtod(unp->unp_conn->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else {
			nam->m_len = sizeof(sun_noname);
			*(mtod(nam, struct sockaddr *)) = sun_noname;
		}
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		unp_shutdown(unp);
		break;

	case PRU_RCVD:
		switch (so->so_type) {

		case SOCK_DGRAM:
			panic("uipc 1");
			/*NOTREACHED*/

		case SOCK_STREAM:
#define	rcv (&so->so_rcv)
#define snd (&so2->so_snd)
			if (unp->unp_conn == 0)
				break;
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_RCVD STREAM so2notso", (so->so_lock == so2->so_lock));
			SOCKBUF_LOCK(rcv);
			SOCKBUF_LOCK(snd);
			/*
			 * Adjust backpressure on sender
			 * and wakeup any waiting to write.
			 */
			snd->sb_mbmax += unp->unp_mbcnt - rcv->sb_mbcnt;
			unp->unp_mbcnt = rcv->sb_mbcnt;
			snd->sb_hiwat += unp->unp_cc - rcv->sb_cc;
			unp->unp_cc = rcv->sb_cc;
#ifdef	_AIX_FULLOSF
			if (so->so_special & SP_PIPE) {	/* Posix/AES */
				struct timeval now;
				microtime(&now);
				unp->unp_atime = now.tv_sec;
			}
#endif	/* _AIX_FULLOSF */
			if (snd->sb_flags & SB_NOTIFY) {
				SOCKBUF_UNLOCK(snd);
				sowwakeup(so2);
			} else
				SOCKBUF_UNLOCK(snd);
			SOCKBUF_UNLOCK(rcv);
#undef snd
#undef rcv
			break;

		default:
			panic("uipc 2");
		}
		break;

	case PRU_SEND:
		if (control && (error = unp_internalize(control)))
			break;
		switch (so->so_type) {

		case SOCK_DGRAM: {
			struct sockaddr *from;

			if (nam) {
				if (unp->unp_conn) {
					error = EISCONN;
					break;
				}
				error = unp_connect(so, nam);
				LOCK_ASSERT("uipc_usrreq PRU_SEND DGRAM so", SOCKET_ISLOCKED(so));
				if (error)
					break;
			} else {
				if (unp->unp_conn == 0) {
					error = ENOTCONN;
					break;
				}
			}
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_SEND DGRAM so2notso", (so2->so_lock == so->so_lock));
			SOCKBUF_LOCK(&so->so_snd);
			SOCKBUF_LOCK(&so2->so_rcv);
			if (unp->unp_addr)
				from = mtod(unp->unp_addr, struct sockaddr *);
			else
				from = (struct sockaddr *)&sun_noname;
			if (sbappendaddr(&so2->so_rcv, from, m, control)) {
				if (so2->so_rcv.sb_flags & SB_NOTIFY) {
					SOCKBUF_UNLOCK(&so2->so_rcv);
					sorwakeup(so2);
				} else
					SOCKBUF_UNLOCK(&so2->so_rcv);
				m = 0;
				control = 0;
			} else {
				SOCKBUF_UNLOCK(&so2->so_rcv);
				error = ENOBUFS;
			}
			SOCKBUF_UNLOCK(&so->so_snd);
			if (nam)
				unp_disconnect(unp);
			break;
		}

		case SOCK_STREAM:
#define	rcv (&so2->so_rcv)
#define	snd (&so->so_snd)
			if (so->so_state & SS_CANTSENDMORE) {
				error = EPIPE;
				break;
			}
			if (unp->unp_conn == 0)
				panic("uipc 3");
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_SEND STREAM so2notso", (so2->so_lock == so->so_lock));
			SOCKBUF_LOCK(snd);
			SOCKBUF_LOCK(rcv);
			/*
			 * Send to paired receive port, and then reduce
			 * send buffer hiwater marks to maintain backpressure.
			 * Wake up readers.
			 */
			if (control) {
				if (sbappendcontrol(rcv, m, control))
					control = 0;
				else {
					unp_dispose(control);
					error = ENOBUFS;
				}
			} else
				sbappend(rcv, m);
			if (error == 0) {
				snd->sb_mbmax -=
				    rcv->sb_mbcnt - unp->unp_conn->unp_mbcnt;
				unp->unp_conn->unp_mbcnt = rcv->sb_mbcnt;
				snd->sb_hiwat -= 
					rcv->sb_cc - unp->unp_conn->unp_cc;
				unp->unp_conn->unp_cc = rcv->sb_cc;
#ifdef	_AIX_FULLOSF
				if (so->so_special & SP_PIPE) {	/* Posix/AES */
					struct timeval now;
					microtime(&now);
					unp->unp_ctime = 
						unp->unp_mtime = now.tv_sec;
				}
#endif	/* _AIX_FULLOSF */
				if (rcv->sb_flags & SB_NOTIFY) {
					SOCKBUF_UNLOCK(rcv);
					sorwakeup(so2);
				} else
					SOCKBUF_UNLOCK(rcv);
				SOCKBUF_UNLOCK(snd);
				m = 0;
			}
#undef snd
#undef rcv
			break;

		default:
			panic("uipc 4");
		}
		break;

	case PRU_ABORT:
		unp_drop(unp, ECONNABORTED);
		break;

	case PRU_SENSE:
		if (so->so_type == SOCK_STREAM && unp->unp_conn != 0) {
			so2 = unp->unp_conn->unp_socket;
#ifdef _AIX_FULLOSF
			/* Pipe stat's must behave per Posix/AES */
			if (so->so_special & SP_PIPE) {
				struct socket *rso, *wso;
				struct unpcb *runp, *wunp;
				/* Make socket pair appear as one entity */
				if (so->so_special & SP_WATOMIC) { /* write */
					wso = so; rso = so2;
					wunp = unp; runp = unp->unp_conn;
				} else {			   /* read */
					wso = so2; rso = so;
					wunp = unp->unp_conn; runp = unp;
				}
				((struct stat *) m)->st_atime = runp->unp_atime;
				((struct stat *) m)->st_mtime = wunp->unp_mtime;
				((struct stat *) m)->st_ctime = wunp->unp_ctime;
				((struct stat *) m)->st_blksize =
					wso->so_snd.sb_hiwat+rso->so_rcv.sb_cc;
				((struct stat *) m)->st_size =
					rso->so_rcv.sb_cc;
				if (unp->unp_vno == 0)
					unp->unp_vno = unp->unp_conn->unp_vno;
			/* Else traditional socket behavior */
			} else 
#endif
			{
				((struct stat *) m)->st_blksize =
					so->so_snd.sb_hiwat+so2->so_rcv.sb_cc;
				((struct stat *) m)->st_size =
					so2->so_rcv.sb_cc;
			}
		} else
			((struct stat *) m)->st_blksize = so->so_snd.sb_hiwat;
#ifdef	_AIX_FULLOSF
		((struct stat *) m)->st_dev = NODEV;
#else	/* _AIX_FULLOSF */
		((struct stat *) m)->st_dev = NODEVICE;
#endif	/* _AIX_FULLOSF */
		if (unp->unp_vno == 0) {
			UNPMISC_LOCK();
			unp->unp_vno = unp_vno++;
			UNPMISC_UNLOCK();
		}
		((struct stat *) m)->st_ino = unp->unp_vno;
		return (0);

	case PRU_RCVOOB:
		return (EOPNOTSUPP);

	case PRU_SENDOOB:
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		if (unp->unp_addr) {
			nam->m_len = unp->unp_addr->m_len;
			bcopy(mtod(unp->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else
			nam->m_len = 0;
		break;

	case PRU_PEERADDR:
		if (unp->unp_conn && unp->unp_conn->unp_addr) {
			nam->m_len = unp->unp_conn->unp_addr->m_len;
			bcopy(mtod(unp->unp_conn->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else if (unp->unp_conn) {
			nam->m_len = sizeof(sun_noname);
			*(mtod(nam, struct sockaddr *)) = sun_noname;
		} else
			nam->m_len = 0;
		break;

	case PRU_SLOWTIMO:
		break;

	default:
		panic("piusrreq");
	}
release:
	if (control)
		m_freem(control);
	if (m)
		m_freem(m);
	return (error);
}

/*
 * Both send and receive buffers are allocated PIPSIZ bytes of buffering
 * for stream sockets, although the total for sender and receiver is
 * actually only PIPSIZ.
 * Datagram sockets really use the sendspace as the maximum datagram size,
 * and don't really want to reserve the sendspace.  Their recvspace should
 * be large enough for at least one max-size datagram plus address.
 */
#define        PIPSIZ  4096
u_long	unpst_sendspace = PIPSIZ;
u_long	unpst_recvspace = PIPSIZ;
u_long	unpdg_sendspace = 2*1024;	/* really max datagram size */
u_long	unpdg_recvspace = 4*1024;

int	unp_rights=0;			/* file descriptors in flight */

unp_attach(so)
	struct socket *so;
{
	register struct unpcb *unp;
	int error;
	
	if (so->so_snd.sb_hiwat == 0 || so->so_rcv.sb_hiwat == 0) {
		switch (so->so_type) {

		case SOCK_STREAM:
			error = soreserve(so, unpst_sendspace, unpst_recvspace);
			break;

		case SOCK_DGRAM:
			error = soreserve(so, unpdg_sendspace, unpdg_recvspace);
			break;
		}
		if (error)
			return (error);
	}
	NET_MALLOC(unp, struct unpcb *, sizeof *unp, M_PCB, M_NOWAIT);
	if (unp == NULL)
		return (ENOBUFS);
	bzero((caddr_t)unp, sizeof *unp);
	so->so_pcb = (caddr_t)unp;
	unp->unp_socket = so;
	UNPCONN_LOCK();
	insque(&unp->unp_queue, &unb.unp_queue);
	UNPCONN_UNLOCK();
	return (0);
}

void
unp_detach(unp)
	register struct unpcb *unp;
{
	struct  socket *unp_save = unp->unp_socket;

	SOCKET_UNLOCK(unp->unp_socket);
	bsdlog_unreg(unp->unp_socket);
	if (unp->unp_vnode) {

		/* 
		 * Respect the lock heiarchy...
		 */
		VN_LOCK(unp->unp_vnode);
		unp->unp_vnode->v_socket = 0;
		VN_UNLOCK(unp->unp_vnode);
#ifdef	_AIX_FULLOSF
		vrele(unp->unp_vnode);
#else	/* _AIX_FULLOSF */
		VNOP_RELE(unp->unp_vnode);
#endif	/* _AIX_FULLOSF */
		unp->unp_vnode = 0;
	} 
	SOCKET_LOCK(unp->unp_socket);
	if (unp->unp_conn)
		unp_disconnect(unp);
	while (unp->unp_refs)
		unp_drop(unp->unp_refs, ECONNRESET);
	soisdisconnected(unp->unp_socket);
	UNPCONN_LOCK();
	remque(&unp->unp_queue);
	UNPCONN_UNLOCK();
	unp->unp_socket->so_pcb = 0;
	m_freem(unp->unp_addr);
	NET_FREE(unp, M_PCB);
	UNPMISC_LOCK();
	if (unp_rights) {
		UNPMISC_UNLOCK();
		sorflush(unp_save);
	}
	else
		UNPMISC_UNLOCK();
}

unp_bind(unp, nam)
	struct unpcb *unp;
	struct mbuf *nam;
{
	struct sockaddr_un *soun;
	struct vnode *vp;
#ifdef	_AIX_FULLOSF
	register struct nameidata *ndp = &u.u_nd;
#endif	/* _AIX_FULLOSF */
	struct vattr vattr;
	int error;

	if (nam == NULL)
		return (EDESTADDRREQ);
	soun = mtod(nam, struct sockaddr_un *);
#ifdef	_AIX_FULLOSF
	ndp->ni_dirp = soun->sun_path;
#endif	/* _AIX_FULLOSF */
	if (unp->unp_vnode != NULL)
		return (EINVAL);
#ifdef	_AIX
#ifdef	notdef /* 152207 */
	if (soun->sun_family != AF_UNIX)
		return (EAFNOSUPPORT);
#endif
#endif
	if (nam->m_len == MLEN) {
		if (*(mtod(nam, caddr_t) + nam->m_len - 1) != 0)
			return (EINVAL);
	} else if (nam->m_len < 3) {
		return (EINVAL);
	} else
		*(mtod(nam, caddr_t) + nam->m_len) = 0;
/* SHOULD BE ABLE TO ADOPT EXISTING AND wakeup() ALA FIFO's */
#ifdef	_AIX_FULLOSF
#if	MACH
	ndp->ni_nameiop = CREATE | FOLLOW | WANTPARENT;
#else
	ndp->ni_nameiop = CREATE | FOLLOW | LOCKPARENT;
#endif
	ndp->ni_segflg = UIO_SYSSPACE;
#if	SEC_BASE
	audstub_unp_bind();
#endif
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (vp != NULL) {
#if	MACH
		VOP_ABORTOP(ndp, error);
		vrele(ndp->ni_dvp);
#else
		VOP_ABORTOP(ndp);
		if (ndp->ni_dvp == vp)
			vrele(ndp->ni_dvp);
		else
			vput(ndp->ni_dvp);
#endif
		vrele(vp);
		return (EADDRINUSE);
	}
#if	MACH
	vattr_null(&vattr);
	vattr.va_type = VSOCK;
	vattr.va_mode = 0777;
	/*
	 * The creation of the socket must be "atomic," so the create
	 * operation needs additional information for the VSOCK case.
	 */
	vattr.va_socket = (char *) unp->unp_socket;
	VOP_CREATE(ndp, &vattr, error);
	if (error)
		return (error);
	vp = ndp->ni_vp;
	if (vp->v_socket != unp->unp_socket)	/* filesystem sanity check */
		panic("unp_bind");
	unp->unp_vnode = vp;
	unp->unp_addr = m_copym(nam, 0, (int)M_COPYALL, M_DONTWAIT);
#else
	VATTR_NULL(&vattr);
	vattr.va_type = VSOCK;
	vattr.va_mode = 0777;
	if (error = VOP_CREATE(ndp, &vattr))
		return (error);
	vp = ndp->ni_vp;
	vp->v_socket = unp->unp_socket;
	unp->unp_vnode = vp;
	unp->unp_addr = m_copym(nam, 0, (int)M_COPYALL, M_DONTWAIT);
	VOP_UNLOCK(vp);
#endif
#else	/* _AIX_FULLOSF */
        bzero(&vattr, sizeof(struct vattr));
        vattr.va_type = VSOCK;
        vattr.va_mode = 0777;
        error = vn_create(soun->sun_path, UIO_SYSSPACE, &vattr, FEXCL, 0, &vp);
        if (error) {
                if (error == EEXIST)
                        return (EADDRINUSE);
		return (error);
	}
	vp->v_socket = unp->unp_socket;
	unp->unp_vnode = vp;
/* Don't forget the null. */
	nam->m_len++;
	unp->unp_addr = m_copy(nam, 0, (int) M_COPYALL); 
	bsdlog_reg(unp->unp_socket);
#endif	/* _AIX_FULLOSF */
	return (0);
}

#ifndef	_AIX_FULLOSF
unp_connect(so, nam)
	struct socket *so;
	struct mbuf *nam;
{
	register struct sockaddr_un *soun = mtod(nam, struct sockaddr_un *);
	struct vnode *vp;
	register struct socket *so2, *so3;
	struct unpcb *unp2;
	int error;
#if	NETSYNC_LOCK
	int so2notso;
#endif
	struct ucred *crp;

	LOCK_ASSERT("unp_connect", SOCKET_ISLOCKED(so));
	if (soun->sun_family != AF_UNIX)
		return (EAFNOSUPPORT);
	if (nam->m_data + nam->m_len == &nam->m_dat[MLEN]) {	/* XXX */
		if (*(mtod(nam, caddr_t) + nam->m_len - 1) != 0)
			return (EMSGSIZE);
	} else
		*(mtod(nam, caddr_t) + nam->m_len) = 0;

	SOCKET_UNLOCK(so);
	crp = crref();
	error = lookupname(soun->sun_path, UIO_SYSSPACE, 0,
                (struct vnode **)0, &vp, crp);
	if (error == 0) {
		if ( (error = VNOP_ACCESS(vp, W_ACC, ACC_SELF, crp)) !=0)
			VNOP_RELE(vp);
	}

	crfree(crp);
        if (error) {
		SOCKET_LOCK(so);
                return (error);
	}

	/* 
	 * Lock vp until both sockets are locked to serialize with 
	 * unp_disconnect...
	 */
	VN_LOCK(vp);
        if (vp->v_type != VSOCK) {
		error = ENOTSOCK;
		VN_UNLOCK(vp);
		goto bad2;
	}
	so2 = vp->v_socket;
	if (so2 == 0 || !so2->so_pcb) {
		error = ECONNREFUSED;
		VN_UNLOCK(vp);
		goto bad2;
	}
	if (so->so_type != so2->so_type) {
		error = EPROTOTYPE;
		VN_UNLOCK(vp);
		goto bad2;
	}
	SOCKET_LOCK2(so, so2);
	VN_UNLOCK(vp);
#if	NETSYNC_LOCK
	so2notso = (so2->so_lock != so->so_lock);
#endif
	if (so->so_proto->pr_flags & PR_CONNREQUIRED) {
		if ((so2->so_options & SO_ACCEPTCONN) == 0) {
			error = ECONNREFUSED;
#if	NETSYNC_LOCK
			if (so2notso)
				SOCKET_UNLOCK(so2);
#endif
			goto bad;
		}
#if	SEC_ARCH
		bcopy(so->so_tag, so2->so_tag, SEC_NUM_TAGS * sizeof(tag_t));
#endif
		unp2 = sotounpcb(so2);	/* Get server name before unlock */
		if (unp2->unp_addr)
			nam = m_copym(unp2->unp_addr, 0, (int)M_COPYALL, M_DONTWAIT);
		else
			nam = 0;
		if ((so3 = sonewconn(so2, 0)) == 0) {
			error = ECONNREFUSED;
			if (nam) m_freem(nam);
#if	NETSYNC_LOCK
			/*
			 * sonewconn() unlocks so2. But if
			 * so and so2 shared the same lock...
			 */
			if (!so2notso)
				goto bad2;
#endif
			goto bad;
		}
#if	NETSYNC_LOCK
		/*
		 * Note that so3 is brand new and has no relation
		 * to so. But sonewconn unlocked the old so2, which
		 * may have shared a lock with so (or have been so
		 * itself!). We thus may need to relock so.
		 */
		if (!so2notso)
			SOCKET_LOCK(so);
#endif
		sotounpcb(so3)->unp_addr = nam;
		so2 = so3;
	}
	error = unp_connect2(so, so2);
	if (error)
		SOCKET_UNLOCK(so2);
	goto bad;
bad2:
	SOCKET_LOCK(so);
bad:
	VNOP_RELE(vp);
	return (error);
}
#endif	/* _AIX_FULLOSF */

unp_connect2(so, so2)
	register struct socket *so;
	register struct socket *so2;
{
	register struct unpcb *unp = sotounpcb(so);
	register struct unpcb *unp2;
	register int error;

	LOCK_ASSERT("unp_connect2 so", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("unp_connect2 so2", SOCKET_ISLOCKED(so2));
	if (so2->so_type != so->so_type)
		return (EPROTOTYPE);
#if	NETSYNC_LOCK
	if (error=solockpair(so, so2))
		return(error);
#endif
	unp2 = sotounpcb(so2);
	unp->unp_conn = unp2;
	switch (so->so_type) {

	case SOCK_DGRAM:
		UNPCONN_LOCK();
		unp->unp_nextref = unp2->unp_refs;
		unp2->unp_refs = unp;
		UNPCONN_UNLOCK();
		soisconnected(so);
		break;

	case SOCK_STREAM:
		unp2->unp_conn = unp;
		soisconnected(so);
		soisconnected(so2);
#ifdef	_AIX_FULLOSF
		if (so->so_special & SP_PIPE) {	/* Posix/AES */
			struct timeval now;
			microtime(&now);
			unp->unp_ctime = unp->unp_atime =
				unp->unp_mtime = now.tv_sec;
			unp2->unp_ctime = unp2->unp_atime =
				unp2->unp_mtime = now.tv_sec;
		}
#endif	/* _AIX_FULLOSF */
		break;

	default:
		panic("unp_connect2");
	}
	return (0);
}

void
unp_disconnect(unp)
	struct unpcb *unp;
{
	register struct unpcb *unp2 = unp->unp_conn;

	if (unp2 == 0)
		return;
#if	NETSYNC_LOCK
	/*
	 * unp->unp_socket is locked on entry... be sure unp2's is also.
	 */
	LOCK_ASSERT("unp_disconnect", SOCKET_ISLOCKED(unp->unp_socket));
	if (unp->unp_socket->so_lock != unp2->unp_socket->so_lock)
		SOCKET_LOCK(unp2->unp_socket);
#endif
	unp->unp_conn = 0;
	switch (unp->unp_socket->so_type) {

	case SOCK_DGRAM:
		UNPCONN_LOCK();
		if (unp2->unp_refs == unp)
			unp2->unp_refs = unp->unp_nextref;
		else {
			unp2 = unp2->unp_refs;
			for (;;) {
				if (unp2 == 0)
					panic("unp_disconnect");
				if (unp2->unp_nextref == unp)
					break;
				unp2 = unp2->unp_nextref;
			}
			unp2->unp_nextref = unp->unp_nextref;
		}
		unp->unp_nextref = 0;
		UNPCONN_UNLOCK();
		unp->unp_socket->so_state &= ~SS_ISCONNECTED;
		break;

	case SOCK_STREAM:
		soisdisconnected(unp->unp_socket);
		unp2->unp_conn = 0;
		soisdisconnected(unp2->unp_socket);
		break;
	}
#if	NETSYNC_LOCK
	if (unp->unp_socket->so_lock != unp2->unp_socket->so_lock)
		SOCKET_UNLOCK(unp2->unp_socket);
#endif
}

#ifdef notdef
void
unp_abort(unp)
	struct unpcb *unp;
{

	unp_detach(unp);
}
#endif

/*ARGSUSED*/
void
unp_shutdown(unp)
	struct unpcb *unp;
{
	struct socket *so;

	if (unp->unp_socket->so_type == SOCK_STREAM && unp->unp_conn &&
	    (so = unp->unp_conn->unp_socket)) {
		LOCK_ASSERT("unp_shutdown", SOCKET_ISLOCKED(so));
		socantrcvmore(so);
	}
}

void
unp_drop(unp, errno)
	struct unpcb *unp;
	int errno;
{
	struct socket *so = unp->unp_socket;

	LOCK_ASSERT("unp_drop", SOCKET_ISLOCKED(so));
	so->so_error = errno;
	unp_disconnect(unp);
	if (so->so_head) {
		UNPCONN_LOCK();
		remque(&unp->unp_queue);
		UNPCONN_UNLOCK();
		so->so_pcb = (caddr_t) 0;
		m_freem(unp->unp_addr);
		NET_FREE(unp, M_PCB);
		sofree(so);
	}
}

#ifdef notdef
void
unp_drain()
{

}
#endif

unp_externalize(rights)
	struct mbuf *rights;
{
	register int i;
	register struct cmsghdr *cm = mtod(rights, struct cmsghdr *);
	register struct file **rp = (struct file **)(cm + 1);
	register struct file *fp;
	int newfds = (cm->cmsg_len - sizeof(*cm)) / sizeof (int);
	int f, last;

	last = 0;
	for (i = 0; i < newfds; i++) {
		if (ufdalloc(last,&f)) {
			/* clean up */
			for (i = 0; i < newfds; i++) {
				fp = *rp;
				FP_LOCK(fp);
				ASSERT(fp->f_count > 0);
				if ((fp)->f_count > 1) {
					(fp)->f_count--;
					FP_UNLOCK(fp);
				} else {
					FP_UNLOCK(fp);
					(void) closef(fp);
				}
				*rp++ = 0;
			}
			return (EMSGSIZE);
		}
		fp = *rp;
		u.u_ofile(f) = fp;
		FP_LOCK(fp);
		fp->f_msgcount--;
		FP_UNLOCK(fp);
		UNPMISC_LOCK();
		unp_rights--;
		UNPMISC_UNLOCK();
		*(int *)rp++ = f;
		last = f;
	}

	return (0);
}

unp_internalize(control)
	struct mbuf *control;
{
	register struct cmsghdr *cm = mtod(control, struct cmsghdr *);
	register struct file **rp;
	struct file *fp;
	register int i, fd, error = 0;
	int oldfds;
	int rc;

	if (cm->cmsg_type != SCM_RIGHTS || cm->cmsg_level != SOL_SOCKET ||
	    cm->cmsg_len != control->m_len) {
		return (EINVAL);
	}
	oldfds = (cm->cmsg_len - sizeof (*cm)) / sizeof (int);
	rp = (struct file **)(cm + 1);
	UNPMISC_LOCK();
	for (i = 0; i < oldfds; i++) {
		rc = *(int *)rp++;
		if (rc < 0 || rc > u.u_maxofile || !u.u_ufd[rc].fp) {
			UNPMISC_UNLOCK();
			return (EBADF);
		}
	}
	rp = (struct file **)(cm + 1);
	for (i = 0; i < oldfds; i++) {
		if (rc = getf(*(int *)rp,&fp)) {
			UNPMISC_UNLOCK();
			return (rc);
		}
		FP_LOCK(fp);
		fp->f_count++;
		fp->f_msgcount++;
		FP_UNLOCK(fp);
		/* Decrement the file descriptor count */
		ufdrele(*(int *)rp);
		*rp++ = fp;
		unp_rights++;
	}
	UNPMISC_UNLOCK();
	return (0);
}

int	unp_defer;
extern	struct domain unixdomain;

struct extra_ref {
	struct file *fp;
	struct extra_ref *fp_next;
};


void
unp_dispose(m)
	struct mbuf *m;
{
	if (m)
		unp_scan(m, unp_discard);
}

void
unp_scan(m0, op)
	register struct mbuf *m0;
	void (*op)(struct file *);
{
	register struct mbuf *m;
	register struct file **rp;
	register struct cmsghdr *cm;
	register int i;
	int qfds;

	while (m0) {
		for (m = m0; m; m = m->m_next)
			if (m->m_type == MT_CONTROL &&
			    m->m_len >= sizeof(*cm)) {
				cm = mtod(m, struct cmsghdr *);
				if (cm->cmsg_level != SOL_SOCKET ||
				    cm->cmsg_type != SCM_RIGHTS)
					continue;
				qfds = (cm->cmsg_len - sizeof *cm)
						/ sizeof (struct file *);
				rp = (struct file **)(cm + 1);
				for (i = 0; i < qfds; i++)
					(*op)(*rp++);
			}
		m0 = m0->m_act;
	}
}

void
unp_mark(fp)
	struct file *fp;
{

	FP_LOCK(fp);
	if (fp->f_flag & GCFMARK) {
		FP_UNLOCK(fp);
		return;
	}
	unp_defer++;
	fp->f_flag |= (GCFMARK|GCFDEFER);
	FP_UNLOCK(fp);
}

void
unp_discard(fp)
	struct file *fp;
{

	FP_LOCK(fp);
	fp->f_msgcount--;
	UNPMISC_LOCK();
	unp_rights--;
	UNPMISC_UNLOCK();
	ASSERT(fp->f_count > 0);
	if ((fp)->f_count > 1) {
		(fp)->f_count--;
		FP_UNLOCK(fp);
	} else {
		FP_UNLOCK(fp);
		(void) closef(fp);
	}

}


/* Slow timeout routine for UNIX domain sockets.
 *
 * The only thing we currently do here is free struct socklocks
 * structures hanging off free_socklocks.
 *
 * First shot at this algorithm:
 *	Grab socklock_free_lock.
 *	while (free_socklocks) {
 *		save free_socklocks.freelist
 *		free free_socklocks
 *		set free_socklocks to saved free_socklocks.freelist
 *	}
 *	Unlock socklock_free_lock.
 *
 * If this doesn't work, try aging the entries on the list by using a
 * free field in struct socklocks as a counter.
 */
void
uipc_slowtimo(void)
{
	register struct socklocks *cur, *next;
	register struct socket *curso, *nextso;

	/* Lock socklock_free_lock. */
	simple_lock(&socklock_free_lock);
	cur = free_socklocks;
	free_socklocks = 0;
	simple_unlock(&socklock_free_lock);

	/* Free items on the list */
	for (; cur; cur = next) {

		next = cur->freelist;
		LOCK_FREE(&cur->sock_lock);
		NET_FREE(cur, M_SOCKET);
	}

	/* Go through free_sockets */
	simple_lock(&free_sockets_lock);
	curso = free_sockets;
	free_sockets = 0;
	simple_unlock(&free_sockets_lock);

	/* Free any sockets on the list whose so->so_spare <= 0. */
	for (; curso; curso = nextso) {

		nextso = curso->so_tpcb;
		
		if (curso->so_spare <= 0) {

			/* Free socklock structure, then socket. */
			struct socklocks *lp = curso->so_lock;

			if (lp) {
				int cnt = --lp->refcnt;
				curso->so_lock = 0;
				curso->so_rcv.sb_lock = curso->so_snd.sb_lock =
					0;
				if (cnt <= 0) {
					LOCK_FREE(&lp->sock_lock);
					NET_FREE(lp, M_SOCKET);
				}
			}
			if ((curso->so_rcv.sb_wakeone != EVENT_NULL) || 
			    (curso->so_snd.sb_wakeone != EVENT_NULL))
				panic("sounlock - freeing socket with sleepers");
			NET_FREE(curso, M_SOCKET);

		} else {

			/* Put it back on the free list. */
			simple_lock(&free_sockets_lock);
			curso->so_tpcb = (caddr_t) free_sockets;
			free_sockets = curso;
			simple_unlock(&free_sockets_lock);
		}
	}

}

