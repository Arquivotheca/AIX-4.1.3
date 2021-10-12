static char sccsid[] = "@(#)98	1.44.2.17  src/bos/kernel/uipc/syscalls.c, sysuipc, bos41J, 9520A_all 5/15/95 11:57:59";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: SYSARGS1
 *		SYSARGS2
 *		SYSARGS3
 *		SYSARGS4
 *		SYSARGS5
 *		SYSARGS6
 *		SYSENT
 *		_accept
 *		_bind
 *		_connect
 *		_getkerninfo
 *		_getpeername
 *		_getsockname
 *		_getsockopt
 *		_listen
 *		_naccept
 *		_ngetpeername
 *		_ngetsockname
 *		_nrecvfrom
 *		_nrecvmsg
 *		_nsendmsg
 *		_recv
 *		_recvfrom
 *		_recvmsg
 *		_send
 *		_sendmsg
 *		_sendto
 *		_setsockopt
 *		_shutdown
 *		_socket
 *		_socketpair
 *		accept1
 *		getpeername1
 *		getsockname1
 *		pipe
 *		recvit
 *		sendit
 *		sockaddr_new
 *		sockaddr_old
 *		sockargs
 *		socksetup
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
/* uipc_syscalls.c	2.1 16:10:53 4/20/90 SecureWare */
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
 * Copyright (c) 1982, 1986, 1989, 1990 Regents of the University of California.
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
 *	Base:	src/bos/kernel/uipc/syscalls.c, sysuipc, bos410, aoot (Berkeley) 7/24/93
 *	Merged: uipc_syscalls.c	7.20 (Berkeley) 6/30/90
 */

#include "net/net_globals.h"
#if	MACH
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#ifdef	_AIX
#include "sys/machine.h"
#include "sys/intr.h"
#include "sys/errno.h"
#include <sys/trchkid.h>
#include <sys/kinfo.h>
#include <sys/uio.h>
#include <sys/fs_locks.h>
#endif	/* _AIX */

#if	MACH
#include "kern/parallel.h"

#if	SEC_ARCH
#include <sys/security.h>
#endif
#endif

#ifdef	_AIX
extern int kinfo_rtable();
extern int kinfo_ndd();
#endif

LOCK_ASSERTL_DECL

#if	defined(COMPAT_43) && !defined(BYTE_ORDER)
#error BYTE_ORDER not defined - necessary for COMPAT_43 networking
#endif

/*
 * System call interface to the socket abstraction.
 */

extern	struct fileops socketops;

#define	SYSARGS1(a)			a
#define	SYSARGS2(a, b)			a, b
#define	SYSARGS3(a, b, c)		a, b, c
#define	SYSARGS4(a, b, c, d)		a, b, c, d
#define	SYSARGS5(a, b, c, d, e)		a, b, c, d, e
#define	SYSARGS6(a, b, c, d, e, f)	a, b, c, d, e, f

#define	SYSENT(name, arglist)				\
name(arglist)						\
{							\
	int retval = 0;					\
							\
	struct args {					\
		int arglist;				\
	} args = {arglist};				\
							\
	u.u_error = _/**/name (NULL, &args, &retval);	\
	return(u.u_error ? -1 : retval);		\
}

SYSENT(socket, SYSARGS3(domain, type, protocol) )
SYSENT(bind, SYSARGS3(s, name, namelen) )
SYSENT(listen, SYSARGS2(s, backlog) )
SYSENT(accept, SYSARGS3(s, name, anamelen) )
SYSENT(naccept, SYSARGS3(s, name, anamelen) )
SYSENT(connect, SYSARGS3(s, name, namelen) )
SYSENT(socketpair, SYSARGS4(domain, type, protocol, rsv) )
SYSENT(sendto, SYSARGS6(s, buf, len, flags, to, tolen) )
SYSENT(send, SYSARGS4(s, buf, len, flags) )
SYSENT(nsendmsg, SYSARGS3(s, uap_msg, flags) )
SYSENT(sendmsg, SYSARGS3(s, uap_msg, flags) )
SYSENT(nrecvfrom, SYSARGS6(s, buf, buf_len, flags, from, fromlenaddr) )
SYSENT(recvfrom, SYSARGS6(s, buf, buf_len, flags, from, fromlenaddr) )
SYSENT(recv, SYSARGS4(s, buf, len, flags) )
SYSENT(nrecvmsg, SYSARGS3(s, uap_msg, flags) )
SYSENT(recvmsg, SYSARGS3(s, uap_msg, flags) )
SYSENT(shutdown, SYSARGS2(s, how) )
SYSENT(setsockopt, SYSARGS5(s, level, name, val, valsize) )
SYSENT(getsockopt, SYSARGS5(s, level, name, val, avalsize) )
SYSENT(getsockname, SYSARGS3(fdes, asa, alen) )
SYSENT(ngetsockname, SYSARGS3(fdes, asa, alen) )
SYSENT(getpeername, SYSARGS3(fdes, asa, alen) )
SYSENT(ngetpeername, SYSARGS3(fdes, asa, alen) )
SYSENT(getkerninfo, SYSARGS4( op, where, size, arg ) )

