/* @(#)39     1.16.2.12  src/bos/kernel/net/if.h, sockinc, bos411, 9428A410j 5/25/94 17:25:14 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: IFMULTI_INITLOCK
 *		IFMULTI_LOCK
 *		IFMULTI_LOCK_RECURSIVE
 *		IFMULTI_LOCKINIT
 *		IFMULTI_UNLOCK_RECURSIVE
 *		IFQ_LOCK
 *		IFQ_LOCKINIT
 *		IFQ_UNLOCK
 *		IF_DEQUEUE
 *		IF_DEQUEUE_NOLOCK
 *		IF_DROP
 *		IF_ENQUEUE
 *		IF_ENQUEUE_NOLOCK
 *		IF_PREPEND
 *		IF_PREPEND_NOLOCK
 *		IF_QFULL
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

#ifndef _NET_IF_H
#define _NET_IF_H

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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	Base:	if.h	7.6 (Berkeley) 9/20/89
 *	Merged: if.h	7.9 (Berkeley) 6/28/90
 */

/*
 * Structures defining a network interface, providing a packet
 * transport mechanism (ala level 0 of the PUP protocols).
 *
 * Each interface accepts output datagrams of a specified maximum
 * length, and provides higher level routines with input datagrams
 * received from its medium.
 *
 * Output occurs when the routine if_output is called, with four parameters:
 *	(*ifp->if_output)(ifp, m, dst, ro)
 * Here m is the mbuf chain to be sent and dst is the destination address.
 * The output routine encapsulates the supplied datagram if necessary,
 * and then transmits it on its medium.
 *
 * On input, each interface unwraps the data received by it, and either
 * places it on the input queue of a internetwork datagram routine
 * and posts the associated software interrupt, or passes the datagram to a raw
 * packet input routine.
 *
 * Routines exist for locating interfaces by their addresses
 * or for locating a interface on a certain network, as well as more general
 * routing and gateway routines maintaining information used to locate
 * interfaces.  These routines live in the files if.c and route.c
 */

#if	defined(_KERNEL) && !defined(_NET_GLOBALS_H_)
#include "net/net_globals.h"
#endif

#ifndef	_TIME_
#ifdef	_KERNEL
#include "sys/time.h"
#else
#include <sys/time.h>
#endif
#endif

/*
 * Structure defining a queue for a network interface.
 */

