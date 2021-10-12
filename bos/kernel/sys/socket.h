/* @(#)63	1.10.2.6  src/bos/kernel/sys/socket.h, sockinc, bos411, 9433A411a 8/12/94 10:09:09 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: CMSG_DATA
 *		CMSG_FIRSTHDR
 *		CMSG_NXTHDR
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
/* @(#)socket.h	2.1 16:08:05 4/20/90 SecureWare */
/*
 * Copyright (c) 1982,1985,1986,1988 Regents of the University of California.
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
 *	Base:	socket.h	7.6 (Berkeley) 9/4/89
 *	Merged: socket.h	7.10 (Berkeley) 6/28/90
 */

#ifndef	_SYS_SOCKET_H_
#define	_SYS_SOCKET_H_

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */
#define	SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define	SO_CKSUMRECV	0x0800		/* defer checksum until receive */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define	AF_UNSPEC	0		/* unspecified */
#define	AF_UNIX		1		/* local to host (pipes, portals) */
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK	3		/* arpanet imp addresses */
#define	AF_PUP		4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS	5		/* mit CHAOS protocols */
#define	AF_NS		6		/* XEROX NS protocols */
#define	AF_ISO		7		/* ISO protocols */
#define	AF_OSI		AF_ISO
#define	AF_ECMA		8		/* european computer manufacturers */
#define	AF_DATAKIT	9		/* datakit protocols */
#define	AF_CCITT	10		/* CCITT protocols, X.25 etc */
#define	AF_SNA		11		/* IBM SNA */
#define AF_DECnet	12		/* DECnet */
#define AF_DLI		13		/* DEC Direct data link interface */
#define AF_LAT		14		/* LAT */
#define	AF_HYLINK	15		/* NSC Hyperchannel */
#define	AF_APPLETALK	16		/* Apple Talk */
#define	AF_ROUTE	17		/* Internal Routing Protocol */
#define	AF_LINK		18		/* Link layer interface */
#define	pseudo_AF_XTP	19		/* eXpress Transfer Protocol (no AF) */
#ifdef	_AIX
#define AF_INTF		20		/* Debugging use only */
#define AF_RIF		21		/* raw interface */
#define	AF_NETWARE	22
#define	AF_NDD		23
#define	AF_MAX		30
#else
#define	AF_MAX		20
#endif

#if	!defined(COMPAT_43) || defined(_KERNEL)
/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	u_char	sa_len;			/* total length */
	u_char	sa_family;		/* address family */
	char	sa_data[14];		/* actually longer; address value */
};
/*
 * 4.3 compat sockaddr, move to compat file later
 */
struct osockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

#ifndef	_KERNEL
#define	recvfrom	nrecvfrom
#define	accept		naccept
#define	getpeername	ngetpeername
#define	getsockname	ngetsockname
#endif

#else	/* BSD4.3 */

struct sockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

#endif	/* BSD4.3 */

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	u_short	sp_family;		/* address family */
	u_short	sp_protocol;		/* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_UNIX		AF_UNIX
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_ISO		AF_ISO
#define	PF_OSI		AF_ISO
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define PF_DECnet	AF_DECnet
#define PF_DLI		AF_DLI
#define PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define	PF_APPLETALK	AF_APPLETALK
#define	PF_ROUTE	AF_ROUTE
#define	PF_LINK		AF_LINK
#define	PF_XTP		pseudo_AF_XTP	/* really just proto family, no AF */
#ifdef	_AIX
#define PF_INTF		AF_INTF
#define PF_RIF		AF_RIF
#define PF_INTF         AF_INTF		/* Used by sysx/intf */
#define PF_NDD		AF_NDD
#endif

#define	PF_MAX		AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#ifdef	_AIX
#define	SOMAXCONN	10
#else
#define	SOMAXCONN	5
#endif


#if	!defined(COMPAT_43) || defined(_KERNEL)
/*
 * Message header for recvmsg and sendmsg calls.
 * Used value-result for recvmsg, value only for sendmsg.
 */
#ifdef _XOPEN_EXTENDED_SOURCE
struct msghdr {
	void 	*msg_name;		/* optional address */
	size_t	msg_namelen;		/* size of address */
	struct	iovec	*msg_iov;	/* scatter/gather array */
	unsigned int	msg_iovlen;	/* # elements in msg_iov */
	struct	cmghdr 	*msg_control;	/* ancillary data, see below */
	size_t	msg_controllen;		/* ancillary data buffer len */
	int	msg_flags;		/* flags on received message */
};
#else
struct msghdr {
	caddr_t	msg_name;		/* optional address */
	u_int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	u_int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_control;		/* ancillary data, see below */
	u_int	msg_controllen;		/* ancillary data buffer len */
	int	msg_flags;		/* flags on received message */
};
#endif /*_XOPEN_EXTENDED_SOURCE */

/*
 * 4.3-compat message header (move to compat file later).
 */
struct omsghdr {
	caddr_t	msg_name;		/* optional address */
	int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_accrights;		/* access rights sent/received */
	int	msg_accrightslen;
};

