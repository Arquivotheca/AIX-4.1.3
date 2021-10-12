/* @(#)61	1.8.1.13  src/bos/kernel/net/if_arp.h, sockinc, bos411, 9428A410j 3/15/94 17:09:58 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: ACMULTI_LOCK
 *		ACMULTI_LOCKINIT
 *		ACMULTI_UNLOCK
 *		ARPTAB_HASH
 *		ARPTAB_LOCK
 *		ARPTAB_LOCKINIT
 *		ARPTAB_LOOK
 *		ARPTAB_UNLOCK
 *		
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_H_IF_ARP
#define	_H_IF_ARP

#include <netinet/in.h>

/*
 * Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  ARP packets are variable
 * in size; the arphdr structure defines the fixed-length portion.
 * Protocol type values are the same as those for 10 Mb/s Ethernet.
 * It is followed by the variable-sized fields ar_sha, arp_spa,
 * arp_tha and arp_tpa in that order, according to the lengths
 * specified.  Field names used correspond to RFC 826.
 */
struct	arphdr {
	u_short	ar_hrd;		/* format of hardware address */
#define ARPHRD_ETHER 	1	/* ethernet hardware address	*/
#define	ARPHRD_802_5	6	/* 802.5 hardware address	*/
#define	ARPHRD_802_3	6	/* 802.3 hardware address	*/
#define ARPHRD_FDDI	1	/* FDDI  hardware address	*/
	u_short	ar_pro;		/* format of protocol address */
	u_char	ar_hln;		/* length of hardware address */
	u_char	ar_pln;		/* length of protocol address */
	u_short	ar_op;		/* one of: */
#define	ARPOP_REQUEST	1	/* request to resolve address */
#define	ARPOP_REPLY	2	/* response to previous request */
/*
 * The remaining fields are variable in size,
 * according to the sizes above.
 */
/*	u_char	ar_sha[];	   sender hardware address */
/*	u_char	ar_spa[];	   sender protocol address */
/*	u_char	ar_tha[];	   target hardware address */
/*	u_char	ar_tpa[];	   target protocol address */
};

/*  arp_flags and at_flags field values */
#define	ATF_INUSE	0x01	/* entry in use */
#define ATF_COM		0x02	/* completed entry (enaddr valid) */
#define	ATF_PERM	0x04	/* permanent entry */
#define	ATF_PUBL	0x08	/* publish entry (respond for other host) */
#define	ATF_USETRAILERS	0x10	/* has requested trailers */
#define	ATF_ASK		0x20	/* arp request outstanding. */

union if_dependent {
	struct   token  {
		u_short	u_rcf;		/* route control field */
		u_short	u_seg[8];	/* routing info */
	} token;
	struct  x25 {
		u_short	u_session_id;	/* x.25 session id */
		u_short	u_unit;		/* unit number     */
	} x25;
	struct fddi {
		u_short u_rcf;		/* route control field 	*/
		u_short u_seg[16];	/* routing info 	*/
	} fddi;
};

#define  MAX_HWADDR		20

/*
 * Internet to link layer address resolution table.
 */
struct  arptab {
	struct	in_addr	at_iaddr;	/* internet address */
	u_char 		hwaddr[MAX_HWADDR];	/* hardware address */
	u_char		at_timer;	/* minutes since last reference */
	u_char		at_flags;	/* flags */
	struct  mbuf	*at_hold;	/* last pkt until resolved/timeout */
	struct 	ifnet	*at_ifp;	/* ifnet associated with entry */
	union if_dependent if_dependent;/* hdwr dependent info */
};

#define	at_enaddr	hwaddr
#define	at_traddr	hwaddr
#define	at_ie3addr	hwaddr
#define	at_x25addr	hwaddr
#define	at_fdaddr	hwaddr

#define	at_rcf		if_dependent.token.u_rcf
#define	at_seg		if_dependent.token.u_seg
#define	at_session_id	if_dependent.x25.u_session_id
#define	at_x25unit	if_dependent.x25.u_unit
#define at_fddi_rcf	if_dependent.fddi.u_rcf
#define at_fddi_seg 	if_dependent.fddi.u_seg

/*
 * ARP ioctl request
 */
struct	arpreq {
	struct	sockaddr	arp_pa;		/* protocol address */
	struct  sockaddr	arp_ha;		/* hardware address */
	int			arp_flags;	/* flags */
	u_short		at_length;		/* length of hardware address */
	union if_dependent ifd;			/* hdwr dependent info */
	u_long	ifType;				/* interface type */
};

#define	arp_rcf		ifd.token.u_rcf
#define	arp_seg		ifd.token.u_seg
#define	arp_session_id	ifd.x25.u_session_id
#define	arp_x25unit	ifd.x25.u_unit
#define arp_fddi_rcf	ifd.fddi.u_rcf
#define arp_fddi_seg 	ifd.fddi.u_seg

