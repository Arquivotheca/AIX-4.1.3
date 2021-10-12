/* @(#)49	1.8  src/bos/kernext/inet/in_systm.h, sockinc, bos411, 9428A410j 7/24/93 13:46:18 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: 
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
 *	Base:	in_systm.h	7.3 (Berkeley) 6/29/88
 *	Merged:	in_systm.h	7.4 (Berkeley) 6/28/90
 */

#ifndef _NETINET_IN_SYSTM_H_
#define _NETINET_IN_SYSTM_H_

/*
 * Miscellaneous internetwork
 * definitions for kernel.
 */

/*
 * Network types.
 *
 * Internally the system keeps counters in the headers with the bytes
 * in native byte order so that machine instructions will work on them.
 * It reverses the bytes if necessary before transmission at each protocol
 * level.  The n_ types represent the types with the bytes in ``high-ender''
 * (big endian) order.
 */
typedef u_short n_short;		/* short as received from the net */
typedef u_long	n_long;			/* long as received from the net */

typedef	u_long	n_time;			/* ms since 00:00 GMT, net byte order */

#ifdef _KERNEL

#include "netinet/proto_inet.h"	/* Function prototypes */
#if	MACH
#include <inet.h>	/* Dynamic config requires dependency */
#endif

/*
 * Global configuration parameters for Internet. With the exception of
 * INETPRINTFS, these control the default value of kernel variables
 * settable by the system administrator. These variables are int's with
 * the names of the #defines, in lower case.
 */

/* GATEWAY turns on IP forwarding and expands ARP tables. */
#ifdef	GATEWAY
#define IPFORWARDING	0
#define IPGATEWAY	1
#else
#define IPFORWARDING	0
#define IPGATEWAY	0
#endif

/* DIRECTED_BROADCAST makes broadcast addresses match
 * only for their proper interface. */
#ifdef	DIRECTED_BROADCAST
#define IPDIRECTED_BROADCAST	1
#else
#define IPDIRECTED_BROADCAST	0
#endif

/* SUBNETSARELOCAL makes subnets appear as directly connected
 * for the purpose of max segsize, etc. */
#ifndef	SUBNETSARELOCAL
#define SUBNETSARELOCAL	1
#endif

/* TCP_COMPAT_42 makes TCP adjust initial send sequence numbers
 * and keepalives to make 4.2-based systems happy. It also causes
 * urgent data to behave (incorrectly) the way it always did. */
#ifndef	TCP_COMPAT_42
#define TCP_COMPAT_42	1
#endif

/* IPSENDREDIRECTS enables the sending of ICMP redirects. */
#ifndef	IPSENDREDIRECTS
#define IPSENDREDIRECTS	1
#endif

/* UDPCKSUM enables the udp checksum...should be on by default. */
#ifndef UDPCKSUM
#define UDPCKSUM 1
#endif

#endif
#endif
