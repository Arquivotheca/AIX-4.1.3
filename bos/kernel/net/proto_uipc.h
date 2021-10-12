/* @(#)19	1.5  src/bos/kernel/net/proto_uipc.h, sysnet, bos41J, 9511A_all 2/27/95 14:13:17 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: P
 *		
 *
 *   ORIGINS: 27,85
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

# define	P(s) s
/* Avoid scoping problems */
struct file; struct uio; struct ucred; struct stat; struct proc;
struct socket; struct sockbuf; struct mbuf;
struct msghdr; struct unpcb;
struct domain;
/* typedef mblk_t */

/* sys_socket.c */
int	soo_read P((struct file *, struct uio *, struct ucred *));
int	soo_write P((struct file *, struct uio *, struct ucred *));
int	soo_ioctl P((struct file *, int, caddr_t));
#if	MACH	/* poll interface */
int	soo_select P((struct file *, short *, short *, int));
#else		/* select interface */
#ifdef	_AIX_FULLOSF
int	soo_select P((struct file *, int));
#endif	/* _AIX_FULLOSF */
#endif
#ifdef	_AIX_FULLOSF
int	soo_stat P((struct socket *, struct stat *));
#else	/* _AIX_FULLOSF */
int	soo_stat P((struct file *, struct stat *));
#endif	/* _AIX_FULLOSF */
int	soo_close P((struct file *));

/* uipc_domain.c */
void	domaininit P((void));
int	domain_add P((struct domain *));
int	domain_del P((struct domain *));
struct	protosw *pffindtype P((int, int));
struct	protosw *pffindproto P((int, int, int));
void	pfctlinput P((int, struct sockaddr *));
void	pfreclaim P((void));
void	pfslowtimo P((void));
void	pffasttimo P((void));

/* uipc_mbuf.c */
void	mbinit P((void));
caddr_t	m_clalloc P((struct mbuf *, int, int));
void	mbufintr P((void));
struct	mbuf *m_retry P((int, int));
struct	mbuf *m_retryhdr P((int, int));
struct	mbuf *m_get P((int, int));
struct	mbuf *m_gethdr P((int, int));
struct	mbuf *m_getclr P((int, int));
struct	mbuf *m_free P((struct mbuf *));
#ifdef	_AIX
struct	mbuf *m_getclustm P((int, int, int));
#endif	/* _AIX */
void	m_freem P((struct mbuf *));
int	m_leadingspace P((struct mbuf *));
int	m_trailingspace P((struct mbuf *));
struct	mbuf *m_prepend P((struct mbuf *, int, int));
struct	mbuf *m_copym P((struct mbuf *, int, int, int));
void	m_copydata P((struct mbuf *, int, int, caddr_t));
void	m_cat P((struct mbuf *, struct mbuf *));
void	m_adj P((struct mbuf *, int));
struct	mbuf *m_pullup P((struct mbuf *, int));
/*
mblk_t	*mbuf_to_mblk P((struct mbuf *, int));
struct	mbuf *mblk_to_mbuf P((mblk_t *, int));
*/

/* uipc_proto.c */
int	uipc_configure P((void));

/* uipc_socket.c */
int	socreate P((int, struct socket **, int, int));
int	sobind P((struct socket *, struct mbuf *));
int	solisten P((struct socket *, int));
void	sofree P((struct socket *));
int	solockpair P((struct socket *, struct socket *));
int	sounlock P((struct socket *));
int	soclose P((struct socket *));
int	soabort P((struct socket *));
int	soaccept P((struct socket *, struct mbuf *));
int	soconnect P((struct socket *, struct mbuf *));
int	soconnect2 P((struct socket *, struct socket *));
int	sodisconnect P((struct socket *));
int	sosend P((struct socket *, struct mbuf *, struct uio *,
				struct mbuf *, struct mbuf *, int));
int	soreceive P((struct socket *, struct mbuf **, struct uio *,
				struct mbuf **, struct mbuf **, int *));
int	soshutdown P((struct socket *, int));
void	sorflush P((struct socket *));
void	sopriv P((struct socket *));
int	sosetopt P((struct socket *, int, int, struct mbuf *));
int	sogetopt P((struct socket *, int, int, struct mbuf **));
void	sohasoutofband P((struct socket *));
int	sodequeue P((struct socket *, struct socket **, struct mbuf **, int));
int	sogetaddr P((struct socket *, struct mbuf **, int, int));