struct ifnet {
	char	*if_name;		/* name, e.g. ``en'' or ``lo'' */
	short	if_unit;		/* sub-unit for lower level driver */
	u_long	if_mtu;			/* maximum transmission unit */
	u_long	if_flags;		/* up/down, broadcast, etc. */
	short	if_timer;		/* time 'til if_watchdog called */
	int	if_metric;		/* routing metric (external only) */
	struct	ifaddr *if_addrlist;	/* linked list of addresses per if */
/* procedure handles */
#ifdef	_KERNEL
	int	(*if_init)(int);
	int	(*if_output)(struct ifnet *, struct mbuf *,
				struct sockaddr *, struct rtentry *);
	int	(*if_start)(struct ifnet *);
	int	(*if_done)(struct ifnet *);
	int	(*if_ioctl)(struct ifnet *, int, caddr_t);
	int	(*if_reset)(int, int);
	int	(*if_watchdog)(int);
#else
	int	(*if_init)();		/* init routine */
	int	(*if_output)();		/* output routine (enqueue) */
	int	(*if_start)();		/* initiate output routine */
	int	(*if_done)();		/* output complete routine */
	int	(*if_ioctl)();		/* ioctl routine */
	int	(*if_reset)();		/* bus reset routine */
	int	(*if_watchdog)();	/* timer routine */
#endif
/* generic interface statistics */
	int	if_ipackets;		/* packets received on interface */
	int	if_ierrors;		/* input errors on interface */
	int	if_opackets;		/* packets sent on interface */
	int	if_oerrors;		/* output errors on interface */
	int	if_collisions;		/* collisions on csma interfaces */
/* end statistics */
	struct	ifnet *if_next;
	u_char	if_type;		/* ethernet, tokenring, etc */
	u_char	if_addrlen;		/* media address length */
	u_char	if_hdrlen;		/* media header length */
	u_char	if_index;		/* numeric abbreviation for this if  */
/* SNMP statistics */
	struct	timeval if_lastchange;	/* last updated */
	int	if_ibytes;		/* total number of octets received */
	int	if_obytes;		/* total number of octets sent */
	int	if_imcasts;		/* packets received via multicast */
	int	if_omcasts;		/* packets sent via multicast */
	int	if_iqdrops;		/* dropped on input, this interface */
	int	if_noproto;		/* destined for unsupported protocol */
	int	if_baudrate;		/* linespeed */
/* netstat -D statistics */
	u_long	if_arpdrops;		/* dropped because no arp response */
	int	if_reserved[8];		/* reserved for future use */
/* stuff for device driver */
        dev_t   devno;                  /* device number */
        chan_t  chan;                   /* channel of mpx device */
#ifdef IP_MULTICAST
	struct	in_multi *if_multiaddrs; /* list of multicast addresses */
#endif
	int	(*if_tap)();		/* packet tap */
	caddr_t if_tapctl;              /* link for tap (ie BPF) */
	int	(*if_arpres)();		/* arp resolver routine */
	int	(*if_arprev)();		/* Reverse-ARP input routine */
	int	(*if_arpinput)();	/* arp input routine */
	struct	ifqueue {
		struct	mbuf *ifq_head;
		struct	mbuf *ifq_tail;
		int	ifq_len;
		int	ifq_maxlen;
		int	ifq_drops;
#ifndef	_KERNEL
#define simple_lock_data_t int
#define lock_data_t      struct { int a; int b; } 
#endif
#ifdef NETSYNC_LOCK
		simple_lock_data_t	ifq_slock;
#endif
	} if_snd;			/* output queue */
#ifdef	LOCK_NETSTATS
	simple_lock_data_t	if_slock;	/* statistics lock */
#endif
#ifdef IP_MULTICAST
        simple_lock_data_t	if_multi_lock;
#endif
};

#ifdef IP_MULTICAST
#if NETSYNC_LOCK
#define IFMULTI_LOCK_DECL()	  int 	_ifml;
#define IFMULTI_LOCK(ifp)	  _ifml = disable_lock(PL_IMP, &(ifp)->if_multi_lock)
#define IFMULTI_UNLOCK(ifp)       unlock_enable(_ifml, &(ifp)->if_multi_lock)
#define IFMULTI_INITLOCK(ifp) {						\
	lock_alloc(&(ifp)->if_multi_lock, LOCK_ALLOC_PIN, \
		IFMULTI_LOCK_FAMILY, ifp); \
	simple_lock_init(&(ifp)->if_multi_lock); 			\
} 
#define IFMULTI_LOCK_RECURSIVE(ifp)	  IFMULTI_LOCK(ifp)
#define IFMULTI_UNLOCK_RECURSIVE(ifp)     IFMULTI_UNLOCK(ifp)
#else
#define IFMULTI_LOCK(ifp)      		 
#define IFMULTI_UNLOCK(ifp)    		 
#define IFMULTI_LOCKINIT(ifp)
#define IFMULTI_LOCK_RECURSIVE(ifp)	 NETSPL(s, net) 
#define IFMULTI_UNLOCK_RECURSIVE(ifp)    NETSPLX(s) 
#endif
#endif

#ifndef IFF_UP
#define	IFF_UP		0x1		/* interface is up */
#endif
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#ifndef IFF_RUNNING
#define	IFF_RUNNING	0x40		/* resources allocated */
#endif
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define	IFF_OACTIVE	0x400		/* transmission in progress */
#define	IFF_SIMPLEX	0x800		/* can't hear own transmissions */

#ifdef	_AIX
#define IFF_DO_HW_LOOPBACK  0x10000	/* force loopback thru hardware */
#define	IFF_ALLCAST	0x20000		/* global broadcast		 */
#define	IFF_BRIDGE	0x40000		/* receive all bridge packets */
#define	IFF_NOECHO	IFF_SIMPLEX	/* receives echo packets */
#endif
#define IFF_BPF		0x8000000	/* bpf is supported for this IF */