#define	getsock(fdes, err, fp) {			\
        err = getft(fdes, &(fp), DTYPE_SOCKET);		\
	if (err == EINVAL)				\
		err = ENOTSOCK;				\
}

/* ARGSUSED */
_socket(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	domain;
		int	type;
		int	protocol;
	} *uap = (struct args *) args;
	struct socket *so;
	struct file *fp;
	int fd, error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_socket_in,
		uap->domain,uap->type,uap->protocol);

	if (error = socksetup(&fd, &fp))
		return(error);

	if (error = socreate(uap->domain, &so, uap->type, uap->protocol)) {
		ufdfree(fd);
		crfree(fp->f_cred);

		/*
		 * fpalloc() sets f_count to 1.  fpfree asserts that it's
		 * 0.  Sooo, do it!
	 	 */
		fp->f_count = 0;
		fpfree(fp);
	} else {
		fp->f_data = (struct vnode *)so;
		U.U_ufd[fd].fp = fp;
		*retval = fd;
	}
	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_socket_out,fd,so);
	return (error);
}

/* ARGSUSED */
_bind(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct args *) args;
	struct file *fp;
	struct mbuf *nam;
	int error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_bind_in,uap->s,uap->name,uap->namelen);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (error == 0) {
#if	SEC_BASE
		/*
		 * Save the bound address for auditing.
		 * MP note: This is pendable.
		 */
		audstub_pathname(mtod(nam, caddr_t), nam->m_len);
#endif
		error = sobind((struct socket *)fp->f_data, nam);
		m_freem(nam);
	}
	ufdrele(uap->s);
	return (error);
}

/* ARGSUSED */
_listen(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		int	backlog;
	} *uap = (struct args *) args;
	struct file *fp;
	int error;

	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_listen_in,uap->s,uap->backlog);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	error = solisten((struct socket *)fp->f_data, uap->backlog);
	ufdrele(uap->s);
	return (error);
}

#ifdef COMPAT_43
_naccept(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (accept1(p, args, retval, 0));
}

_accept(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (accept1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define accept1(p, args, retval, compat_43)	accept(p, args, retval) 
#endif

/* ARGSUSED */
accept1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	name;
		int	*anamelen;
	} *uap = (struct args *) args;
	struct file *fp;
	struct file *nfp;
	struct mbuf *nam = 0;
	int namelen=0, error;
	register struct socket *so;
	struct socket *aso;
	int nonblock;
	DOMAIN_FUNNEL_DECL(f)

	TRCHKL3T(HKWD_SYSC_TCPIP|hkwd_accept_in,uap->s,uap->name,uap->anamelen);

	if (uap->name && (error = copyin((caddr_t)uap->anamelen,
	    (caddr_t)&namelen, sizeof (namelen))))
		return (error);

	if (namelen < 0)
		return(EINVAL);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	so = (struct socket *)fp->f_data;
again:
	nonblock = ( (fp->f_flag & (FNDELAY|FNONBLOCK)) ||
			(so->so_state & SS_NBIO) ) ;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto out;
	}
	if (nonblock && so->so_qlen == 0) {
		error = EWOULDBLOCK;
		goto out;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		if (error = sosleep(so, (caddr_t)&so->so_timeo,
		    (PZERO+1) | PCATCH, 0))
			goto out;
	}
	if (so->so_error) {
		error = so->so_error;
		so->so_error = 0;
		goto out;
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);

	/*
	 * We used to do various things BEFORE dequeuing the new socket
	 * from the head, but it creates several race conditions. It is
	 * necessary to get hold of the new socket first. The only thing
	 * that will behave differently is a failure of falloc. Previously
	 * the socket would not be lost.
	 */
	if (error = sodequeue(so, &aso, &nam, compat_43)) {
		/* We may have lost the connection when we unlocked "so". */
		if (error == ENOTCONN)
			goto again;
		goto out2;
	}

