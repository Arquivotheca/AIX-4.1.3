/* @(#)50	1.14  src/bos/kernext/inet/in_var.h, sockinc, bos411, 9428A410j 7/7/94 16:18:38 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: IA_SIN
 *		IFP_TO_IA
 *		INADDR_TO_IFP
 *		INIFADDR_LOCKINIT
 *		INIFADDR_READ_LOCK
 *		INIFADDR_UNLOCK
 *		INIFADDR_WRITE_LOCK
 *		IN_FIRST_MULTI
 *		IN_LOOKUP_MULTI
 *		IN_LOOKUP_MULTI_NOLOCK
 *		IN_NEXT_MULTI
 *		UNLOCK_LAST_MULTI
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
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1985, 1986 Regents of the University of California.
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
 *	Base:	in_var.h	7.4 (Berkeley) 4/22/89
 *	Merged:	in_var.h	7.6 (Berkeley) 6/28/90
 */

/*
 * Interface address, Internet version.  One of these structures
 * is allocated for each interface with an Internet address.
 * The ifaddr structure contains the protocol-independent part
 * of the structure and is assumed to be first.
 */
struct in_ifaddr {
	struct	ifaddr ia_ifa;		/* protocol-independent info */
#define	ia_ifp		ia_ifa.ifa_ifp
#define ia_flags	ia_ifa.ifa_flags
					/* ia_{,sub}net{,mask} in host order */
	u_long	ia_net;			/* network number of interface */
	u_long	ia_netmask;		/* mask of net part */
	u_long	ia_subnet;		/* subnet number, including net */
	u_long	ia_subnetmask;		/* mask of subnet part */
	struct	in_addr ia_netbroadcast; /* to recognize net broadcasts */
	struct	in_ifaddr *ia_next;	/* next in list of internet addresses */
	struct	sockaddr_in ia_addr;	/* reserve space for interface name */
	struct	sockaddr_in ia_dstaddr; /* reserve space for broadcast addr */
#define	ia_broadaddr	ia_dstaddr
	struct	sockaddr_in ia_sockmask; /* reserve space for general netmask */
};

struct	in_aliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr_in ifra_addr;
	struct	sockaddr_in ifra_broadaddr;
#define ifra_dstaddr ifra_broadaddr
	struct	sockaddr_in ifra_mask;
};
/*
 * Given a pointer to an in_ifaddr (ifaddr),
 * return a pointer to the addr as a sockaddr_in.
 */
#define	IA_SIN(ia) (&(((struct in_ifaddr *)(ia))->ia_addr))

#ifdef	_KERNEL
#if     NETSYNC_LOCK
extern  simple_lock_data_t     inifaddr_lock;
#define INIFADDR_LOCKINIT()     {					\
	lock_alloc(&inifaddr_lock, LOCK_ALLOC_PIN, INIFADDR_LOCK_FAMILY, -1);\
	simple_lock_init(&inifaddr_lock);		\
}
#define INIFADDR_LOCK_DECL()	int	_inifl;
#define INIFADDR_WRITE_LOCK()   _inifl = disable_lock(PL_IMP, &inifaddr_lock)
#define INIFADDR_READ_LOCK()    _inifl = disable_lock(PL_IMP, &inifaddr_lock)
#define INIFADDR_UNLOCK()       unlock_enable(_inifl, &inifaddr_lock)
#else
#define INIFADDR_LOCKINIT()
#define INIFADDR_WRITE_LOCK()
#define INIFADDR_READ_LOCK()
#define INIFADDR_UNLOCK()
#endif
extern	struct domain inetdomain;
extern	CONST struct protosw inetsw[];
extern	int inetprintfs;
extern	int ipforwarding;
extern	CONST	struct sockaddr_in in_zeroaddr;
extern	struct	in_ifaddr *in_ifaddr;
extern	struct	ifqueue	ipintrq;	/* ip packet input queue */
#endif
#ifdef IP_MULTICAST
/*
 * Internet multicast address structure.  There is one of these for each IP
 * multicast group to which this host belongs on a given network interface.
 * They are kept in a linked list, rooted at the interface (struct ifnet)
 * structure.
 */
struct in_multi {
	struct in_addr	  inm_addr;	/* IP multicast address             */
	struct ifnet     *inm_ifp;	/* back pointer to ifnet            */
	u_int		  inm_refcount;	/* no. membership claims by sockets */
	u_int		  inm_timer;	/* IGMP membership report timer     */
	struct in_multi  *inm_next;	/* ptr to next multicast address    */
};

#ifdef _KERNEL
/*
 * Macro for finding the interface (ifnet structure) corresponding to one
 * of our IP addresses.
 */
