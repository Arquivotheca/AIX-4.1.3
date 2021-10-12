/* @(#)45	1.18  src/bos/kernext/inet/if_802_3.h, sockinc, bos411, 9428A410j 8/7/92 10:50:56 */
/* 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * if_802_3.h -	Address Resolution Protocol for IEEE 802.3: definitions
 */

/*
 * 802.3 MAC header
 */
#define IE8023_ADDRLEN		6       /* PTM P5654 */
#define IE8023_FRAMECHARS	12      /* PTM P5654 - preamble (8) + FCS (4) */

struct ie3_mac_hdr {
	u_char	ie3_mac_dst[6];	/* destination address		*/
	u_char	ie3_mac_src[6];	/* source address		*/
	u_short	ie3_mac_len;	/* frame length			*/
};

#define mac_dst ie3_mac_dst
#define mac_src ie3_mac_src
#define mac_len ie3_mac_len

#ifndef _802_2_LLC
#define	_802_2_LLC
/*
 * 802.3 LLC header
 */
struct ie2_llc_hdr {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
	unsigned char	prot_id[3];	/* protocol id			*/
	unsigned short	type;		/* type field			*/
};
#endif

#define	DSAP_INET	0xaa		/* SNAP SSAP			*/
#define	SSAP_INET	0xaa		/* SNAP DSAP			*/
#define	SSAP_RESP	0x01		/* SSAP response bit		*/
#define	CTRL_UI		0x03		/* unnumbered info		*/
#define CTRL_XID	0xaf		/* eXchange IDentifier		*/
#define	CTRL_TEST	0xe3		/* test frame			*/

#define DSAP_ISO	0xfe		/* ISO DSAP */
#define SSAP_ISO	0xfe		/* ISO SSAP */

#define DSAP_XNS	0xfa		/* XNS DSAP */
#define SSAP_XNS	0xfa		/* XNS SSAP */

#define DSAP_NETWARE 0xff
#define SSAP_NETWARE 0xff

#define DSAP_NETBIOS 0xf0
#define SIZEOF_NETWARE_LLC_802_3 0

#define	_802_3_TYPE_AP	0x809b		/* APPLETALK protocol */
#define	_802_3_TYPE_IP	0x0800		/* IP protocol */
#define _802_3_TYPE_ARP	0x0806		/* Addr. resolution protocol */

/*
 * The _802_3_TYPE_NTRAILER packet types starting at _802_3_TYPE_TRAIL have
 * (type-_802_3_TYPE_TRAIL)*512 bytes of data followed
 * by an _802_3_ type (as given above) and then the (variable-length) header.
 */
#define	_802_3_TYPE_TRAIL		0x1000		/* Trailer packet */
#define	_802_3_TYPE_NTRAILER	16

#define	_802_3_MTU	1492
#define	_802_3_MIN	(60-22)

/*
 * Token Ring Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	ie3_arp {
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

struct sockaddr_802_3 {
	ushort			sa_family;	/* address family: AF_802_3 */
	struct ie3_mac_hdr	sa_mac;		/* MAC portion		    */
	struct ie2_llc_hdr	sa_llc;		/* LLC portion		    */
};

#ifdef	_KERNEL
extern u_char ie3_broadcastaddr[6];
struct	arptab *ie3_arptnew();
char *ie3_sprintf();
#endif

/*
 * missing from ioctl.h
 */
#ifndef	SIOCSARP_802_3
#define	SIOCSARP_802_3	_IOW(i, 96, struct arpreq)	/* set arp entry */
#define	SIOCGARP_802_3	_IOWR(i,97, struct arpreq)	/* get arp entry */
#define	SIOCDARP_802_3	_IOW(i, 98, struct arpreq)	/* delete arp entry */
#endif