#if	SEC_ARCH
	/*
	 * Perform an access check between the prototype socket and
	 * the new one.  The completely obvious choice of SP_IOCTLACC is
	 * to force a MAC-only equality check.  We should probably
	 * define a new access type with equivalent semantics and
	 * more mnemonic value. The audit hook is used to store
	 * the new socket tags (assuming the new socket and the
	 * prototype will be at the same levels.
	 *
	 * It is not necessary to lock the sockets simply to peek at
	 * the (constant) so_tag's.
	 */
	audstub_levels(aso->so_tag);
	if (SP_ACCESS(so->so_tag, aso->so_tag, SP_IOCTLACC, NULL))
		error = EACCES;

	if (error || (error = socksetup(retval, &nfp))) {
#else
	if (error = socksetup(retval, &nfp)) {
#endif
		(void) soabort(aso);
		(void) soclose(aso);
		goto out2;
	}
	nfp->f_data = (struct vnode *)aso;
	nfp->f_flag = fp->f_flag;
	U.U_ufd[*retval].fp = nfp;
#if	SEC_BASE
	/*
	 * Save the client address for auditing
	 * MP note: This is pendable.
	 */
	audstub_pathname(mtod(nam, caddr_t), nam->m_len);
#endif
	if (uap->name) {
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		if ((error = copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen)) == 0)
			error = copyout((caddr_t)&namelen,
			    (caddr_t)uap->anamelen, sizeof (*uap->anamelen));
#ifdef	_AIX
		if (error)
			(void) soclose(aso);
#endif
	}
	m_freem(nam);
	ufdrele(uap->s);
	return (error);

out:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
out2:
	ufdrele(uap->s);
	if (nam)
		m_freem(nam);
	return (error);
}

/* ARGSUSED */
_connect(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct args *) args;
	struct file *fp;
	register struct socket *so;
	struct mbuf *nam;
	int nonblock, error;
	DOMAIN_FUNNEL_DECL(f)

	TRCHKL3T(HKWD_SYSC_TCPIP|hkwd_connect_in,uap->s,uap->name,uap->namelen);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	so = (struct socket *)fp->f_data;
	nonblock = ( (fp->f_flag & (FNDELAY|FNONBLOCK)) ||
		     (so->so_state & SS_NBIO) ) ;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (nonblock && (so->so_state & SS_ISCONNECTING)) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		ufdrele(uap->s);
		return (EALREADY);
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);	/* XXX suboptimal? */
	error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (error) {
		ufdrele(uap->s);
		return (error);
	}
#if	SEC_BASE
	/*
	 * Save the target address for auditing.
	 * MP note: This is pendable.
	 */
	audstub_pathname(mtod(nam, caddr_t), nam->m_len);
#endif
	error = soconnect(so, nam);
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (error)
		goto bad;
	if (nonblock && (so->so_state & SS_ISCONNECTING)) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		ufdrele(uap->s);
		m_freem(nam);
		return (EINPROGRESS);
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		if (error = sosleep(so, (caddr_t)&so->so_timeo,
		    (PZERO+1) | PCATCH, 0))
			goto bad;
	if (error == 0) {
		error = so->so_error;
		so->so_error = 0;
	}
bad:
	so->so_state &= ~SS_ISCONNECTING;
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	ufdrele(uap->s);
	m_freem(nam);
	if (error == ERESTART)
		error = EINTR;
	return (error);
}

/* ARGSUSED */
_socketpair(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	domain;
		int	type;
		int	protocol;
		int	*rsv;
	} *uap = (struct args *) args;
	struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int fd, error, sv[2];
	lock_t	lockt;

	TRCHKL4T(HKWD_SYSC_TCPIP | hkwd_socketpair_in,uap->domain,uap->type,
		uap->protocol,uap->rsv);

	if (uap->domain != AF_UNIX)
		return(EOPNOTSUPP);

	if (error = socreate(uap->domain, &so1, uap->type, uap->protocol))
		return (error);
	if (error = socreate(uap->domain, &so2, uap->type, uap->protocol))
		goto free1;
	if (error = socksetup(&fd, &fp1))
		goto free2;
	sv[0] = fd;
	fp1->f_data = (struct vnode *)so1;
	U.U_ufd[fd].fp = fp1;
	if (error = socksetup(&fd, &fp2)) {
		(void)soclose(so2); /* cleanup before exit */
		goto free3;
	}
	fp2->f_data = (struct vnode *)so2;
	U.U_ufd[fd].fp = fp2;
	sv[1] = fd;
	if (error = soconnect2(so1, so2))
		goto free4;
	if (uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 if (error = soconnect2(so2, so1))
			goto free4;
	}
	if (error = copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int)))
		goto free4;
	TRCHKL4T(HKWD_SYSC_TCPIP | hkwd_socketpair_out,sv[0],sv[1],so1,so2);
	return (0);
free4:
	close(sv[1]);
free3:
	close(sv[0]);
	return (error);
free2:
	(void)soclose(so2);
free1:
	(void)soclose(so1);
	return (error);
}