/* Device-specific flags */
#define IFF_D1		0x8000
#define IFF_D2		0x4000
#define IFF_D3		0x2000
#define IFF_D4		0x1000
#define IFF_SNAP	IFF_D1		/* Ethernet driver outputs SNAP hdr */

#ifdef IP_MULTICAST
#define	IFF_MULTICAST	0x80000		/* supports multicast */
/*
 * The IFF_MULTICAST flag indicates that the network can support the
 * transmission and reception of higher-level (e.g., IP) multicast packets.
 * It is independent of hardware support for multicasting; for example,
 * point-to-point links or pure broadcast networks may well support
 * higher-level multicasts.
 */

#define	IFF_CANTCHANGE	\
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_SIMPLEX|IFF_RUNNING|IFF_OACTIVE|IFF_MULTICAST)
#else
/* flags set internally only: */
#define	IFF_CANTCHANGE	\
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_SIMPLEX|IFF_RUNNING|IFF_OACTIVE)
#endif

/*
 * Multiprocessor queue locking.
 *
 * Note that the IF_QFULL and IF_DROP macros become racy in an mp environment.
 * The exact number of ifq_drops probably isn't important; on the other hand,
 * it is possible that an unlocked ifq could grow larger than its declared
 * ifq_maxlen as processors race between IF_QFULL and IF_ENQUEUE.  However,
 * it is still ABSOLUTELY NECESSARY that modification of ifq_len be locked!
 *
 */
#define IFQ_LOCK_DECL()		int	_qs;
#define IFQ_LOCKINIT(ifq) {						\
	lock_alloc(&((ifq)->ifq_slock), LOCK_ALLOC_PIN, IFQ_LOCK_FAMILY, ifq);\
	simple_lock_init(&((ifq)->ifq_slock));				\
}
#define IFQ_LOCK(ifq)		_qs = disable_lock(PL_IMP ,&((ifq)->ifq_slock))
#define IFQ_UNLOCK(ifq)		unlock_enable(_qs, &((ifq)->ifq_slock))

/*
 * Output queues (ifp->if_snd) and internetwork datagram level (pup level 1)
 * input routines have queues of messages stored on ifqueue structures
 * (defined above).  Entries are added to and deleted from these structures
 * by these macros, which should be called with ipl raised to splimp().
 */
#define	IF_QFULL(ifq)		((ifq)->ifq_len >= (ifq)->ifq_maxlen)
#define	IF_DROP(ifq)		((ifq)->ifq_drops++)
#define	IF_ENQUEUE_NOLOCK(ifq, m) { \
	(m)->m_nextpkt = 0; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_head = m; \
	else \
		(ifq)->ifq_tail->m_nextpkt = m; \
	(ifq)->ifq_tail = m; \
	(ifq)->ifq_len++; \
}
#define	IF_ENQUEUE(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_ENQUEUE_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}
#define	IF_PREPEND_NOLOCK(ifq, m) { \
	(m)->m_nextpkt = (ifq)->ifq_head; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_tail = (m); \
	(ifq)->ifq_head = (m); \
	(ifq)->ifq_len++; \
}
#define	IF_PREPEND(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_PREPEND_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}
#define	IF_DEQUEUE_NOLOCK(ifq, m) { \
	(m) = (ifq)->ifq_head; \
	if (m) { \
		if (((ifq)->ifq_head = (m)->m_nextpkt) == 0) \
			(ifq)->ifq_tail = 0; \
		(m)->m_nextpkt = 0; \
		(ifq)->ifq_len--; \
	} \
}
#define	IF_DEQUEUE(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_DEQUEUE_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}

#define	IFQ_MAXLEN	50
#define	IFNET_SLOWHZ	1		/* granularity is 1 second */

/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct	sockaddr *ifa_addr;	/* address of interface */
	struct	sockaddr *ifa_dstaddr;	/* other end of p-to-p link */
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
	struct	sockaddr *ifa_netmask;	/* used to determine subnet */
	struct	ifnet *ifa_ifp;		/* back-pointer to interface */
	struct	ifaddr *ifa_next;	/* next address for interface */