#define INADDR_TO_IFP(addr, ifp)					  \
	/* struct in_addr addr; */					  \
	/* struct ifnet  *ifp;  */					  \
{									  \
	register struct in_ifaddr *ia;					  \
									  \
	INIFADDR_READ_LOCK();						  \
									  \
	for (ia = in_ifaddr;						  \
	     ia != NULL && IA_SIN(ia)->sin_addr.s_addr != (addr).s_addr;  \
	     ia = ia->ia_next);						  \
	(ifp) = (ia == NULL) ? NULL : ia->ia_ifp;			  \
									  \
	INIFADDR_UNLOCK();   						  \
}

/*
 * Macro for finding the internet address structure (in_ifaddr) corresponding
 * to a given interface (ifnet structure).
 */
#define IFP_TO_IA(ifp, ia)						  \
	/* struct ifnet     *ifp; */					  \
	/* struct in_ifaddr *ia;  */					  \
{									  \
									  \
	INIFADDR_READ_LOCK();						  \
									  \
	for ((ia) = in_ifaddr;						  \
	     (ia) != NULL && (ia)->ia_ifp != (ifp);			  \
	     (ia) = (ia)->ia_next);					  \
									  \
	INIFADDR_UNLOCK();   						  \
}

/*
 * Structure used by macros below to remember position when stepping through
 * all of the in_multi records.
 */
struct in_multistep {
	struct ifnet     *i_if;
	struct ifnet     *i_lastif;
	struct in_multi  *i_inm;
};

/*
 * Macro for looking up the in_multi record for a given IP multicast address
 * on a given interface.  If no matching record is found, "inm" returns NULL.
 */
#if NETSYNC_LOCK
#define IN_LOOKUP_MULTI(addr, ifp, inm)					    \
	/* struct in_addr  addr; */					    \
	/* struct ifnet    *ifp; */					    \
	/* struct in_multi *inm; */					    \
{									    \
            IFMULTI_LOCK(ifp);						    \
	    for ((inm) = ifp->if_multiaddrs;				    \
		 (inm) != NULL && (inm)->inm_addr.s_addr != (addr).s_addr;  \
		 (inm) = inm->inm_next);				    \
	    IFMULTI_UNLOCK(ifp);					    \
}
#else
#define IN_LOOKUP_MULTI(addr, ifp, inm)					    \
	/* struct in_addr  addr; */					    \
	/* struct ifnet    *ifp; */					    \
	/* struct in_multi *inm; */					    \
{									    \
	    for ((inm) = ifp->if_multiaddrs;				    \
		 (inm) != NULL && (inm)->inm_addr.s_addr != (addr).s_addr;  \
		 (inm) = inm->inm_next);				    \
}
#endif

#define IN_LOOKUP_MULTI_NOLOCK(addr, ifp, inm)				    \
	/* struct in_addr  addr; */					    \
	/* struct ifnet    *ifp; */					    \
	/* struct in_multi *inm; */					    \
{									    \
	    for ((inm) = ifp->if_multiaddrs;				    \
		 (inm) != NULL && (inm)->inm_addr.s_addr != (addr).s_addr;  \
		 (inm) = inm->inm_next);				    \
}


#if NETSYNC_LOCK
#define UNLOCK_LAST_MULTI(step)						\
       if((step).i_lastif) IFMULTI_UNLOCK_RECURSIVE((step).i_lastif)
#else
#define UNLOCK_LAST_MULTI(step)						
#endif

/*
 * Macro to step through all of the in_multi records, one at a time.
 * The current position is remembered in "step", which the caller must
 * provide.  IN_FIRST_MULTI(), below, must be called to initialize "step"
 * and get the first record.  Both macros return a NULL "inm" when there
 * are no remaining records.
 */
#define IN_NEXT_MULTI(step, inm)					\
	/* struct in_multistep  step; */				\
	/* struct in_multi     *inm;  */				\
{									\
	if (((inm) = (step).i_inm) != NULL) {				\
		(step).i_inm = (inm)->inm_next;				\
	}								\
	else while ((step).i_if != NULL) {				\
                UNLOCK_LAST_MULTI(step);				\
		IFMULTI_LOCK_RECURSIVE((step).i_if);			\
		(inm) = (step).i_if->if_multiaddrs;			\
		(step).i_lastif = (step).i_if;				\
		(step).i_if = (step).i_if->if_next;			\
		if ((inm) != NULL) {					\
			(step).i_inm = (inm)->inm_next;			\
			break;						\
		}							\
	}								\
}


#define IN_FIRST_MULTI(step, inm)					\
	/* struct in_multistep  step; */				\
	/* struct in_multi     *inm;  */				\
{									\
        (step).i_if  = ifnet;		                                \
	(step).i_inm = NULL;						\
	(step).i_lastif = NULL;						\
	IN_NEXT_MULTI((step), (inm));					\
}

struct in_multi *in_addmulti();
#endif /* _KERNEL */
#endif /* IP_MULTICAST */
