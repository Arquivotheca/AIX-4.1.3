/* @(#)54	1.8.1.3  src/bos/kernext/inet/tcp.h, sockinc, bos411, 9428A410j 12/20/93 11:55:55 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 26,27,85,90
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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
 *	Base:	tcp.h	7.5 (Berkeley) 6/29/88
 *	Merged:	tcp.h	7.7 (Berkeley) 6/28/90
 */

#ifndef	_NETINET_TCP_H
#define	_NETINET_TCP_H

#include <netinet/ip.h>

typedef	u_long	tcp_seq;

/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	u_short	th_sport;		/* source port */
	u_short	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
	struct	ip_firstfour ip_ff;	/* see <netinet/ip.h> */
#define	th_off		ip_v		/* offset */
#define	th_x2		ip_hl		/* unused */
#define	th_xoff		ip_vhl		/* offset+unused */
#define	th_flags	ip_tos		/* flags */
#define	th_win		ip_ff.ip_fwin	/* window (unsigned) */
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
	u_short	th_sum;			/* checksum */
	u_short	th_urp;			/* urgent pointer */
};

#define	TCPOPT_EOL		0
#define	TCPOPT_NOP		1
#define	TCPOPT_MAXSEG		2
#define TCPOPT_WINDOWSCALE	3	/* RFC 1323 window scale option */
#define TCPOPT_TIMESTAMP	8	/* RFC 1323 timestamp option */

#define TCP_MAXWINDOWSCALE	14	/* RFC 1323 max shift factor */

/*
 * RFC 1323 - this define is used for fast timstamp option parsing...
 */
#define TCP_FASTNAME			0x0101080A
#define TCP_TIMESTAMP_OPTLEN		12

/* 
 * RFC 1323 - Used by PAWS algorithm.
 */
#define TCP_24DAYS_WORTH_OF_SLOWTICKS	(24 * 24 * 60 * 60 * PR_SLOWHZ)


/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 is probably more convenient.
 * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).
 */
#define	TCP_MSS	512

#define	TCP_MAXWIN	65535		/* largest value for window */

/*
 * User-settable options (used with setsockopt).
 */
#define	TCP_NODELAY	0x01	/* don't delay send to coalesce packets */
#define	TCP_MAXSEG	0x02	/* set maximum segment size */
#define TCP_RFC1323	0x04	/* Use RFC 1323 algorithms/options */
#endif