/* ARGSUSED */
_sendto(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	to;
		int	tolen;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;

	TRCHKL5T(HKWD_SYSC_TCPIP | hkwd_sendto_in,uap->s,uap->buf,uap->len,
		uap->flags,uap->to);

	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
#ifdef COMPAT_43
	msg.msg_flags = 0;
#endif
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	return (sendit(uap->s, &msg, uap->flags, retval));
}

#ifdef COMPAT_43
/* ARGSUSED */
_send(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct args *) args;
	struct iovec aiov;
	struct uio auio;
	struct file *fp;
	int error;

	TRCHKL4T(HKWD_SYSC_TCPIP | hkwd_send_in,uap->s,uap->buf,uap->len,
		uap->flags);
	
	getsock(uap->s, error, fp);
	if (error)
		return (error);

	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = uap->len;
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		uap->flags |= MSG_NONBLOCK;

	/*
	 * If the protocol knows best how to send, then let it send!
	 */
#define so ((struct socket *)fp->f_data)
	if (so->so_proto->pr_send)
		error = (*so->so_proto->pr_send)(so, (struct mbuf *)0, &auio,
			(struct mbuf *)0, (struct mbuf *)0, uap->flags);
	else
		error = sosend(so, (struct mbuf *)0, &auio, (struct mbuf *)0,
			(struct mbuf *)0, uap->flags);
#undef so
	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at sosend() */
			if (auio.uio_resid != uap->len)
				error = 0;
			break;
		case EPIPE:
			pidsig(getpid(), SIGPIPE);
			break;
		}
	}
	if (error == 0)
		*retval = uap->len - auio.uio_resid;
	ufdrele(uap->s);
	return (error);
}

/* ARGSUSED */
_sendmsg(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct args *) args;

	struct msghdr msg;
	struct iovec aiov[UIO_SMALLIOV], *iov;
	int error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_sendmsg_in,uap->s,uap->msg,uap->flags);

	if (error = copyin(uap->msg, (caddr_t)&msg, sizeof(struct omsghdr)))
		return (error);
	if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
		if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
			return (EMSGSIZE);
		NET_MALLOC(iov, struct iovec *,
			sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
			M_WAITOK);
	} else
		iov = aiov;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0]))
		return (EMSGSIZE);
	if ((error = copyin((caddr_t)msg.msg_iov, (caddr_t)iov,
	     (unsigned)(msg.msg_iovlen * sizeof (struct iovec)))) == 0) {
		msg.msg_flags = MSG_COMPAT;
		msg.msg_iov = iov;
		error = sendit(uap->s, &msg, uap->flags, retval);
	}
	if (iov != aiov)
		NET_FREE(iov, M_IOV);
	return (error);

}
#endif

/* ARGSUSED */
_nsendmsg(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[UIO_SMALLIOV], *iov;
	int error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_sendmsg_in,uap->s,uap->msg,uap->flags);

	if (error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg)))
		return (error);
	if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
		if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
			return (EMSGSIZE);
		NET_MALLOC(iov, struct iovec *,
			sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
			M_WAITOK);
	} else
		iov = aiov;
	if (msg.msg_iovlen &&
	    ((error = copyin((caddr_t)msg.msg_iov, (caddr_t)iov,
	     (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))) == 0) {
		msg.msg_iov = iov;
#ifdef COMPAT_43
		msg.msg_flags = 0;
#endif
		error = sendit(uap->s, &msg, uap->flags, retval);
	}
	if (iov != aiov)
		NET_FREE(iov, M_IOV);
	return (error);
}

sendit(s, mp, flags, retsize)
	int s;
	register struct msghdr *mp;
	int flags, *retsize;
{
	struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *control;
	int len, error;
	
	getsock(s, error, fp);
	if (error)
		return (error);
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	to = control = 0;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
#ifdef	_AIX
		if (iov->iov_len == 0)
			continue;
#endif
		if (iov->iov_len < 0 || (auio.uio_resid += iov->iov_len) < 0) {
			error = EINVAL;
			goto bad;
		}
	}
	if (mp->msg_name) {
		if (error = sockargs(&to, mp->msg_name, mp->msg_namelen,
		    MT_SONAME))
			goto bad;
	}
	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_43
		    && mp->msg_flags != MSG_COMPAT
#endif
		) {
			error = EINVAL;
			goto bad;
		}
		if (error = sockargs(&control, mp->msg_control,
		    mp->msg_controllen, MT_CONTROL))
			goto bad;
#ifdef COMPAT_43
		if (mp->msg_flags == MSG_COMPAT) {
			register struct cmsghdr *cm;

			M_PREPEND(control, sizeof(*cm), M_WAIT);
			if (control == 0) {
				error = ENOBUFS;
				goto bad;
			} else {
				cm = mtod(control, struct cmsghdr *);
				cm->cmsg_len = control->m_len;
				cm->cmsg_level = SOL_SOCKET;
				cm->cmsg_type = SCM_RIGHTS;
			}
		}
