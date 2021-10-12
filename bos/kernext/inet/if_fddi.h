/* @(#)45	1.2.1.9  src/bos/kernext/inet/if_fddi.h, sockinc, bos411, 9428A410j 4/8/94 11:27:53 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: FDDI_MAP_IP_MULTICAST
 *		fdadcpy
 *		has_route_f
 *		is_broadcast_f
 *		mac_size_f
 *		mac_to_llc_f
 *		route_bytes_f
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

/* if_fddi.h -	FDDI definitions - RFC 1103 */
#ifndef _NETINET_IF_FDDI_H
#define _NETINET_IF_FDDI_H

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

#define	FCF_FDDI		0x50	/* FDDI			*/

#define FDDI_ADDRLEN 	6 
#define FDDI_FRAMECHARS 7     /*  starting delim(1) + FCS(4) + 
				  ED(1) + Frame status(1) */
#define TRCHKFDDI 1
#define	RCF_ALL_BROADCAST	0x8000	/* all routes broadcast		*/
#define	RCF_LOCAL_BROADCAST	0x4000	/* single route broadcast	*/
#define	RCF_DIRECTION		0x0080	/* direction			*/
#define	RI_PRESENT		0x80	/* turn on bit 0 of byte 0 src addr */


#define	route_bytes_f(mac)	(((mac)->mac_rcf_f >> 8) & 0x1f)
#define	has_route_f(mac)		((mac)->mac_src_f[0] & RI_PRESENT)
#define	is_broadcast_f(mac)	(has_route_f(mac)		\
				 && ((mac)->mac_rcf_f & (RCF_LOCAL_BROADCAST \
							| RCF_ALL_BROADCAST)))

/*
 * FDDI MAC header
 */
struct fddi_mac_hdr {
	struct {
		u_char	reserved[3];	/* Reserved 3 bytes for adapter */
		u_char	Mac_fcf;	/* frame control field		*/
		u_char	Mac_dst[6];	/* destination address		*/
		u_char	Mac_src[6];	/* source address		*/
	} _First;
	struct {
		u_short Mac_rcf;	/* routing control field	*/
		u_short Mac_seg[14];	/* 30 routing segments 		*/
	} _Variable;
};

struct fddi_hdr {
	struct fddi_mac_hdr	fddi_mac;
	struct ie2_llc_snaphdr	fddi_llc;
};

#define	mac_fcf_f	_First.Mac_fcf
#define	mac_dst_f	_First.Mac_dst
#define	mac_src_f	_First.Mac_src
#define mac_rcf_f	_Variable.Mac_rcf
#define mac_seg_f	_Variable.Mac_seg

/*
 * mac_to_llc	-	given the ^ to the MAC, get to the LLC
 */
#define mac_to_llc_f(mac)	\
	(struct ie2_llc_snaphdr *) (((char *) (mac)) + mac_size_f(mac))

#define	mac_size_f(mac)		\
	(sizeof ((mac)->_First) + (has_route_f((mac)) ? route_bytes_f((mac)) : 0))


/*
 * For compatibility...
 */
#define	_FDDI_TYPE_IP		SNAP_TYPE_IP
#define _FDDI_TYPE_ARP		SNAP_TYPE_ARP
#define DSAP_NETWARE_FDDI 	DSAP_APPLETALK
#define SSAP_NETWARE_FDDI 	DSAP_APPLETALK

#define	_FDDI_MTU	4352

/*
 * FDDI Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	fddi_arp {
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
 * FDDI RAW socket addresses
 */
struct sockaddr_fddi {
	u_char			sa_len;    /* totals length */
	u_char			sa_family; /* address family: AF_802_2 */
	struct fddi_hdr		sa_data;   /* mac and llc data portion */
};


#ifdef	_KERNEL
char *fddi_sprintf();
#ifdef IP_MULTICAST
/*
 * Macro to map an IP multicast address to an FDDI multicast address.
 * The high-order 25 bits of the FDDI address are statically assigned,
 * and the low-order 23 bits are taken from the low end of the IP address.
 */
#define FDDI_MAP_IP_MULTICAST(ipaddr, fdaddr)			\
	/* struct in_addr *ipaddr; */					\
	/* u_char fdaddr[6];       */					\
{									\
	(fdaddr)[0] = 0x80;						\
	(fdaddr)[1] = 0x00;						\
	(fdaddr)[2] = 0x7A;						\
	(fdaddr)[3] = bit_reverse[((u_char *)ipaddr)[1]] & 0xfe;	\
	(fdaddr)[4] = bit_reverse[((u_char *)ipaddr)[2]];		\
	(fdaddr)[5] = bit_reverse[((u_char *)ipaddr)[3]];		\
}
#endif /* IP_MULTICAST */

#define FDDI_FILLIN_HDR(macp, srcp, destp, rcf, segp) \
/* struct fddi_mac_hdr 	*macp; \
caddr_t			srcp; \
caddr_t			destp; \
u_short			rcf; \
caddr_t			segp; \
*/ \
{ \
	*(struct char6 {char x[6];}*)(macp)->mac_dst_f = \
		*(struct char6 *)(destp); /* PERF */ \
	*(struct char6 *)(macp)->mac_src_f = \
		*(struct char6 *)(srcp); /* PERF */ \
	(macp)->mac_fcf_f     = FCF_FDDI; \
	if ((rcf)) { \
		(macp)->mac_src_f[0] |= RI_PRESENT; \
		(macp)->mac_rcf_f     = (rcf); \
		bcopy((segp), (macp)->mac_seg_f, mac_size_f((macp))); \
	} else \
		(macp)->mac_src_f[0] &= ~RI_PRESENT; \
	ie2_llc((caddr_t)((macp))+mac_size_f((macp)), _FDDI_TYPE_IP); \
}

#endif /* _KERNEL */
#endif /* _NETINET_IF_FDDI_H */