#ifdef	_KERNEL
	void	(*ifa_rtrequest)(int, struct rtentry *, struct sockaddr *);
#else
	void	(*ifa_rtrequest)();	/* check or clean routes (+ or -)'d */
#endif
	struct	rtentry *ifa_rt;	/* ??? for ROUTETOIF */
	u_short	ifa_flags;		/* mostly rt_flags for cloning */
	short	ifa_refcnt;		/* extra to malloc for link info */
};
#define IFA_ROUTE	RTF_UP		/* route installed */
/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
#define	IFNAMSIZ	16
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		u_long	ifru_flags;
		int	ifru_metric;
		caddr_t	ifru_data;
#ifdef	_AIX
		u_long   ifru_mtu;
#endif
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
#ifdef	_AIX
#define ifr_mtu         ifr_ifru.ifru_mtu       /* mtu of interface */
#endif
};

struct ifaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr ifra_addr;
	struct	sockaddr ifra_broadaddr;
	struct	sockaddr ifra_mask;
};

#ifdef	_AIX	/* for AIX 3.1 binary compatibility */
struct  oifreq {
        char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
        union {
                struct  sockaddr ifru_addr;
                struct  sockaddr ifru_dstaddr;
                struct  sockaddr ifru_broadaddr;
                long    ifru_flags;
                int     ifru_metric;
                caddr_t ifru_data;
                u_long  ifru_mtu;
        } ifr_ifru;
        u_char  reserved[8];
};
#endif

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

#ifdef	_KERNEL
#include "net/if_arp.h"
extern struct	ifnet *ifnet, loif;
#ifdef	NETSYNC_LOCK
extern simple_lock_data_t	ifa_refcnt_lock;
#define IFAREFCNT_LOCK_DECL()	int	_ifa_refcnt;
#define IFAREFCNT_LOCK()	_ifa_refcnt = disable_lock(PL_IMP, &ifa_refcnt_lock);
#define IFAREFCNT_UNLOCK()	unlock_enable(_ifa_refcnt, &ifa_refcnt_lock);
#else
#define IFAREFCNT_LOCK_DECL()
#define IFAREFCNT_LOCK()
#define IFAREFCNT_UNLOCK()
#endif
extern void ifafree(struct ifaddr *);
#define IFAFREE(ifa) \
	if ((ifa)->ifa_refcnt <= 0) \
		ifafree(ifa); \
	else \
		(ifa)->ifa_refcnt--;
#else
#include <net/if_arp.h>
#endif

#ifdef _AIX
/* 
 * These defines should not be used.  They are here for compatibility with
 * previous releases.  Please use types defined in <net/if_types.h>.
 */
#ifndef _NET_IF_TYPES_H
#include "net/if_types.h"
#endif

#define UCAST_TYPE	1
#define NUCAST_TYPE	2

#define OTHER_TYPE      IFT_OTHER               /* other network type */
#define REGULAR1822     IFT_1822
#define HDH1822         IFT_HDH1822
#define DDN_X25         IFT_X25DDN
#define RFC877_X25      IFT_X25
#define ETHERNET_CSMACD IFT_ETHER
#define ISO88023_CSMACD IFT_ISO88023
#define ISO88024_TOKBUS IFT_ISO88024
#define ISO88025_TOKRNG IFT_ISO88025
#define ISO88026_MAN    IFT_ISO88026
#define STARLAN         IFT_STARLAN
#define PROTEON_10MBIT  IFT_P10
#define PROTEON_80MBIT  IFT_P80
#define HYPERCHANNEL    IFT_HY
#define FDDI            IFT_FDDI
#define LAPB            IFT_LAPB
#define SDLC            IFT_SDLC
#define T1_CARRIER      IFT_T1
#define CEPT            IFT_CEPT              /* European equivalent of T-1 */
#define BASICISDN       IFT_ISDNBASIC
#define PRIMARYISDN     IFT_ISDNPRIMARY
#define PROPPTPSERIAL   IFT_PTPSERIAL              /* Proprietary Serail */
#endif /* _AIX */
#endif /* _NET_IF_H */