#endif
	}
#if	SEC_ARCH
	if (error = sec_internalize_rights(&control))
		goto bad;
#endif
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags |= MSG_NONBLOCK;
	len = auio.uio_resid;

	/*
	 * If the protocol knows best how to send, then let it send!
	 */
#define so ((struct socket *)fp->f_data)
	if (so->so_proto->pr_send)
		error = (*so->so_proto->pr_send)((struct socket *)fp->f_data, 
				to, &auio, (struct mbuf *)0, control, flags);
	else
#undef so
		error = sosend((struct socket *)fp->f_data, to, &auio,
				(struct mbuf *)0, control, flags);
	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at sosend() */
			if (auio.uio_resid != len)
				error = 0;
			break;
		case EPIPE:
#ifndef	_AIX
			psignal_inthread(u.u_procp, SIGPIPE);
#else
			pidsig(getpid(), SIGPIPE);
#endif
			break;
		}
	}
	if (error == 0)
		*retsize = len - auio.uio_resid;
bad:
	ufdrele(s);
	if (to)
		m_freem(to);
	return (error);
}

#ifdef COMPAT_43
_recvfrom(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct args *) args;

	uap->flags |= MSG_COMPAT;
	return (_nrecvfrom(p, args, retval));
}
#endif

/* ARGSUSED */
_nrecvfrom(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;
	int error;

	TRCHKL5T(HKWD_SYSC_TCPIP | hkwd_recvfrom_in,uap->s,uap->buf,uap->len,
		uap->flags,uap->from);

	if (uap->fromlenaddr) {
		if (error = copyin((caddr_t)uap->fromlenaddr,
		    (caddr_t)&msg.msg_namelen, sizeof (msg.msg_namelen)))
			return (error);
	} else
		msg.msg_namelen = 0;
	msg.msg_name = uap->from;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = uap->flags;
	return (recvit(uap->s, &msg, (caddr_t)uap->fromlenaddr, retval));
}

#ifdef COMPAT_43
/* ARGSUSED */
_recv(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;
	struct file *fp;
	struct uio auio;
	int error, len;

	TRCHKL4T(HKWD_SYSC_TCPIP | hkwd_recv_in,uap->s,uap->buf,uap->len,
		uap->flags);

	getsock(uap->s, error, fp);
	if (error)
		return (error);

	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = uap->len;
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		uap->flags |= MSG_NONBLOCK;

#define so ((struct socket *)fp->f_data)
	if (so->so_proto->pr_receive)
		error = (*so->so_proto->pr_receive) (so, (struct mbuf **)0,
			&auio, (struct mbuf **)0, (struct mbuf **)0,
			&uap->flags);
	else 
		error = soreceive(so, (struct mbuf **)0, &auio,
			(struct mbuf **)0, (struct mbuf **)0, &uap->flags);
#undef so
	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at soreceive() */
			if (auio.uio_resid != uap->len)
				error = 0;
			break;
		}
	}
	if (error == 0)
		*retval = uap->len - auio.uio_resid;
	ufdrele(uap->s);
	return (error);
}

/*
 * Old recvmsg.  This code takes advantage of the fact that the old msghdr
 * overlays the new one, missing only the flags, and with the (old) access
 * rights where the control fields are now.
 */
/* ARGSUSED */
_recvmsg(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		struct	omsghdr *msg;
		int	flags;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[UIO_SMALLIOV], *iov;
	int rightslen, error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_recvmsg_in,uap->s,uap->msg,uap->flags);

	if (error = copyin((caddr_t)uap->msg, (caddr_t)&msg,
	    sizeof (struct omsghdr)))
		return (error);
	if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
		if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
			return (EMSGSIZE);
		NET_MALLOC(iov, struct iovec *,
			sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
			M_WAITOK);
	} else
		iov = aiov;
	rightslen = msg.msg_controllen;
	msg.msg_flags = uap->flags | MSG_COMPAT;
	if (error = copyin((caddr_t)msg.msg_iov, (caddr_t)iov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		goto done;
	msg.msg_iov = iov;
	error = recvit(uap->s, &msg, (caddr_t)&uap->msg->msg_namelen, retval);

	if (rightslen != msg.msg_controllen && error == 0)
		error = copyout((caddr_t)&msg.msg_controllen,
		    (caddr_t)&uap->msg->msg_accrightslen, sizeof (int));
done:
	if (iov != aiov)
		NET_FREE(iov, M_IOV);
	return (error);
}
#endif

/* ARGSUSED */
_nrecvmsg(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		struct	msghdr *msg;
		int	flags;
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[UIO_SMALLIOV], *uiov, *iov;
	register int error;

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_recvmsg_in,uap->s,uap->msg,uap->flags);

	if (error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg)))
		return (error);
	if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
		if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
			return (EMSGSIZE);
		NET_MALLOC(iov, struct iovec *,
			sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
			M_WAITOK);
	} else
		iov = aiov;
