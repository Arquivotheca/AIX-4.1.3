static char sccsid[] = "@(#)97	1.26.1.12  src/bos/kernel/uipc/sys_sock.c, sysuipc, bos41J, 9523A_all 6/2/95 15:01:58";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: IOCGROUP
 *		find_netopt
 *		soo_close
 *		soo_ioctl
 *		soo_read
 *		soo_rw
 *		soo_select
 *		soo_stat
 *		soo_write
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
/* @(#)sys_socket.c	2.1 16:10:28 4/20/90 SecureWare, Inc. */
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
 */
/*
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
 *	Base:	sys_socket.c	7.5 (Berkeley) 5/9/89
 *	Merged: sys_socket.c	7.8 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#if	_AIX_FULLOSF
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/ioctl.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/stat.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

LOCK_ASSERTL_DECL

#ifdef	_AIX
#define	IOCGROUP(x)	(((x) >> 8) & 0xff)
#include "sys/errno.h"
#include "sys/poll.h"
#include <sys/fs_locks.h>
#include "net/netopt.h"
struct netopt *netopts = NULL;		/* for the network option ioctl's */
#ifdef SIOCSX25XLATE
#include "net/if.h"		/* pick up definition of struct oifreq */
#endif
#endif	/* _AIX */

#ifdef	_AIX_FULLOSF
CONST struct	fileops socketops =
    { soo_read, soo_write, soo_ioctl, soo_select, soo_close };
#else	/*_AIX_FULLOSF */
int	soo_rw(), soo_ioctl(), soo_select(), soo_close(), soo_stat();
struct	fileops socketops =
    { soo_rw, soo_ioctl, soo_select, soo_close, soo_stat };

soo_rw(fp, rw, uio, ext)
	struct file *fp;
	enum uio_rw rw;
	struct uio *uio;
	int ext;
{
	return ( (*(rw==UIO_READ?soo_read:soo_write)) (fp, uio, 0));
}

#endif	/*_AIX_FULLOSF */

/* ARGSUSED */
soo_read(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
	int flags = 0;
	int len;
	int error;

	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags = MSG_NONBLOCK;

	len = uio->uio_resid;
	/*
	 * If the protocol knows best how to receive, then let it receive!
	 */
#define so ((struct socket *)fp->f_data)

	if (so->so_proto->pr_receive)
		error = (*so->so_proto->pr_receive) (so, (struct mbuf **)0,
			uio, (struct mbuf **)0, (struct mbuf **)0, &flags);
	else 
		error = soreceive(so, (struct mbuf **)0, uio,
			(struct mbuf **)0, (struct mbuf **)0, &flags);
#undef so

	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at soreceive() */
			if (uio->uio_resid != len)
				error = 0;
			break;
		}
	}
	return(error);
}

/* ARGSUSED */
soo_write(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
	int error;
	int flags = 0;
	int len;

	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags = MSG_NONBLOCK;

	len = uio->uio_resid;

	/*
	 * If the protocol knows best how to send, then let it send!
	 */
#define so ((struct socket *)fp->f_data)

	if (so->so_proto->pr_send)
		error = (*so->so_proto->pr_send)(so, (struct mbuf *) 0, uio, 
			(struct mbuf *)0, (struct mbuf *)0, flags);
	else
		error = sosend(so, (struct mbuf *)0, uio,
			(struct mbuf *)0, (struct mbuf *)0, flags);
#undef so

	if (error) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at sosend() */
			if (uio->uio_resid != len)
				error = 0;
			break;
		case EPIPE:
			pidsig(getpid(), SIGPIPE);
			break;
		}
	}
	return(error);
}

soo_ioctl(fp, cmd, udatap)
	struct file *fp;
	int cmd;
	register caddr_t udatap;
{
	register struct socket *so = (struct socket *)fp->f_data;
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)
#ifdef	_AIX
	struct optreq op, *usop;
	struct netopt *nop;
	char buf[sizeof(struct optreq)];
#endif	/* _AIX */
#ifndef	_AIX_FULLOSF
	register u_int size;
#define STK_PARAMS 128
	char stkbuf[STK_PARAMS];
	register caddr_t data = stkbuf;

	/*
	 * Interpret high order word to find amount of data to be
	 * copied to/from the user's address space.
	 */
