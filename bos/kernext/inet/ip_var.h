/* @(#)53	1.20  src/bos/kernext/inet/ip_var.h, sockinc, bos411, 9428A410j 3/15/94 16:58:01 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: IPFRAG_LOCK
 *		IPFRAG_LOCKINIT
 *		IPFRAG_UNLOCK
 *		IPMISC_LOCK
 *		IPMISC_LOCKINIT
 *		IPMISC_UNLOCK
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
 *	Base:	ip_var.h	7.6 (Berkeley) 9/20/89
 *	Merged:	ip_var.h	7.7 (Berkeley) 6/28/90
 */

#ifndef	_NETINET_IN_VAR_H
#define	_NETINET_IN_VAR_H

#include <netinet/ip.h>

/*
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly {
	caddr_t	ih_next, ih_prev;	/* for protocol sequence q's */
	u_char	ih_x1;			/* (unused) */
	u_char	ih_pr;			/* protocol */
	u_short	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

/*
 * Ip reassembly queue structure.  Each fragment
 * being reassembled is attached to one of these structures.
 * They are timed out after ipq_ttl drops to 0, and may also
 * be reclaimed if memory becomes tight.
 */
struct ipq {
	struct	ipq *next,*prev;	/* to other reass headers */
	struct	ipasfrag *ipq_next,*ipq_prev;
					/* to ip headers of fragments */
	u_char	ipq_ttl;		/* time for reass q to live */
	u_char	ipq_p;			/* protocol of this fragment */
	u_short	ipq_id;			/* sequence id for reassembly */
	struct	in_addr ipq_src,ipq_dst;
};

/*
 * Ip header, when holding a fragment.
 */
struct	ipasfrag {
	struct	ipasfrag *ipf_next,*ipf_prev;	/* fragment queue */
	struct	mbuf *ipf_mbuf;		/* initial mbuf */
	struct	ip ipf_ip;		/* ip header */
#define	ipf_vhl	ipf_ip.ip_vhl
#define	ipf_len	ipf_ip.ip_len
#define	ipf_off	ipf_ip.ip_off
#define	ipf_id	ipf_ip.ip_id
#define	ipf_p	ipf_ip.ip_p
#define	ipf_mff	ipf_ip.ip_sum		/* copied from (ip_off&IP_MF) */
#define	ipf_src	ipf_ip.ip_src
#define	ipf_dst	ipf_ip.ip_dst
};

/*
 * Structure stored in mbuf in inpcb.ip_options
 * and passed to ip_output when ip options are in use.
 * The actual length of the options (including ipopt_dst)
 * is in m_len.
 */
#define MAX_IPOPTLEN	40

struct ipoption {
	struct	in_addr ipopt_dst;	/* first-hop dst if source routed */
	char	ipopt_list[MAX_IPOPTLEN];	/* options proper */
};

#ifdef IP_MULTICAST
/*
 * Structure stored in an mbuf attached to inpcb.ip_moptions and
 * passed to ip_output when IP multicast options are in use.
 */
struct ip_moptions {
	struct	ifnet   *imo_multicast_ifp;  /* ifp for outgoing multicasts */
	u_char	         imo_multicast_ttl;  /* TTL for outgoing multicasts */
	u_char		 imo_multicast_loop; /* 1 => hear sends if a member */
	u_short		 imo_num_memberships;/* no. memberships this socket */
	struct in_multi *imo_membership[IP_MAX_MEMBERSHIPS];
};
#endif

/* Source route holding structure (moved here from ip_input.c) */
struct ip_srcrt {
	struct	in_addr dst;			/* final destination */
	char	nop;				/* one NOP to align */
	char	srcopt[IPOPT_OFFSET + 1];	/* OPTVAL, OLEN and OFFSET */
	struct	in_addr route[MAX_IPOPTLEN/sizeof(struct in_addr)];
};