#ifdef COMPAT_43
	msg.msg_flags = uap->flags &~ MSG_COMPAT;
#else
	msg.msg_flags = uap->flags;
#endif
	uiov = msg.msg_iov;
	msg.msg_iov = iov;
	if (error = copyin((caddr_t)uiov, (caddr_t)iov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		goto done;
	if ((error = recvit(uap->s, &msg, (caddr_t)0, retval)) == 0) {
		msg.msg_iov = uiov;
		error = copyout((caddr_t)&msg, (caddr_t)uap->msg, sizeof(msg));
	}
done:
	if (iov != aiov)
		NET_FREE(iov, M_IOV);
	return (error);
}

recvit(s, mp, namelenp, retsize)
	int s;
	register struct msghdr *mp;
	caddr_t namelenp;
	int *retsize;
{
	struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	int len, error;
	struct mbuf *from = 0, *control = 0;
	
	getsock(s, error, fp);
	if (error)
		return (error);

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
#ifdef	_AIX
		if (iov->iov_len == 0)
			continue;
#endif
		if (iov->iov_len < 0 || (auio.uio_resid += iov->iov_len) < 0) {
			ufdrele(s);
			return (EINVAL);
		}
	}
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		mp->msg_flags |= MSG_NONBLOCK;
	len = auio.uio_resid;
#define so ((struct socket *)fp->f_data)
	if (so->so_proto->pr_receive)
		error = (*so->so_proto->pr_receive) (so, &from,
			&auio, (struct mbuf **)0, &control, &mp->msg_flags);
	else 
		error = soreceive(so, &from, &auio,
			(struct mbuf **)0, &control, &mp->msg_flags);
#undef so
	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at soreceive() */
			if (auio.uio_resid != len)
				error = 0;
			break;
		}
	}
	if (error)
		goto out;
	*retsize = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
#ifdef COMPAT_43
			if (mp->msg_flags & MSG_COMPAT)
				mtod(from, struct osockaddr *)->sa_family =
				    mtod(from, struct sockaddr *)->sa_family;
#endif
			if (len > from->m_len)
				len = from->m_len;
			/* else if len < from->m_len ??? */
			if (error = copyout(mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len))
				goto out;
		}
		mp->msg_namelen = len;
		if (namelenp &&
		    (error = copyout((caddr_t)&len, namelenp, sizeof (int)))) {
#ifdef COMPAT_43
			if (mp->msg_flags & MSG_COMPAT)
				error = 0;	/* old recvfrom didn't check */
			else
#endif
			goto out;
		}
	}
	if (mp->msg_control) {
#if	SEC_ARCH
		sec_externalize_rights(&control, (struct socket *) fp->f_data);
#endif
#ifdef	COMPAT_43
		/*
		 * We assume that old recvmsg calls won't receive access
		 * rights and other control info, esp. as control info
		 * is always optional and those options didn't exist in 4.3.
		 * If we receive rights, trim the cmsghdr; anything else
		 * is tossed.
		 */
		if (control && mp->msg_flags & MSG_COMPAT) {
			if (mtod(control, struct cmsghdr *)->cmsg_level !=
			    SOL_SOCKET ||
			    mtod(control, struct cmsghdr *)->cmsg_type !=
			    SCM_RIGHTS) {
				mp->msg_controllen = 0;
				goto out;
			}
			control->m_len -= sizeof (struct cmsghdr);
			control->m_data += sizeof (struct cmsghdr);
		}
#endif
		len = mp->msg_controllen;
		if (len <= 0 || control == 0)
			len = 0;
		else {
			if (len >= control->m_len)
				len = control->m_len;
			else
				mp->msg_flags |= MSG_CTRUNC;
			error = copyout((caddr_t)mtod(control, caddr_t),
			    (caddr_t)mp->msg_control, (unsigned)len);
		}
		mp->msg_controllen = len;
	}
out:
	ufdrele(s);
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);
	return (error);
}

/* ARGSUSED */
_shutdown(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		int	how;
	} *uap = (struct args *) args;
	struct file *fp;
	int error;

	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_shutdown_in,uap->s,uap->how);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	/*
	 * Decrementing fp's reference count can result in a call to
	 * soclose when racing another thread calling close(uap->s).
	 * Defer possibility until after calling soshutdown.
	 */
	if (((struct socket *)fp->f_data)->so_state & SS_ISCONNECTED) 
		error = soshutdown((struct socket *)fp->f_data, uap->how);
	else
		error = ENOTCONN;
	ufdrele(uap->s);
	return (error);
}

