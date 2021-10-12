/* @(#)85	1.1  src/bos/kernext/inet/inet_config.h, sysxinet, bos411, 9428A410j 7/24/93 13:51:57 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS:
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
 * OSF/1 1.2
 */

#define INET_CONFIG_VERSION_1	0x01091590

#define IN_DEFAULT_VALUE	-12345
#define IN_USEVALUE		0x01
#define IN_USEDEFAULTS		0x02

typedef struct inet_config {

	int	version;
	int	errcode;
	int	flags;

	int	inetprintfs;		/* If configured, enable printfs (0) */
	int	useloopback;		/* Use loopback for own packets (1) */

	int	ipgateway;		/* Configure as gateway (0) */
	int	ipforwarding;		/* Act as gateway (0) */
	int	ipsendredirects;	/* Send ICMP redirects (1) */
	int	ipdirected_broadcast;	/* Broadcasts accepted uniquely (0) */
	int	ipsrcroute;		/* Enable host source routing (1) */
	int	subnetsarelocal;	/* Subnets appear as connected (1) */
	int	ipqmaxlen;		/* Length of IP input queue (50) */

	/* All times in SECONDS */

	int	tcpttl;			/* Default time to live (60) */
	int	tcpmssdflt;		/* Default max segsize (512) */
	int	tcprttdflt;		/* Default initial rtt (3) */
	int	tcpkeepidle;		/* Keepalive idle timer (7200) */
	int	tcpkeepintvl;		/* Keepalive interval (75) */
	int	tcpcompat_42;		/* BSD4.2 compat keepalive/urg (1) */
	int	tcprexmtthresh;		/* Retransmit threshold (3) */
	int	tcpconsdebug;		/* If configured, debug printfs (0) */
	u_long	tcp_sendspace;		/* Default send queue (4096) */
	u_long	tcp_recvspace;		/* Default receive queue (4096) */

	int	udpttl;			/* Default time to live (60) */
	int	udpcksum;		/* Enable checksumming (1) */
	u_long	udp_sendspace;		/* Default send queue (4096) */
	u_long	udp_recvspace;		/* Default receive queue (4096) */

	int	arpkillc;		/* Time to remove completed (1200) */
	int	arpkilli;		/* Time to remove incomplete (180) */
	int	arprefresh;		/* Time to refresh entry (120) */
	int	arphold;		/* Time to hold packet (5) */
	int	arplost;		/* Count to broadcast refresh (3) */
	int	arpdead;		/* Count to assume dead (6) */
	int	arpqmaxlen;		/* Length of ARP input queue (50) */
	int	arptabbsiz;		/* Table bucket size (16/9 gw/!gw) */
	int	arptabnb;		/* Number of buckets (37/19 gw/!gw) */
} inet_config_t;