struct	ipstat {
	long	ips_total;		/* total packets received */
	long	ips_badsum;		/* checksum bad */
	long	ips_tooshort;		/* packet too short */
	long	ips_toosmall;		/* not enough data */
	long	ips_badhlen;		/* ip header length < data size */
	long	ips_badlen;		/* ip length < ip header length */
	long	ips_fragments;		/* fragments received */
	long	ips_fragdropped;	/* frags dropped (dups, out of space) */
	long	ips_fragtimeout;	/* fragments timed out */
	long	ips_forward;		/* packets forwarded */
	long	ips_cantforward;	/* packets rcvd for unreachable dest */
	long	ips_redirectsent;	/* packets forwarded on same net */
	long	ips_noproto;		/* unknown or unsupported protocol */
	long	ips_delivered;		/* packets consumed here */
	long	ips_localout;		/* total ip packets generated here */
	long	ips_odropped;		/* lost packets due to nobufs, etc. */
	long	ips_reassembled;	/* total packets reassembled ok */
	long	ips_fragmented;		/* output packets fragmented ok */
	long	ips_ofragments;		/* output fragments created */
	long	ips_cantfrag;		/* don't fragment flag was set, etc. */
        u_long  ips_badoptions;         /* error in option processing */
        u_long  ips_noroute;            /* packets discarded due to no route */
        u_long  ips_badvers;            /* ip version != 4 */
        u_long  ips_rawout;             /* total raw ip packets generated */
        /* SNMP BASE begin */
        u_long  ipInHdrErrors;          /* # of header erros: also add
                                           ips_badsum, ips_tooshort,
                                           ips_toosmall, ips_badhlen and
                                           ips_badlen from above */
        u_long  ipInAddrErrors;         /* # of datagrams with ip addr errors*/
        u_long  ipInDiscards;           /* # of input datagrams discarded */
#ifdef IP_MULTICAST
        u_long  ipInMAddrErrors;         /* # of dropped multicast datagrams 
					    because we are not receiving on 
					    this address */
#endif /* IP_MULTICAST */
        /* SNMP BASE end */
#if	defined(_KERNEL) && LOCK_NETSTATS
	simple_lock_data_t ips_lock;	/* statistics lock */
#endif
};

#ifdef _KERNEL
#if	NETSYNC_LOCK
extern	simple_lock_data_t	ip_frag_lock;
extern	simple_lock_data_t	ip_misc_lock;
#define IPFRAG_LOCKINIT()	{					\
	lock_alloc(&ip_frag_lock, LOCK_ALLOC_PIN, IPFRAG_LOCK_FAMILY, -1);\
	simple_lock_init(&ip_frag_lock);		\
}
#define IPFRAG_LOCK_DECL()	int	_ipfl;
#define IPFRAG_LOCK()		_ipfl = disable_lock(PL_IMP, &ip_frag_lock)
#define IPFRAG_UNLOCK()		unlock_enable(_ipfl, &ip_frag_lock)
#else	/* !NETSYNC_LOCK */
#define IPMISC_LOCKINIT()
#define IPMISC_LOCK()
#define IPMISC_UNLOCK()
#define IPFRAG_LOCKINIT()
#define IPFRAG_LOCK()
#define IPFRAG_UNLOCK()
#endif

/* flags passed to ip_output as last parameter */
#define	IP_FORWARDING		0x1		/* most of ip header exists */
#define IP_RAWOUTPUT            0x2             /* raw ip header exists */
#ifdef IP_MULTICAST
#define IP_MULTICASTOPTS        0x4             /* multicast opts present */
#define IP_IFMULTI_NOLOCK	0x8		/* Don't grab ifmulti lock */
#endif
#define	IP_ROUTETOIF		SO_DONTROUTE	/* bypass routing tables */
#define	IP_ALLOWBROADCAST	SO_BROADCAST	/* can send broadcast packets */

extern	CONST u_char inetctlerrmap[];
extern	struct	ipstat	ipstat;
extern	struct	ipq	ipq;			/* ip reass. queue */
extern	u_short	ip_id;				/* ip packet ctr, for ids */
#endif
#endif