/* ARGSUSED */
_setsockopt(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	valsize;
	} *uap = (struct args *) args;
	struct file *fp;
	struct mbuf *m = NULL;
	int error;

	TRCHKL5T(HKWD_SYSC_TCPIP | hkwd_setsockopt_in,uap->s,uap->level,
		uap->name,uap->val,uap->valsize);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	if (uap->valsize > MLEN || uap->valsize < 0) {
		ufdrele(uap->s);
		return (EINVAL);
	}
	if (uap->val) {
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			ufdrele(uap->s);
			return (ENOBUFS);
		}
		if (error = copyin(uap->val, mtod(m, caddr_t),
		    (u_int)uap->valsize)) {
			(void) m_free(m);
			ufdrele(uap->s);
			return (error);
		}
		m->m_len = uap->valsize;
	}
	error = sosetopt((struct socket *)fp->f_data, uap->level, uap->name, m);
	ufdrele(uap->s);
	return (error);
}

/* ARGSUSED */
_getsockopt(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	*avalsize;
	} *uap = (struct args *) args;
	struct file *fp;
	struct mbuf *m = NULL;
	int valsize, error;

	TRCHKL5T(HKWD_SYSC_TCPIP | hkwd_getsockopt_in,uap->s,uap->level,
		uap->name,uap->val,uap->avalsize);

	getsock(uap->s, error, fp);
	if (error)
		return (error);
	if (uap->val) {
		if (error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
		    sizeof (valsize))) {
			ufdrele(uap->s);
			return (error);
		}
	} else
		valsize = 0;
	if ((error = sogetopt((struct socket *)fp->f_data, uap->level,
	    uap->name, &m)) == 0 && uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
		error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
		if (error == 0)
			error = copyout((caddr_t)&valsize,
			    (caddr_t)uap->avalsize, sizeof (valsize));
	}
	ufdrele(uap->s);
	if (m)
		m_freem(m);
	return (error);
}

#ifndef	_AIX
/* ARGSUSED */
pipe(p, args, retval)
	struct proc *p;
	void *args;
	int retval[];
{
	struct file *rf, *wf;
	struct socket *rso, *wso;
	int fd, error;

	if (error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0))
		return (error);
	if (error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0))
		goto free1;
	/* Need write atomicity and pipe times behavior */
	wso->so_special |= (SP_WATOMIC|SP_PIPE);
	rso->so_special |= SP_PIPE;
	if (error = falloc(&rf, &fd))
		goto free2;
	retval[0] = fd;
	FP_LOCK(rf);
	rf->f_flag = FREAD|FWRITE;
	rf->f_type = DTYPE_SOCKET;
	rf->f_ops = &socketops;
	rf->f_data = (caddr_t)rso;
#if	SER_COMPAT
	rf->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(rf);
	if (error = falloc(&wf, &fd))
		goto free3;
	FP_LOCK(wf);
	wf->f_flag = FREAD|FWRITE;
	wf->f_type = DTYPE_SOCKET;
	wf->f_ops = &socketops;
	wf->f_data = (caddr_t)wso;
#if	SER_COMPAT
	wf->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(wf);
	retval[1] = fd;
	if (error = soconnect2(wso, rso))
		goto free4;
	U_FD_SET(retval[0], rf, u.utask->uu_fd);
	U_FD_SET(retval[1], wf, u.utask->uu_fd);
	return (0);
free4:
	U_FD_SET(retval[1], NULL, u.utask->uu_fd);
	ffree(wf);
free3:
	U_FD_SET(retval[0], NULL, u.utask->uu_fd);
	ffree(rf);
free2:
	(void)soclose(wso);
free1:
	(void)soclose(rso);
	return (error);
}
#endif

/*
 * Get socket name.
 */
#ifdef COMPAT_43
_ngetsockname(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (getsockname1(p, args, retval, 0));
}

_getsockname(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (getsockname1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define getsockname1(p, args, retval, compat_43) getsockname(p, args, retval)
#endif

/* ARGSUSED */
getsockname1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	int *retval;
{
	register struct args {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct args *) args;
	struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len, error;
	DOMAIN_FUNNEL_DECL(f)

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_getsockname_in,uap->fdes,uap->asa,
		uap->alen);

	getsock(uap->fdes, error, fp);
	if (error)
		return (error);
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len))) {
		ufdrele(uap->fdes);
		return (error);
	}
	so = (struct socket *)fp->f_data;
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		ufdrele(uap->fdes);
		return (ENOBUFS);
	}
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR,
				(struct mbuf *)0, m, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (error == 0)
		error = copyout((caddr_t)&len, (caddr_t)uap->alen,
		    sizeof (len));