#ifndef IOCPARM_LEN
#define IOCPARM_LEN(cmd) (((cmd)>>16)&IOCPARM_MASK)
#endif
	switch (cmd) {
	case SIOCDNETOPT: case SIOCGNETOPT: case SIOCSNETOPT:
		/* In this case, we know that size is bogus because
		 * sizeof(struct optreq) is larger than IOCPARM_MASK.  So
		 * just set data to udatap and let the code for
		 * SIOC[DGS]NETOPT do the copyin/copyout.  Set size to 0 so
		 * we don't do copyout() at the end.
		 */

	case SIOCSX25XLATE: case SIOCGX25XLATE: case SIOCDX25XLATE:
		/* X.25 and HIPPI are written so they do the
		 * copyin/copyout, and it's easier to fix this here.
		 * So set data to udatap and size to 0.  This way we
		 * don't do either the copyin() or copyout(), and the
		 * code below that calls ifioctl() doesn't change either.
		 */
		data = udatap;
		size = 0;
		break;

	default:
		size = IOCPARM_LEN(cmd);

		if (cmd & IOC_IN)
			
			if (size) {
				if (copyin(udatap, data, size))
					return EFAULT;
			} else
				*(caddr_t *) data = udatap;

		else if ((cmd & IOC_OUT) && size)

			/* Zero the buffer so the user always gets back
			 * something deterministic.
			 */
			bzero(data, size);

		else if (cmd & IOC_VOID)

			*(caddr_t *) data = udatap;

		else
			/* In this case, the command is "irregular", so we
			 * want to pass the actual address down to the lower
			 * layers, and let them deal with doing coypin/copyout
			 * as necessary.
			 */
			data = udatap;
	}
#endif	/* _AIX_FULLOSF */

	
	DOMAIN_FUNNEL(sodomain(so), f);
	switch (cmd) {

	case FIONBIO:
		if (so->so_special & SP_PIPE)	/* Handled by soo_read/write */
			break;
		if (*(int *)data) { 
			so->so_state |= SS_NBIO;
			if (!(fp->f_flag & (FNDELAY|FNONBLOCK))) { /* sockets knows best. */
				FP_LOCK(fp);
				fp->f_flag |= (FNDELAY|FNONBLOCK);
				FP_UNLOCK(fp);
			}
		} else {
			so->so_state &= ~SS_NBIO;
			if (fp->f_flag & (FNDELAY|FNONBLOCK)) { /* sockets knows best. */
				FP_LOCK(fp);
				fp->f_flag &= ~(FNDELAY|FNONBLOCK);
				FP_UNLOCK(fp);
			}
		}
		break;

	case FIOASYNC:
		SOCKBUF_LOCK(&so->so_rcv);
		SOCKBUF_LOCK(&so->so_snd);
		if (*(int *)data) {
			so->so_state |= SS_ASYNC;
			so->so_rcv.sb_flags |= SB_ASYNC;
			so->so_snd.sb_flags |= SB_ASYNC;
		} else {
			so->so_state &= ~SS_ASYNC;
			so->so_rcv.sb_flags &= ~SB_ASYNC;
			so->so_snd.sb_flags &= ~SB_ASYNC;
		}
		SOCKBUF_UNLOCK(&so->so_snd);
		SOCKBUF_UNLOCK(&so->so_rcv);
		break;

	case FIONREAD:
#if	SEC_ARCH
		*(int *)data = sec_sobufcount(&so->so_rcv, so);
#else
		SOCKBUF_LOCK(&so->so_rcv);
		*(int *)data = so->so_rcv.sb_cc;
		SOCKBUF_UNLOCK(&so->so_rcv);
#endif
		break;

	case SIOCSPGRP:
		so->so_pgid = *(int *)data;
		break;

	case SIOCGPGRP:
		*(int *)data = so->so_pgid;
		break;

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		break;

#ifdef	_AIX
	/* return the value of the network option */
	case SIOCGNETOPT:
		if (netopts == 0) {
			error = ENOENT; 
			break;
		}
		usop = (struct optreq *) data;
		if (copyin (data, &op, sizeof(struct optreq))) {
			error = EFAULT; 
			break;
		}
		if (op.name[0] == 0) { /* find first netopt in list */
			nop = netopts;
			if (copyout(netopts->name, usop->name,
				strlen(netopts->name)+1)) {
					error = EFAULT;
					break;
			}
		} else {
			if ((nop = find_netopt(op.name)) == 0) {
				error = ENOENT;
				break;
			}
			if (op.getnext) {
				if ((nop = nop->next) == 0) {
					error = ENOENT;
					break;
				}
				if (copyout(nop->name, usop->name,
					strlen(nop->name)+1)) {
						error = EFAULT;
						break;
				}
			}
		}
		sprintf(buf, nop->format, *(int *)nop->obj);
		if (copyout(buf, usop->data, strlen(buf)+1))
			error = EFAULT;
		break;

	/* set the value of the network option */
 	case SIOCSNETOPT:
		if (copyin (data, &op, sizeof(struct optreq))) {
			error = EFAULT;
			break;
		}

		if ((nop = find_netopt(op.name)) == 0) {
			error = ENOENT;
			break;
		}
		if (nop->init != NULL)
			error = (*nop->init)(op.data, nop);
		else
			bcopy((caddr_t)op.data, nop->obj, nop->size);
		break;

	/* set the value of the network option to its default */
 	case SIOCDNETOPT:
		if (copyin (data, &op, sizeof(struct optreq))) {
			error = EFAULT;
			break;
		}
		if ((nop = find_netopt(op.name)) == 0) {
			error = ENOENT;
			break;
		}
		if (nop->init != NULL)
			error = (*nop->init)(nop->deflt, nop);
		else
			bcopy(nop->deflt, nop->obj, nop->size);
		break;
#endif	/* _AIX */

	default:
		SOCKET_LOCK(so);
		/*
		 * Interface/routing/protocol specific ioctls:
		 * interface and routing ioctls should have a
		 * different entry since a socket's unnecessary
		 * However, socket SS_PRIV bit serves as auth.
		 */
		sopriv(so);
		SOCKET_UNLOCK(so);
		if (IOCGROUP(cmd) == 'i') {
			error = ifioctl(so, cmd, data);
		}
		else if (IOCGROUP(cmd) == 'r')
			error = rtioctl(so, cmd, data);
		else
			error = ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
		    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));
		break;
	}
	DOMAIN_UNFUNNEL(f);

	if (error == 0 && (cmd & IOC_OUT) && size)
		error = copyout(data, udatap, size);

	return (error);
}