/* uipc_socket2.c */
void	soisconnecting P((struct socket *));
void	soisconnected P((struct socket *));
void	soisdisconnecting P((struct socket *));
void	soisdisconnected P((struct socket *));
struct	socket *sonewsock P((struct socket *, int));
void	soqinsque P((struct socket *, struct socket *, int));
int	soqremque P((struct socket *, int));
void	socantsendmore P((struct socket *));
void	socantrcvmore P((struct socket *));
void	sbselqueue P((struct sockbuf *));
void	sbseldequeue P((struct sockbuf *));
int	sosblock P((struct sockbuf *, struct socket *));
void	sbunlock P((struct sockbuf *));
int	sosbwait P((struct sockbuf *, struct socket *));
int	sosleep P((struct socket *, caddr_t, int, int));
void	sowakeup P((struct socket *, struct sockbuf *));
int	sbwakeup P((struct socket *, struct sockbuf *, int));
#ifdef _AIX_FULLOSF
int	sbpoll P((struct socket *, struct sockbuf *));
#endif
int	soreserve P((struct socket *, u_long, u_long));
int	sbreserve P((struct sockbuf *, u_long));
void	sbrelease P((struct sockbuf *));
void	sbappend P((struct sockbuf *, struct mbuf *));
void	sbappendrecord P((struct sockbuf *, struct mbuf *));
void	sbinsertoob P((struct sockbuf *, struct mbuf *));
int	sbappendaddr P((struct sockbuf *, struct sockaddr *,
				struct mbuf *, struct mbuf *));
int	sbappendcontrol P((struct sockbuf *, struct mbuf *, struct mbuf *));
void	sbcompress P((struct sockbuf *, struct mbuf *, struct mbuf *));
void	sbflush P((struct sockbuf *));
void	sbdrop P((struct sockbuf *, int));
void	sbdroprecord P((struct sockbuf *));
int	lock_socheck P((struct socket *));
int	unlock_socheck P((struct socket *));
int	lock_sbcheck P((struct sockbuf *));
int	unlock_sbcheck P((struct sockbuf *));

/* uipc_syscalls.c */
#ifndef	_AIX
int	socket P((struct proc *, void *, int *));
int	bind P((struct proc *, void *, int *));
int	listen P((struct proc *, void *, int *));
int	accept P((struct proc *, void *, int *));
int	oaccept P((struct proc *, void *, int *));
int	connect P((struct proc *, void *, int *));
int	socketpair P((struct proc *, void *, int *));
int	sendto P((struct proc *, void *, int *));
int	osend P((struct proc *, void *, int *));
int	osendmsg P((struct proc *, void *, int *));
int	sendmsg P((struct proc *, void *, int *));
int	recvfrom P((struct proc *, void *, int *));
int	orecvfrom P((struct proc *, void *, int *));
int	orecv P((struct proc *, void *, int *));
int	orecvmsg P((struct proc *, void *, int *));
int	recvmsg P((struct proc *, void *, int *));
int	shutdown P((struct proc *, void *, int *));
int	setsockopt P((struct proc *, void *, int *));
int	getsockopt P((struct proc *, void *, int *));
int	pipe P((struct proc *, void *, int *));
int	getsockname P((struct proc *, void *, int *));
int	ogetsockname P((struct proc *, void *, int *));
int	getpeername P((struct proc *, void *, int *));
int	ogetpeername P((struct proc *, void *, int *));
int	sendit P((int, struct msghdr *, int, int *));
int	recvit P((int, struct msghdr *, caddr_t, int *));
#ifdef	COMPAT_43
int	accept1 P((struct proc *, void *, int *, int));
int	recvfrom1 P((int));
int	getsockname1 P((struct proc *, void *, int *, int));
int	getpeername1 P((struct proc *, void *, int *, int));
#endif
#endif	/* _AIX */
void	sockaddr_new P((struct mbuf *));
void	sockaddr_old P((struct mbuf *));
int	sockargs P((struct mbuf **, caddr_t, int, int));
struct	file *getsock P((int, int *));
int	socksetup P((int *, struct file **));

/* uipc_usrreq.c */
void	uipc_init P((void));
int	uipc_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	unp_attach P((struct socket *));
void	unp_detach P((struct unpcb *));
int	unp_bind P((struct unpcb *, struct mbuf *));
int	unp_connect P((struct socket *, struct mbuf *));
int	unp_connect2 P((struct socket *, struct socket *));
void	unp_disconnect P((struct unpcb *));
void	unp_abort P((struct unpcb *));
void	unp_shutdown P((struct unpcb *));
void	unp_drain P((void));
int	unp_externalize P((struct mbuf *));
int	unp_internalize P((struct mbuf *));
void	unp_drop P((struct unpcb *, int));
void	unp_gc P((void));
void	unp_dispose P((struct mbuf *));
void	unp_scan P((struct mbuf *, void (*)(struct file *)));
void	unp_mark P((struct file *));
void	unp_discard P((struct file *));
void	uipc_slowtimo P((void));

#undef P