bad:
	ufdrele(uap->fdes);
	m_freem(m);
	return (error);
}

/*
 * Get name of peer for connected socket.
 */
#ifdef COMPAT_43
_ngetpeername(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (getpeername1(p, args, retval, 0));
}

_getpeername(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	return (getpeername1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define getpeername1(p, args, retval, compat_43) getpeername(p, args, retval)
#endif

/* ARGSUSED */
getpeername1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	int *retval;
{
	struct args {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct args *) args;
	struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len, error;
	DOMAIN_FUNNEL_DECL(f)

	TRCHKL3T(HKWD_SYSC_TCPIP | hkwd_getpeername_in,uap->fdes,uap->asa,
		uap->alen);

	getsock(uap->fdes, error, fp);
	if (error)
		return (error);
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		ufdrele(uap->fdes);
		return (ENOBUFS);
	}
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len)))
		goto bad;
	so = (struct socket *)fp->f_data;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		error = ENOTCONN;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR,
				(struct mbuf *)0, m, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (error == 0)
		error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof(len));
bad:
	ufdrele(uap->fdes);
	m_freem(m);
	return (error);
}

void
sockaddr_new(m)
	struct mbuf *m;
{
	if (m->m_type == MT_SONAME) {
		register struct sockaddr *sa = mtod(m, struct sockaddr *);
#if	defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
		if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
			sa->sa_family = sa->sa_len;
#endif
		sa->sa_len = m->m_len;
	}
}

void
sockaddr_old(m)
	struct mbuf *m;
{
#ifdef	COMPAT_43
	if (m->m_type == MT_SONAME) {
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
	}
#endif
}

sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	int buflen, type;
{
	register struct mbuf *m;
	int error;

	if ((u_int)buflen > MLEN) {
#ifdef COMPAT_43
		if (type == MT_SONAME && (u_int)buflen <= 112)
			buflen = MLEN;		/* unix domain compat. hack */
		else
#endif
		return (EINVAL);
	}
	m = m_getclr(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = buflen;
#ifdef COMPAT_43
	if (type == MT_CONTROL) {
		if ((u_int)buflen > MLEN - sizeof (struct cmsghdr)) {
			(void) m_free(m);
			return (EINVAL);
		}
		m->m_data += sizeof (struct cmsghdr);
	}
#endif
	error = 0;
	if (copyin(buf, mtod(m, caddr_t), (u_int)buflen)) {
		error = EFAULT;
		(void) m_free(m);
	} else {
		sockaddr_new(*mp = m);
	}
	return (error);
}

/*
 * socksetup -	common socket setup.  It allocates the file desciptor and
 * the file table entry, yet does NOT associate the two.  This is left to 
 * the caller to do.
 */
socksetup(fdp, fpp)
int *fdp;
struct file **fpp;
{
	int	error;
	struct ucred *crp;

	error = ufdalloc(0, fdp);
	if (error)
		return(error);
        error = fpalloc (NULL, FREAD|FWRITE, DTYPE_SOCKET, &socketops, fpp);
	if (error) {
		ufdfree(*fdp);
                return(error);
	}
	crp = crref();
	(*fpp)->f_cred = crp;
	return(0);
}

/* ARGSUSED */
_getkerninfo(p, args, retval)
	struct proc *p;
	void *args;	
	int *retval;
{
	struct args {
		int	op;
		char	*where;
		int	*size;
		int	arg;
	} *uap = (struct args *) args;

	int	bufsize,	/* max size of users buffer */
		needed,	locked, (*server)(), error = 0;

	if (error = copyin((caddr_t)uap->size, (caddr_t)&bufsize, 
	    sizeof (bufsize)))
		goto done;

	switch (ki_type(uap->op)) {

	case KINFO_RT:
		server = kinfo_rtable;
		break;

	case KINFO_NDD:
		server = kinfo_ndd;
		break;

	default:
		error = EINVAL;
		goto done;
	}
	if (uap->where == NULL || uap->size == NULL) {
		error = (*server)(uap->op, NULL, NULL, uap->arg, &needed);
		goto done;
	}
	/*
	 * lock down target pages
	 */
	if (error = pinu (uap->where, bufsize, UIO_USERSPACE))
	    goto done;
	locked = bufsize;

#ifdef	ACCESS
	if (!useracc(uap->where, bufsize, B_WRITE))
		snderr(EFAULT);
#endif	/* ACCESS */
	error = (*server)(uap->op, uap->where, &bufsize, uap->arg, &needed);
	(void) unpinu (uap->where, locked, UIO_USERSPACE);
	if (error == 0)
		error = copyout((caddr_t)&bufsize,
				(caddr_t)uap->size, sizeof (bufsize));
done:
	if (!error)
		*retval = needed;
	return(error);
}
