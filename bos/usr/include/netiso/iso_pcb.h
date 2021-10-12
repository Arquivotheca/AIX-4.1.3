/* @(#)53	1.4  src/bos/usr/include/netiso/iso_pcb.h, sockinc, bos411, 9428A410j 5/10/91 16:40:43 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* $Header: iso_pcb.h,v 4.3 88/06/29 15:00:01 hagens Exp $ */
/* $Source: /usr/argo/sys/netiso/RCS/iso_pcb.h,v $ */
/*	(#)iso_pcb.h	7.3 (Berkeley) 8/29/89 */

#define	MAXX25CRUDLEN	16	/* 16 bytes of call request user data */

/*
 * Common structure pcb for argo protocol implementation.
 */
struct isopcb {
	struct	isopcb			*isop_next,*isop_prev; /* pointers to other pcb's */
	struct	isopcb			*isop_head;	/* pointer back to chain of pcbs for 
								this protocol */
	struct	socket			*isop_socket;	/* back pointer to socket */
	struct	sockaddr_iso	*isop_laddr;
	struct	sockaddr_iso	*isop_faddr;
	struct	route_iso {
		struct	rtentry 	*ro_rt;
		struct	sockaddr_iso ro_dst;
	}						isop_route;			/* CLNP routing entry */
	struct	mbuf			*isop_options;		/* CLNP options */
	struct	mbuf			*isop_optindex;		/* CLNP options index */
	struct	mbuf			*isop_clnpcache;	/* CLNP cached hdr */
	u_int			isop_chanmask;		/* which ones used - max 32 supported */
	u_int			isop_negchanmask;	/* which ones used - max 32 supported */
	u_short					isop_lport;			/* MISLEADLING work var */
	int						isop_x25crud_len;	/* x25 call request ud */
	char					isop_x25crud[MAXX25CRUDLEN];
	struct ifaddr			*isop_ifa;		/* ESIS interface assoc w/sock */
	struct	sockaddr_iso	isop_sladdr,		/* preallocated laddr */
							isop_sfaddr;		/* preallocated faddr */
};

#ifdef sotorawcb
/*
 * Common structure pcb for raw clnp protocol access.
 * Here are clnp specific extensions to the raw control block,
 * and space is allocated to the necessary sockaddrs.
 */
struct rawisopcb {
	struct	rawcb risop_rcb;		/* common control block prefix */
	int		risop_flags;			/* flags, e.g. raw sockopts */
	struct	isopcb risop_isop;		/* space for bound addresses, routes etc.*/
};
#endif

#define	sotoisopcb(so)	((struct isopcb *)(so)->so_pcb)
#define	sotorawisopcb(so)	((struct rawisopcb *)(so)->so_pcb)

#ifdef _KERNEL
struct	isopcb *iso_pcblookup();
#endif
