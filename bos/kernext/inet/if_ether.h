/* @(#)35	1.22  src/bos/kernext/inet/if_ether.h, sockinc, bos411, 9428A410j 7/7/94 16:18:03 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: ETHER_MAP_IP_MULTICAST
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
 *      Base:   if_ether.h      7.4 (Berkeley) 2/17/89
 *      Merged: if_ether.h      7.5 (Berkeley) 6/28/90
 */

/*
 * For compatibility with code that included this header file without
 * the new cdli headers.
 */
#ifndef _SYS_CDLI_H
#include <sys/cdli.h>
#endif

#ifndef _NET_ND_LAN_H
#include <net/nd_lan.h>
#endif

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	ether_header {
	u_char	ether_dhost[6];
	u_char	ether_shost[6];
	u_short	ether_type;
};

struct ie3_mac_hdr {
        u_char  ie3_mac_dst[6]; /* destination address          */
        u_char  ie3_mac_src[6]; /* source address               */
        u_short ie3_mac_len;    /* frame length                 */
};

struct ie3_hdr {
        struct ie3_mac_hdr mac;
        struct ie2_llc_snaphdr llc;
};

struct sockaddr_802_3 {
        u_char                  sa_len;
        u_char                  sa_family;
        struct ie3_mac_hdr      sa_mac;
        struct ie2_llc_snaphdr  sa_llc;
};

#define mac_dst_802_3 ie3_mac_dst
#define mac_src_802_3 ie3_mac_src
#define mac_len_802_3 ie3_mac_len

#define	ETHERTYPE_PUP		0x0200	/* PUP protocol */
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#define ETHERTYPE_ARP		0x0806	/* Addr. resolution protocol */
#define ETHERTYPE_REVARP 	0x8035	/* Reverse ARP */
#define ETHERTYPE_NETWARE 	0x8137	/* Netware protocol */
#define ETHERTYPE_NS		0x0600	/* XNS protocol */

/*
 * For compatibility...
 */
#define _802_3_TYPE_AP  SNAP_TYPE_AP    /* APPLETALK protocol */
#define _802_3_TYPE_IP  SNAP_TYPE_IP    /* IP protocol */
#define _802_3_TYPE_ARP SNAP_TYPE_ARP   /* Addr. resolution protocol */

/*
 * The ETHERTYPE_NTRAILER packet types starting at ETHERTYPE_TRAIL have
 * (type-ETHERTYPE_TRAIL)*512 bytes of data followed
 * by an ETHER type (as given above) and then the (variable-length) header.
 */
#define	ETHERTYPE_TRAIL		0x1000		/* Trailer packet */
#define	ETHERTYPE_NTRAILER	16
#define ETHER_ADDRLEN           6       /* PTM P5654 */
#define ETH_FRAMECHARS          12      /* PTM P5654 Preamble(8) + CRC(4) */

#define	ETHERMTU	1500
#define	ETHERMIN	(60-14)

/*
 * Ethernet Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	ether_arp {
	struct	arphdr ea_hdr;	/* fixed-size header */
	u_char	arp_sha[6];	/* sender hardware address */
	u_char	arp_spa[4];	/* sender protocol address */
	u_char	arp_tha[6];	/* target hardware address */
	u_char	arp_tpa[4];	/* target protocol address */
};
#define	arp_hrd	ea_hdr.ar_hrd
#define	arp_pro	ea_hdr.ar_pro
#define	arp_hln	ea_hdr.ar_hln
#define	arp_pln	ea_hdr.ar_pln
#define	arp_op	ea_hdr.ar_op

#ifdef _SUN
/*
 * ethernet address struct (for the SUN RPC stuff)
 */
struct ether_addr {
	u_char	ether_addr_octet[6];
};
#endif  /* _SUN  */

#ifdef	_KERNEL
#ifdef IP_MULTICAST
/*
 * Macro to map an IP multicast address to an Ethernet multicast address.
 * The high-order 25 bits of the Ethernet address are statically assigned,
 * and the low-order 23 bits are taken from the low end of the IP address.
 */
#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr)				\
	/* struct in_addr *ipaddr; */					\
	/* u_char enaddr[6];       */					\
{									\
	(enaddr)[0] = 0x01;						\
	(enaddr)[1] = 0x00;						\
	(enaddr)[2] = 0x5e;						\
	(enaddr)[3] = ((u_char *)ipaddr)[1] & 0x7f;			\
	(enaddr)[4] = ((u_char *)ipaddr)[2];				\
	(enaddr)[5] = ((u_char *)ipaddr)[3];				\
}
#endif /* IP_MULTICAST */
#endif
