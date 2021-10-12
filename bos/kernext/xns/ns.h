/* @(#)93	1.2  src/bos/kernext/xns/ns.h, sysxxns, bos411, 9428A410j 9/15/93 15:04:22 */
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: ns_hosteq
 *		ns_hosteqnh
 *		ns_neteq
 *		ns_neteqnn
 *		ns_netof
 *		ns_nullhost
 *		satons_addr
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *      Base:   ns.h    7.7 (Berkeley) 6/28/90
 */

/*
 * Constants and Structures defined by the Xerox Network Software
 * per "Internet Transport Protocols", XSIS 028112, December 1981
 */

/*
 * Protocols
 */
#define NSPROTO_RI	1		/* Routing Information */
#define NSPROTO_ECHO	2		/* Echo Protocol */
#define NSPROTO_ERROR	3		/* Error Protocol */
#define NSPROTO_PE	4		/* Packet Exchange */
#define NSPROTO_SPP	5		/* Sequenced Packet */
#define NSPROTO_RAW	255		/* Placemarker*/
#define NSPROTO_MAX	256		/* Placemarker*/


/*
 * Port/Socket numbers: network standard functions
 */

#define NSPORT_RI	1		/* Routing Information */
#define NSPORT_ECHO	2		/* Echo */
#define NSPORT_RE	3		/* Router Error */

/*
 * Ports < NSPORT_RESERVED are reserved for priveleged
 * processes (e.g. root).
 */
#define NSPORT_RESERVED		3000

/* flags passed to ns_output as last parameter */

#define	NS_FORWARDING		0x1	/* most of idp header exists */
#define	NS_ROUTETOIF		0x10	/* same as SO_DONTROUTE */
#define	NS_ALLOWBROADCAST	SO_BROADCAST	/* can send broadcast packets */

#define NS_MAXHOPS		15

/* flags passed to get/set socket option */
#define	SO_HEADERS_ON_INPUT	1
#define	SO_HEADERS_ON_OUTPUT	2
#define	SO_DEFAULT_HEADERS	3
#define	SO_LAST_HEADER		4
#define	SO_NSIP_ROUTE		5
#define SO_SEQNO		6
#define	SO_ALL_PACKETS		7
#define SO_MTU			8


/*
 * NS addressing
 */
union ns_host {
	u_char	c_host[6];
	u_short	s_host[3];
};

union ns_net {
	u_char	c_net[4];
	u_short	s_net[2];
};

union ns_net_u {
	union ns_net	net_e;
	u_long		long_e;
};

struct ns_addr {
	union ns_net	x_net;
	union ns_host	x_host;
	u_short	x_port;
};

#ifdef _AIX
#define _SOCKADDR_LEN
#endif

/*
 * Socket address, Xerox style
 */
struct sockaddr_ns {
#if     defined(_SOCKADDR_LEN) || defined(_KERNEL)
        u_char          sns_len;
        u_char          sns_family;
#else
        u_short         sns_family;
#endif
	struct ns_addr	sns_addr;
	char		sns_zero[2];
};
#define sns_port sns_addr.x_port

#ifdef vax
#define ns_netof(a) (*(long *) & ((a).x_net)) /* XXX - not needed */
#endif
#define ns_neteqnn(a,b) (((a).s_net[0]==(b).s_net[0]) && \
					((a).s_net[1]==(b).s_net[1]))
#define ns_neteq(a,b) ns_neteqnn((a).x_net, (b).x_net)
#define satons_addr(sa)	(((struct sockaddr_ns *)&(sa))->sns_addr)
#define ns_hosteqnh(s,t) ((s).s_host[0] == (t).s_host[0] && \
	(s).s_host[1] == (t).s_host[1] && (s).s_host[2] == (t).s_host[2])
#define ns_hosteq(s,t) (ns_hosteqnh((s).x_host,(t).x_host))
#define ns_nullhost(x) (((x).x_host.s_host[0]==0) && \
	((x).x_host.s_host[1]==0) && ((x).x_host.s_host[2]==0))

#ifdef _KERNEL
#include "netns/proto_ns.h"     /* Function prototypes */
#if     MACH
#include <ns.h>                 /* Dynamic config requires dependency */
#endif
#ifndef INET                    /* Attach to INET is crude... */
#include <inet.h>
#endif
#if     (INET > 0) && (INET_DYNAMIC == 0)
#define NSIP    1               /* for now */
#else
#undef  NSIP
#endif

extern struct domain nsdomain;
extern union ns_host ns_thishost;
extern union ns_host ns_zerohost;
extern union ns_host ns_broadhost;
extern union ns_net ns_zeronet;
extern union ns_net ns_broadnet;

#else /* _KERNEL */

#ifdef __STDC__
extern struct ns_addr ns_addr(const char *);
extern char *ns_ntoa(struct ns_addr);

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int ns_ntoa_r(struct ns_addr, char *, int);
#endif  /* _REENTRANT || _THREAD_SAFE */

#else /* __STDC__ */

extern struct ns_addr ns_addr();
extern char *ns_ntoa();

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int ns_ntoa_r();
#endif  /* _REENTRANT || _THREAD_SAFE */

#endif /* __STDC__ */

#endif /* _KERNEL */
