/* @(#)46	1.15.1.7  src/bos/kernext/inet/if_802_5.h, sockinc, bos411, 9428A410j 4/8/94 11:27:59 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: IEEE802_5_MAP_IP_MULTICAST
 *		has_route
 *		is_broadcast
 *		largest_frame
 *		mac_size
 *		rcv_mac_to_llc
 *		route_bytes
 *		snd_mac_to_llc
 *		
 *
 *   ORIGINS: 26,27,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/* * if_token.h -	IBM Token Ring definitions */

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

#define	ACF_PRIORITY3		0x00	/* priority level 3		*/
#define	ACF_TOKEN		0x10	/* token			*/
#define	FCF_LLC_FRAME		0x40	/* for frame control field	*/
#define	RCF_ALL_BROADCAST	0x8000	/* all routes broadcast		*/
#define	RCF_LOCAL_BROADCAST	0x4000	/* single route broadcast	*/
#define	RCF_DIRECTION		0x0080	/* direction			*/
#define	RCF_FRAME0		0x0000	/*  516 max LLC pkt		*/
#define	RCF_FRAME0_MAX		516
#define	RCF_FRAME1		0x0010	/* 1500 max LLC pkt		*/
#define	RCF_FRAME1_MAX		1500
#define	RCF_FRAME2		0x0020	/* 2052 max LLC pkt		*/
#define	RCF_FRAME2_MAX		2052
#define	RCF_FRAME3		0x0030	/* 4472 max LLC pkt		*/
#define	RCF_FRAME3_MAX		4472
#define	RCF_FRAME4		0x0040	/* 8144 max LLC pkt		*/
#define	RCF_FRAME4_MAX		8144
#define	RCF_FRAME5		0x0050	/* 11407 max LLC pkt		*/
#define	RCF_FRAME5_MAX		11407
#define	RCF_FRAME6		0x0060	/* 17800 max LLC pkt		*/
#define	RCF_FRAME6_MAX		17800
#define	RCF_FRAME_MASK		0x0070	/* mask for frame bits		*/
#define	RI_PRESENT		0x80	/* turn on bit 0 of byte 0 src addr */

/* PS2 token support */
#define 	MAX_BRIDGE	8	/* maximum hop count */
#define 	RCF_LEN_MASK	0x1f00	/* length field in routing control */
#define 	L_ADDR		6
#define 	MIN_MAC_HDR_SIZE	14	/* mac header size if no routing info */

#define	route_bytes(mac)	(((mac)->mac_rcf >> 8) & 0x1f)
#define	has_route(mac)		((mac)->mac_src_802_5[0] & RI_PRESENT)
#define	largest_frame(mac)	((mac)->mac_rcf & RCF_FRAME_MASK)
#define	is_broadcast(mac)	(has_route(mac)		\
				 && ((mac)->mac_rcf & (RCF_LOCAL_BROADCAST \
							| RCF_ALL_BROADCAST)))

/*
 * Token ring MAC header
 *
 * Strangely enuf, input frames are fixed format, while non-broadcast
 * output frames are variable format.
 */
#define IE8025_ADDRLEN 6        /* PTM P5654 */
#define IE8025_FRAMECHARS 7     /* PTM P5654 - starting delim(1) +
                                   FCS(4) + ED(1) + Frame status(1) */

struct ie5_mac_hdr {
	struct {
		u_char	Mac_acf;	/* access control field		*/
		u_char	Mac_fcf;	/* frame control field		*/
		u_char	Mac_dst[6];	/* destination address		*/
		u_char	Mac_src[6];	/* source address		*/
	} _First;
	struct {
		u_short	Mac_rcf;	/* routing control field	*/
		u_short	Mac_seg[8];	/* up to 8 segment numbers	*/
	} _Variable;
};

struct ie5_hdr {
	struct ie5_mac_hdr 	ie5_mac;
	struct ie2_llc_snaphdr	ie5_llc;
};