soo_select(fp, corl, reqevents, rtneventsp, notify)
	struct file *fp;
	int corl;
	ushort reqevents;
	ushort *rtneventsp;
	void (*notify)();
{
	register struct socket *so = (struct socket *)fp->f_data;
        register struct sockbuf *sb;
	int rc=0;
	int return_events = 0;
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);

       	if ( (reqevents & POLLOUT) && (sowriteable(so)) )
		 return_events |= POLLOUT;
       	if ( (reqevents & POLLIN) && (soreadable(so)) )
		 return_events |= POLLIN;
	if ( (reqevents & POLLPRI) &&
	     (so->so_oobmark || (so->so_state & SS_RCVATMARK)) )
		 return_events |= POLLPRI;

	if ( !return_events && !(reqevents & POLLSYNC)) {
                if (so->so_special & SP_DISABLE)
                        unlock_enable(so->so_lock->spl,&so->so_lock->sock_lock);
                else
                        simple_unlock(&so->so_lock->sock_lock);
		DOMAIN_UNFUNNEL(f);
		rc = selreg(corl, POLL_SOCKET, so, reqevents, notify);
		if (rc != 0)
			goto out;
		DOMAIN_FUNNEL(sodomain(so), f);
		SOCKET_LOCK(so);
		if (reqevents & POLLOUT) {
			if (sowriteable(so)) {
				return_events |= POLLOUT;
			} else {
				sb = &(so->so_snd);
				sb->sb_reqevents |= POLLOUT;
				sb->sb_flags |= SB_SEL;
			}
		}
		if (reqevents & POLLIN) {
			if (soreadable(so)) {
				return_events |= POLLIN;
			} else {
				sb = &(so->so_rcv);
				sb->sb_reqevents |= POLLIN;
				sb->sb_flags |= SB_SEL;
			}
		}
		if (reqevents & POLLPRI) {
			if (so->so_oobmark || (so->so_state & SS_RCVATMARK)) {
				return_events |= POLLPRI;
			} else {
				sb = &(so->so_rcv);
				sb->sb_reqevents |= POLLPRI;
				sb->sb_flags |= SB_SEL;
			}
		}
	}
	*rtneventsp |= return_events;
	if (so->so_special & SP_DISABLE)
		unlock_enable(so->so_lock->spl,&so->so_lock->sock_lock);
	else
		simple_unlock(&so->so_lock->sock_lock);
	DOMAIN_UNFUNNEL(f);
out:
        return (rc);
}

#ifdef	_AIX_FULLOSF
soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{
#else	/* _AIX_FULLOSF */
soo_stat(fp, ub)
	struct file *fp;
	register struct stat *ub;
{
	register struct socket *so = (struct socket *) fp->f_data;
#endif	/* _AIX_FULLOSF */
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

	bzero((caddr_t)ub, sizeof (*ub));
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error = ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0));
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

soo_close(fp)
	struct file *fp;
{
	struct socket *so;
	int error = 0;

	/*
	 * Should pass the f_flag to soclose as for per-file nonblock,
	 * but use of NBIO avoids interface change. Socket is discarded
	 * so NBIO goes with it.
	 */
	FP_LOCK(fp);
	if (so = (struct socket *)fp->f_data) {
		if (fp->f_flag & (FNDELAY|FNONBLOCK))
			so->so_state |= SS_NBIO;
		FP_UNLOCK(fp);
		error = soclose(so);
		FP_LOCK(fp);
		fp->f_data = 0;
	}
	FP_UNLOCK(fp);
	return (error);
}

#ifdef	_AIX
struct netopt *
find_netopt(id)
	char *id;
{
	struct netopt *np = netopts;

	while (np && strcmp(id, np->name) != 0)
		np = np->next;
	return(np);
}
#endif	/* _AIX */