#define IN_USE  0x0001          /* interface entry in use when bit = = 1 */

/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 */
struct  arpcom {
	struct  ifnet	ac_if;			/* network visible interface */
	u_char		ac_hwaddr[MAX_HWADDR];	/* hardware address */
	struct	in_addr ac_ipaddr;		/* copy of IP address */
#ifdef IP_MULTICAST
	struct driver_multi *ac_multiaddrs;      /* list of multicast addrs */
#if    NETSYNC_LOCK
	simple_lock_data_t	ac_multi_lock;   /* lock for walking list of 
							multicast addrs */
#endif
#endif
};

#define ac_enaddr	ac_hwaddr
#define ac_traddr	ac_hwaddr
#define ac_fdaddr 	ac_hwaddr

#ifdef		GATEWAY
#define	ARPTAB_BSIZ	7		/* bucket size */
#define	ARPTAB_NB	25		/* number of buckets */
#else
#define	ARPTAB_BSIZ	5		/* bucket size */
#define	ARPTAB_NB	20		/* number of buckets */
#endif

#ifdef _KERNEL
#if     NETSYNC_LOCK
extern Simple_lock arptab_lock;
#include <sys/lock_alloc.h>
#define ARPTAB_LOCKINIT(lockp)  { \
	lock_alloc((lockp), LOCK_ALLOC_PIN, ARPTAB_LOCK_FAMILY, -1); \
	simple_lock_init((lockp)); \
}
#define ARPTAB_LOCK(lockp)      s = disable_lock(PL_IMP, (lockp));
#define ARPTAB_UNLOCK(lockp)    unlock_enable(s, (lockp));
#else
#define ARPTAB_LOCKINIT(lockp)
#define ARPTAB_LOCK(lockp)	NETSPL(s,imp)
#define ARPTAB_UNLOCK(lockp)	NETSPLX(s)
#endif

extern struct arptab	*arptabp;
extern int		arptabbsiz;
extern int		arptabnb;
extern int		arptabsize;

extern int	arptab_bsiz;
extern int	arptab_bsiz_dflt;
extern int	arptab_nb;
extern int	arptab_nb_dflt;


#define	ARPTAB_HASH(a)		\
		((u_long)(a) % arptabnb)

#define	ARPTAB_LOOK(at, addr) {	\
		register n;	\
		at = &arptabp[ARPTAB_HASH(addr) * arptabbsiz];	\
		for (n = 0 ; n < arptabbsiz ; n++, at++)	\
			if (at->at_iaddr.s_addr == addr)	\
				break;		\
		if  (n >= arptabbsiz)		\
			at = 0; \
}
#endif /* _KERNEL */

/* timer values */
#define	ARPT_AGE	(60 * 1)	/* aging timer, 1 min. */
#define	ARPT_KILLC	20		/* kill completed entry in 20 minutes */
#define	ARPT_KILLI	3		/* kill incomplete entry in 3 minutes */

#define  MAX_NASTIES  69	/* of course */

#ifdef IP_MULTICAST
/*
 * Device driver multicast address structure.  There is one of these for each
 * multicast address or range of multicast addresses that we are supposed
 * to listen to on a particular interface.  They are kept in a linked list,
 * rooted in the interface's arpcom structure.  (This really has nothing to
 * do with ARP, or with the Internet address family, but this appears to be
 * the minimally-disrupting place to put it.)
 */
struct driver_multi {
	u_char              enm_addrlo[6];/* low  or only address of range */
	u_char              enm_addrhi[6];/* high or only address of range */
	struct arpcom      *enm_ac;	  /* back pointer to arpcom        */
	u_int               enm_refcount; /* no. claims to this addr/range */
	struct driver_multi *enm_next;	  /* ptr to next driver_multi       */
};

#if     NETSYNC_LOCK
#define ACMULTI_LOCK_DECL()    int _acml;
#define ACMULTI_LOCK(acp)      _acml = disable_lock(PL_IMP, &((acp)->ac_multi_lock))
#define ACMULTI_UNLOCK(acp)    unlock_enable(_acml, &((acp)->ac_multi_lock))
#define ACMULTI_LOCKINIT(acp)  {					\
	lock_alloc(&((acp)->ac_multi_lock), LOCK_ALLOC_PIN, \
		ACMULTI_LOCK_FAMILY, acp);	\
	simple_lock_init(&((acp)->ac_multi_lock));			\
}
#else
#define ACMULTI_LOCK(acp)      NETSPL(s, net)
#define ACMULTI_UNLOCK(acp)    NETSPLX(s)
#define ACMULTI_LOCKINIT(acp)
#endif

#endif /* IP_MULTICAST */

#endif	/* _H_IF_ARP */