#define	mac_acf	_First.Mac_acf
#define	mac_fcf	_First.Mac_fcf
#define	mac_dst_802_5	_First.Mac_dst
#define	mac_src_802_5	_First.Mac_src
#define mac_rcf	_Variable.Mac_rcf
#define mac_seg	_Variable.Mac_seg

/*
 * mac_to_llc	-	given the ^ to the MAC, get to the LLC
 */
#define snd_mac_to_llc(mac)	\
	(struct ie2_llc_snaphdr *) (((char *) (mac)) + mac_size(mac))

#define rcv_mac_to_llc(mac)	(struct ie2_llc_snaphdr *) ((mac) + 1)

#define	mac_size(mac)		\
	(sizeof ((mac)->_First) + (has_route((mac)) ? route_bytes((mac)) : 0))

/*
 * For compatibility...
 */
#define	_802_5_TYPE_AP	SNAP_TYPE_AP		/* APPLETALK protocol */
#define	_802_5_TYPE_IP	SNAP_TYPE_IP		/* IP protocol */
#define _802_5_TYPE_ARP	SNAP_TYPE_ARP		/* Addr. resolution protocol */
#define DSAP_NETWARE_802_5 DSAP_APPLETALK
#define SSAP_NETWARE_802_5 DSAP_APPLETALK

#define	_802_5_MTU      1492	
#define	_802_5_MIN	(60-14)

/*
 * Token Ring Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	ie5_arp {
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

/*
 * token ring RAW socket addresses
 */
struct sockaddr_802_5 {
	u_char			sa_len;    /* total length */
	u_char			sa_family; /* address family: AF_802_2 */
	struct ie5_hdr		sa_data;   /* mac and llc data portion */
};

#ifdef	_KERNEL
struct	arptab *ie5_arptnew();
char *ie5_sprintf();
#ifdef IP_MULTICAST
/*
 * Macro to map an IP multicast address to an tokenring functional address.
 */

#define IEEE802_5_MAP_IP_MULTICAST(ipaddr, enaddr)                      \
        /* struct in_addr *ipaddr; */                                   \
        /* u_char enaddr[6];       */                                   \
{                                                                       \
        (enaddr)[0] = 0xc0;                                             \
        (enaddr)[1] = 0x00;                                             \
        (enaddr)[2] = 0x00;                                             \
        (enaddr)[3] = 0x04;                                             \
        (enaddr)[4] = 0x00;                                             \
        (enaddr)[5] = 0x00;                                             \
}
#endif
#endif

#ifndef SIOCSARP_802_5
#define	SIOCSARP_802_5	_IOW(i, 93, struct arpreq)	/* set arp entry */
#define	SIOCGARP_802_5	_IOWR(i,94, struct arpreq)	/* get arp entry */
#define	SIOCDARP_802_5	_IOW(i, 95, struct arpreq)	/* delete arp entry */

#define IE5_FILLIN_HDR(macp, srcp, destp, Rcf, segp) { \
	*(struct char6 {char x[6];}*)(macp)->mac_dst_802_5 =	\
		*(struct char6 *)(destp); /* PERF */	\
	*(struct char6 *)(macp)->mac_src_802_5 =	\
		*(struct char6 *)(srcp); /* PERF */	\
	(macp)->mac_acf     = ACF_PRIORITY3;	\
	(macp)->mac_fcf     = FCF_LLC_FRAME;	\
	if ((Rcf)) {	\
		(macp)->mac_src_802_5[0] |= RI_PRESENT;	\
		(macp)->mac_rcf     = (Rcf);	\
		bcopy((segp), (macp)->mac_seg, mac_size((macp)));	\
	} else	\
		(macp)->mac_src_802_5[0] &= ~RI_PRESENT;	\
	ie2_llc((caddr_t)((macp))+mac_size((macp)), _802_5_TYPE_IP);	\
}
#endif