#ifndef	_KERNEL
#define	recvmsg		nrecvmsg
#define	sendmsg		nsendmsg
#endif

#else	/* BSD4.3 */

struct msghdr {
	caddr_t	msg_name;		/* optional address */
	int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_accrights;		/* access rights sent/received */
	int	msg_accrightslen;
};

#endif	/* BSD4.3 */

#define  MSG_MAXIOVLEN   16              /* compat only, no longer enforced */

#ifndef	UIO_MAXIOV
#define UIO_MAXIOV      1024            /* max 1K of iov's */
#endif

#ifndef	UIO_SMALLIOV
#define UIO_SMALLIOV    8               /* 8 on stack, else malloc */
#endif

#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */
#define	MSG_EOR		0x8		/* data completes record */
#define	MSG_TRUNC	0x10		/* data discarded before delivery */
#define	MSG_CTRUNC	0x20		/* control data lost before delivery */
#define	MSG_WAITALL	0x40		/* wait for full request or error */
/* Following used within kernel */
#define	MSG_NONBLOCK	0x4000		/* nonblocking request */
#define MSG_COMPAT	0x8000		/* 4.3-format sockaddr */

/*
 * Header for ancillary data objects in msg_control buffer.
 * Used for additional information with/about a datagram
 * not expressible by flags.  The format is a sequence
 * of message elements headed by cmsghdr structures.
 */
struct cmsghdr {
#ifdef _XOPEN_EXTENDED_SOURCE
	size_t	cmsg_len;
#else
	u_int	cmsg_len;		/* data byte count, including hdr */
#endif /* _XOPEN_EXTENDED_SOURCE */
	int	cmsg_level;		/* originating protocol */
	int	cmsg_type;		/* protocol-specific type */
/* followed by	u_char  cmsg_data[]; */
};

/* given pointer to struct adatahdr, return pointer to data */
#define	CMSG_DATA(cmsg)		((u_char *)((cmsg) + 1))

/* given pointer to struct adatahdr, return pointer to next adatahdr */
#define	CMSG_NXTHDR(mhdr, cmsg)	\
	(((caddr_t)(cmsg) + (cmsg)->cmsg_len + sizeof(struct cmsghdr) > \
	    (mhdr)->msg_control + (mhdr)->msg_controllen) ? \
	    (struct cmsghdr *)NULL : \
	    (struct cmsghdr *)((caddr_t)(cmsg) + ALIGN((cmsg)->cmsg_len)))

#define	CMSG_FIRSTHDR(mhdr)	((struct cmsghdr *)(mhdr)->msg_control)

/* "Socket"-level control message types: */
#define	SCM_RIGHTS	0x01		/* access rights (array of int) */

#ifndef	_KERNEL
#ifdef	_NO_PROTO
int	accept();
int	bind();
int	connect();
int	getpeername();
int	getsockname();
int	getsockopt();
int	listen();
#ifdef	_XOPEN_EXTENDED_SOURCE
ssize_t	recv();
ssize_t	recvfrom();
ssize_t recvmsg();
ssize_t send();
ssize_t sendto();
ssize_t sendmsg();
#else
int	recv();
int	recvfrom();
int	recvmsg();
int	send();
int	sendto();
int	sendmsg();
#endif 	/* _XOPEN_EXTENDED_SOURCE */
int	setsockopt();
int	shutdown();
int	socket();
int	socketpair();
#else	/* _NO_PROTO */
int	accept(int, struct sockaddr *, int *);
int	bind(int, const struct sockaddr *, int);
int	connect(int, const struct sockaddr *, int);
int	getpeername(int, struct sockaddr *, int *);
#ifdef	_XOPEN_EXTENDED_SOURCE
int	getsockname(int, struct sockaddr *, int);
#else
int	getsockname(int, struct sockaddr *, int *);
#endif	/* _XOPEN_EXTENDED_SOURCE */
int	getsockopt(int, int, int, void *, int *);
int	listen(int, int);
#ifdef	_XOPEN_EXTENDED_SOURCE
ssize_t	recv(int, void *, int, int);
ssize_t	recvfrom(int, void *, int, int, struct sockaddr *, size_t *);
ssize_t	recvmsg(int, struct msghdr *, int);
ssize_t	send(int, const void *, int, int);
ssize_t	sendto(int, const void *, int, int, const struct sockaddr *, int);
ssize_t	sendmsg(int, const struct msghdr *, int);
int	setsockopt(int, int, int, void *, size_t *);
#else
int	recv(int, void *, int, int);
int	recvfrom(int, void *, int, int, struct sockaddr *, int *);
int	recvmsg(int, struct msghdr *, int);
int	send(int, const void *, int, int);
int	sendto(int, const void *, int, int, const struct sockaddr *, int);
int	sendmsg(int, const struct msghdr *, int);
int	setsockopt(int, int, int, const void *, int);
#endif	/* _XOPEN_EXTENDED_SOURCE */
int	shutdown(int, int);
int	socket(int, int, int);
int	socketpair(int, int, int, int *);
#endif
#endif
#endif
