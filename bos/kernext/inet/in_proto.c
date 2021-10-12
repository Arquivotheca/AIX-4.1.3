static char sccsid[] = "@(#)70	1.7.1.10  src/bos/kernext/inet/in_proto.c, sysxinet, bos412, 9446B 11/16/94 12:30:56";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: inet_configure
 *		inet_funnel
 *		inet_sanity
 *		inet_unfunnel
 *		
 *
 *   ORIGINS: 26,27,85,89
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *      Base:   in_proto.c      7.4 (Berkeley) 4/22/89
 *      Merged: in_proto.c      7.5 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/inet_config.h"

#include "sys/sysconfig.h"

/* NSIP encapsulation option check */
#ifndef	NS
#include <netns/ns.h>
#endif
#if	(NS > 0) && (NS_DYNAMIC == 0)
#include <netns/proto_ns.h>
#endif

/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */

extern	struct domain inetdomain;

#if	NETSYNC_SPL
static void
inet_sanity()
{
	panic("inet unfunnel");
}

static void
inet_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = inet_sanity;
	NETSPLX(dfp->object.spl);
	unix_release();
}

static void
inet_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("inet funnel");
	dfp->unfunnel = inet_unfunnel;
	unix_master();
	NETSPL(dfp->object.spl,net);
}

/* No "force unfunnel" op required with unix_master/spl */

#else
#define inet_funnel	0
#endif

CONST struct protosw inetsw[] = {
{ 0,		&inetdomain,	0,		PR_INTRLEVEL,
  rip_input,	ip_output,	0,		0,
  0, 		0,		0,
  ip_init,	0,		ip_slowtimo,	ip_drain
},
{ SOCK_DGRAM,	&inetdomain,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  udp_input,	0,		udp_ctlinput,	ip_ctloutput,
  udp_usrreq, 	udp_send,	udp_receive,
  udp_init,	0,		0,		0,
},
{ SOCK_STREAM,	&inetdomain,	IPPROTO_TCP,	PR_CONNREQUIRED|PR_WANTRCVD|PR_NOEOR|PR_INTRLEVEL,
  tcp_input,	0,		tcp_ctlinput,	tcp_ctloutput,
  tcp_usrreq, 	0,		0,
  0,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  rip_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq, 	0,		0,
  0,		0,		0,		0
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_ICMP,	PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  icmp_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,	0,		0,
  0,		0,		0,		0
},
#ifdef IP_MULTICAST
{ SOCK_RAW,	&inetdomain,	IPPROTO_IGMP,	PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  igmp_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,	0,		0,
  igmp_init,	igmp_fasttimo,	0,		0,
},
#endif IP_MULTICAST
	/* raw wildcard */
{ SOCK_RAW,	&inetdomain,	0,		PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  rip_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,	0,		0,
  0,		0,		ip_slowtimo,		0
}
};

struct domain inetdomain =
    { AF_INET, "internet", 0, 0, 0, 
      inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])],
      0, 0, inet_funnel, 0 };
