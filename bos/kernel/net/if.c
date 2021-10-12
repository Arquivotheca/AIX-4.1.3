static char sccsid[] = "@(#)27	1.46.1.20  src/bos/kernel/net/if.c, sysnet, bos41J, 9518A_all 5/1/95 10:51:55";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: if_attach
 *		if_detach
 *		if_down
 *		if_nostat
 *		if_qflush
 *		if_slowsched
 *		if_slowtimo
 *		ifa_ifwithaddr
 *		ifa_ifwithaf
 *		ifa_ifwithdstaddr
 *		ifa_ifwithnet
 *		ifaof_ifpforaddr
 *		ifconf
 *		ifinit
 *		ifioctl
 *		ifreset
 *		ifubareset
 *		ifunit
 *		link_rtrequest
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
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
 * Copyright (c) 1980, 1986 Regents of the University of California.
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
 *	Base:	if.c	7.8 (Berkeley) 5/5/89
 *	Merged: if.c	7.13 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#ifdef	_AIX_FULLOSF
#include "sys/kernel.h"
#endif	/* _AIX_FULLOSF */
#include "sys/ioctl.h"
#include "sys/errno.h"
#ifndef	_AIX_FULLOSF
#include "sys/timer.h"
#include "sys/intr.h"
#endif	/* _AIX_FULLOSF */

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/if_dl.h"
#include "net/if_types.h"
#include "net/netisr.h"
#include "net/route.h"
#ifdef	_AIX
#include "sys/cdli.h"
#include "net/nd_lan.h"
#endif	/* _AIX */

#include "net/net_malloc.h"

#ifdef IP_MULTICAST
#include <netinet/in_var.h>
#endif

#include <sys/domain.h>

LOCK_ASSERTL_DECL

int	ifqmaxlen = IFQ_MAXLEN;

#ifdef	NETSYNC_LOCK
simple_lock_data_t       ifa_refcnt_lock;
#endif

volatile rt_ifp_del_hit;

/*
 * Network interface utility routines.
 *
 * Routines with ifa_ifwith* names take sockaddr *'s as
 * parameters.
 */

struct	ifnet *ifnet;
extern	struct ifnet loif;
int	if_index, if_indexlim = 8;
struct	ifaddr **ifnet_addrs;

void
ifinit()
{
	register struct ifnet *ifp;

	loinit();		/* Be sure to init loopback */

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_snd.ifq_maxlen == 0)
			ifp->if_snd.ifq_maxlen = ifqmaxlen;
		if (ifp->if_hdrlen > max_linkhdr)
			max_linkhdr = ifp->if_hdrlen;
	}

	(void) netisr_add(NETISR_IFSLOW, if_slowtimo,
				(struct ifqueue *)0, (struct domain *)0);
#ifdef	_AIX_FULLOSF
	if_slowtimo();
#else
	if_slowsched();
#endif	/* _AIX_FULLOSF */
}

#ifdef	vax
void
ifubareset(uban)
{
	ifreset(uban);
}
#endif

/*
 * Call each interface on a bus reset.
 */
void
ifreset(n)
	int n;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_reset)
			(*ifp->if_reset)(ifp->if_unit, n);
}

/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
void
if_attach(ifp)
	struct ifnet *ifp;
{
	unsigned socksize, ifasize;
	int namelen, unitlen;
	char workbuf[16];
	register struct ifnet **p = &ifnet;
	register struct sockaddr_dl *sdl;
	register struct ifaddr *ifa;

	if (ifp != &loif) {
		IFQ_LOCKINIT(&(ifp->if_snd));
#ifdef IP_MULTICAST
                IFMULTI_INITLOCK(ifp);
#endif /* IP_MULTICAST */
		NETSTAT_LOCKINIT(&(ifp->if_slock));
	}
	/*
	 * It is here assumed if_attach is called only during configure().
	 */
	while (*p)
		p = &((*p)->if_next);
	*p = ifp;

	ifp->if_index = ++if_index;
	if (ifnet_addrs == 0 || if_index >= if_indexlim) {
		unsigned n = (if_indexlim <<= 1) * sizeof(ifa);
		struct ifaddr **q;
		NET_MALLOC(q, struct ifaddr **, n, M_IFADDR, M_WAITOK);
		if (ifnet_addrs) {
			bcopy((caddr_t)ifnet_addrs, (caddr_t)q, n/2);
			NET_FREE(ifnet_addrs, M_IFADDR);
		}
		ifnet_addrs = q;
	}

	/*
	 * Only add a LINK addr if one doesn't already exist.
	 */
	for (ifa=ifp->if_addrlist; 
		ifa && ifa->ifa_addr->sa_family != AF_LINK;
		ifa = ifa->ifa_next);

	if (!ifa) {
		/*
		 * create a Link Level name for this device
		 */
		sprintf(workbuf, ifp->if_unit);
		namelen = strlen(ifp->if_name);

		unitlen = strlen(workbuf);
#define _offsetof(t, m) ((int)((caddr_t)&((t *)0)->m))
		socksize = _offsetof(struct sockaddr_dl, sdl_data[0]) +
					unitlen + namelen + ifp->if_addrlen;
		socksize = (socksize + (sizeof(long)-1)) & ~(sizeof(long)-1);
		if (socksize < sizeof(*sdl))
			socksize = sizeof(*sdl);
		ifasize = sizeof(*ifa) + 2 * socksize;
		NET_MALLOC(ifa, struct ifaddr *, ifasize, M_IFADDR, M_WAITOK);
		bzero((caddr_t)ifa, ifasize);
		sdl = (struct sockaddr_dl *)(ifa + 1);
		ifa->ifa_addr = (struct sockaddr *)sdl;
		ifa->ifa_ifp = ifp;
		sdl->sdl_len = socksize;
		sdl->sdl_family = AF_LINK;
		bcopy(ifp->if_name, sdl->sdl_data, namelen);
		bcopy((caddr_t)workbuf, namelen + (caddr_t)sdl->sdl_data, unitlen);
		sdl->sdl_nlen = (namelen += unitlen);
		sdl->sdl_index = ifp->if_index;
		sdl->sdl_type = ifp->if_type;
#ifdef _AIX_FULLOSF
		/*
		 * TODO: generalize this.
		 */
		if (sdl->sdl_type == IFT_ETHER && ifp->if_addrlen == 6) {
			sdl->sdl_alen = ifp->if_addrlen;
			bcopy((caddr_t)(ifp + 1), /* XXX struct arpcom -> ac_enaddr */
				namelen + (caddr_t)sdl->sdl_data,
				sdl->sdl_alen);
		} /* end TODO */
#else
		if (ifp->if_addrlen != 0) {
			sdl->sdl_alen = ifp->if_addrlen;
			/*
			 * copy hwaddr from the arpcom struct.  This bcopy
			 * assumes the hwaddr immediately follows the ifnet
			 * struct in the arpcom struct.
			 */
			bcopy((caddr_t)(ifp + 1), namelen + (caddr_t)sdl->sdl_data,
				sdl->sdl_alen);
		} else
			sdl->sdl_alen = 0;
#endif /* _AIX_FULLOSF */

		sdl = (struct sockaddr_dl *)(socksize + (caddr_t)sdl);
		ifa->ifa_netmask = (struct sockaddr *)sdl;
		sdl->sdl_len = socksize - ifp->if_addrlen;
		while (namelen != 0)
			sdl->sdl_data[--namelen] = 0xff;
		ifa->ifa_next = ifp->if_addrlist;
		ifa->ifa_rtrequest = link_rtrequest;
		ifnet_addrs[ifp->if_index - 1] = ifp->if_addrlist = ifa;
	}
#ifdef	_AIX
	{
	struct timestruc_t ct;

	curtime (&ct);
	ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
	ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
	}
#endif	/* _AIX */
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != addr->sa_family)
			continue;
		if (equal(addr, ifa->ifa_addr))
			return (ifa);
		if ((ifp->if_flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
		    equal(ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}
/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) 
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			if (equal(addr, ifa->ifa_dstaddr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is first found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	if (af == AF_LINK) {
		register struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
		if (sdl->sdl_index && sdl->sdl_index <= if_index)
			return (ifnet_addrs[sdl->sdl_index - 1]);
	}
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		register char *cp, *cp2, *cp3;
		register char *cplim;
		if (ifa->ifa_addr->sa_family != af || ifa->ifa_netmask == 0)
			continue;
		cp = addr->sa_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		for (; cp3 < cplim; cp3++)
			if ((*cp++ ^ *cp2++) & *cp3)
				break;
		if (cp3 == cplim)
			return (ifa);
	    }
	return ((struct ifaddr *)0);
}

/*
 * Find an interface using a specific address family
 */
struct ifaddr *
ifa_ifwithaf(af)
	register int af;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == af)
			return (ifa);
	return ((struct ifaddr *)0);
}

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *
ifaof_ifpforaddr(addr, ifp)
	struct sockaddr *addr;
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register char *cp, *cp2, *cp3;
	register char *cplim;
	struct ifaddr *ifa_maybe = 0;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != af)
			continue;
		ifa_maybe = ifa;
		if (ifa->ifa_netmask == 0) {
			if (equal(addr, ifa->ifa_addr) ||
			    (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return (ifa);
			continue;
		}
		cp = addr->sa_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		for (; cp3 < cplim; cp3++)
			if ((*cp++ ^ *cp2++) & *cp3)
				break;
		if (cp3 == cplim)
			return (ifa);
	}
	return (ifa_maybe);
}

/*
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 */
void
link_rtrequest(cmd, rt, sa)
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct ifaddr *ifa;
	struct sockaddr *dst;
	struct ifnet *ifp, *oldifnet = ifnet;

	IFAREFCNT_LOCK_DECL()
	if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
	    ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
		return;
	if (ifa = ifaof_ifpforaddr(dst, ifp)) {
		IFAREFCNT_LOCK()
		IFAFREE(rt->rt_ifa);
		rt->rt_ifa = ifa;
		ifa->ifa_refcnt++;
		IFAREFCNT_UNLOCK()
		if (ifa->ifa_rtrequest && ifa->ifa_rtrequest != link_rtrequest)
			ifa->ifa_rtrequest(cmd, rt, sa);
	}
}
/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_down(ifp)
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;

	ifp->if_flags &= ~IFF_UP;
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		pfctlinput(PRC_IFDOWN, ifa->ifa_addr);
	if_qflush(&ifp->if_snd);
#ifdef	_AIX
	{
	struct timestruc_t ct;

	curtime (&ct);
	ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
	ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
	}
#endif	/* _AIX */
}

/*
 * Flush an interface queue.
 */
void
if_qflush(ifq)
	register struct ifqueue *ifq;
{
	register struct mbuf *m, *n;
	IFQ_LOCK_DECL()	

	IFQ_LOCK(ifq);
	n = ifq->ifq_head;
	while (m = n) {
		n = m->m_act;
		m_freem(m);
	}
	ifq->ifq_head = 0;
	ifq->ifq_tail = 0;
	ifq->ifq_len = 0;
	IFQ_UNLOCK(ifq);
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
void
if_slowtimo()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp->if_unit);
	}
#ifdef	_AIX_FULLOSF
	timeout(netisr_timeout, (caddr_t)NETISR_IFSLOW, hz/IFNET_SLOWHZ);
#endif	/* _AIX_FULLOSF */
}

#ifndef	_AIX_FULLOSF
if_slowsched()
{
	int ticks;
	extern int if_slowsched();
	static int first = 1;
	static struct trb *iftime;

	if (first) {
		first = 0;	
		/* Allocate timer */
		iftime = talloc();
		iftime->func = (void *)if_slowsched; 
		iftime->func_data = (ulong) 0;
		iftime->ipri = INTCLASS2;
	}
	schednetisr(NETISR_IFSLOW);
	iftime->timeout.it_value.tv_sec = IFNET_SLOWHZ;
	iftime->timeout.it_value.tv_nsec = 0;
	iftime->ipri = INTCLASS2;
	tstart(iftime);
}

/*
 * Zero statistical elements of the interface array before
 * attach.
 */
if_nostat(ifp)
	struct ifnet *ifp;
{
	ifp->if_ipackets = 0;
	ifp->if_ierrors = 0;
	ifp->if_opackets = 0;
	ifp->if_oerrors = 0;
	ifp->if_collisions = 0;
	ifp->if_ibytes=0;
	ifp->if_obytes=0;
	ifp->if_imcasts=0;
	ifp->if_omcasts=0;
	ifp->if_noproto=0;
	ifp->if_iqdrops=0;
        ifp->if_snd.ifq_len = 0;
	ifp->if_snd.ifq_drops=0;
}

if_ifp_rtflush(rn, ifp)
	struct radix_node *rn;
	struct ifnet  *ifp;
{
	int	error;
	int	flags;
	struct sockaddr netmask;

	bzero(&netmask, sizeof(netmask));
	for (; rn; rn = rn->rn_dupedkey) {
		struct rtentry *rt = (struct rtentry *)rn;

		if ( (rn->rn_flags & RNF_ROOT) || (rt->rt_ifp != ifp) )
			continue;

		if (rt->rt_flags & RTF_HOST)
			flags = RTF_HOST;
		else
			flags = RTF_GATEWAY;

		if (((struct sockaddr_in *)rt_key(rt))->sin_addr.s_addr != 0)
			netmask = *(rt->rt_ifa->ifa_netmask);

		if (rtrequest_nolock(RTM_DELETE, rt_key(rt), rt->rt_gateway,
		    &netmask, flags, NULL) == 0)
			rt_ifp_del_hit++;
	}
	return (0);
}

/*
 * NAME:	if_rt_delete
 * FUNCTION:	removes routing entries associated with given interface.
 * DESCRIPTION:
 *	called from if_detach().
 * RETURNS:
 *	nothing.
 */
if_rt_delete(ifp)
	struct ifnet *ifp;
{
	struct ifaddr *ifa;
	struct radix_node_head *rnh;
	int i;
	ROUTE_LOCK_DECL()

	ROUTE_WRITE_LOCK();
        for (rnh = radix_node_head; rnh; rnh = rnh->rnh_next) {
                if (rnh->rnh_af != AF_INET)
                        continue;

		for (i = 0; i < 256; i++) {
			rt_ifp_del_hit = 0;
			rt_walk(rnh->rnh_treetop, if_ifp_rtflush, ifp);
			if (!rt_ifp_del_hit)
				break;
		}
        }
        ROUTE_WRITE_UNLOCK();

}

/*
 * Detach an interface from the
 * list of "active" interfaces.
 */
if_detach(ifp)
register struct ifnet 	*ifp;
{
	register struct ifnet 	*p;
	register struct ifnet 	*prevp;
	struct mbuf		*m;
	struct ifaddr		*ifa;
	struct domain		*dp;
	struct protosw CONST 	*pr;
	int			i;
	struct in_aliasreq 	ifra;
	IFQ_LOCK_DECL()

	pfctlinput(PRC_IFDETACH, (struct sockaddr *)ifp);
	if_down(ifp);

	for (p = ifnet; p && p != ifp; p = p->if_next)
		prevp = p;
	if (p == ifp) {
		if (p == ifnet)
		    ifnet = NULL;
		else
		    prevp->if_next = p->if_next;
		ifp->if_next = NULL;
	} else
		return(ENOENT);

	IF_DEQUEUE(&ifp->if_snd, m);
	while(m) {
		m_freem(m);
		IF_DROP(&ifp->if_snd);
		IF_DEQUEUE(&ifp->if_snd, m);
	}

	if_rt_delete(ifp);
	bzero(&ifra, sizeof(ifra));
	sprintf(ifra.ifra_name, "%s%d", ifp->if_name, ifp->if_unit);
	ifra.ifra_addr.sin_family = AF_INET;
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		struct socket *so;

                if (ifa->ifa_addr->sa_family != AF_INET)
                        continue;
                ifra.ifra_addr.sin_addr.s_addr = 
		    ((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr;
		if (socreate(AF_INET, &so, SOCK_DGRAM, 0) == 0) {
			SOCKET_LOCK(so);
			ifioctl(so, SIOCDIFADDR, &ifra);
			sofree(so);
			SOCKET_UNLOCK(so);
		}
        }


	if (ifp->if_ioctl)
		(void) (*ifp->if_ioctl)(ifp, SIOCIFDETACH, 0);

	return(0);
}
#endif	/* _AIX_FULLOSF */

/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *
ifunit(name)
	register char *name;
{
	register char *cp;
	register struct ifnet *ifp;
	int unit;
	unsigned len;
	char *ep, c;

	for (cp = name; cp < name + IFNAMSIZ && *cp; cp++)
		if (*cp >= '0' && *cp <= '9')
			break;
	if (*cp == '\0' || cp == name + IFNAMSIZ)
		return ((struct ifnet *)0);
	/*
	 * Save first char of unit, and pointer to it,
	 * so we can put a null there to avoid matching
	 * initial substrings of interface names.
	 */
	len = cp - name + 1;
	c = *cp;
	ep = cp;
	for (unit = 0; *cp >= '0' && *cp <= '9'; )
		unit = unit * 10 + *cp++ - '0';
	*ep = 0;
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (bcmp(ifp->if_name, name, len))
			continue;
		if (unit == ifp->if_unit)
			break;
	}
	*ep = c;
	return (ifp);
}

/*
 * Interface ioctls.
 */
ifioctl(so, cmd, data)
	struct socket *so;
	int cmd;
	caddr_t data;
{
	register struct ifnet *ifp;
	register struct ifreq *ifr;
	int error;

	LOCK_ASSERT("ifioctl", SOCKET_ISLOCKED(so));

	switch (cmd) {

	case SIOCGIFCONF:
#ifdef	OSIOCGIFCONF
	case OSIOCGIFCONF:
#endif
#ifdef	CSIOCGIFCONF
	case CSIOCGIFCONF:
#endif
		return (ifconf(cmd, data));

	case SIOCSARP:
	case SIOCDARP:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		/* FALL THROUGH */
	case SIOCGARP:
#ifdef	OSIOCGARP
	case OSIOCGARP:
#endif
#ifndef	_AIX
		return (arpioctl(cmd, data));
#else	/* _AIX */
                {
                int             af;
                struct arpreq   *ar;

                ar = (struct arpreq *)data;
                af = ar->arp_pa.sa_family;

                if ( (af > AF_MAX) || (af_table[af].config.ioctl == NULL) )
                        return(EINVAL);

                return( (*(af_table[af].config.ioctl))(cmd, data) );
                }
#endif	/* _AIX */
	}
	ifr = (struct ifreq *)data;
	ifp = ifunit(ifr->ifr_name);
	if (ifp == 0)
		return (ENXIO);
	switch (cmd) {

	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCSIFFLAGS:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		if (ifp->if_flags & IFF_UP && (ifr->ifr_flags & IFF_UP) == 0) {
			if_down(ifp);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		if (ifp->if_ioctl)
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		break;

	case SIOCSIFMETRIC:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		ifp->if_metric = ifr->ifr_metric;
		break;

#ifdef	_AIX
	case SIOCSIFMTU:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		ifp->if_mtu = ifr->ifr_mtu;
		break;

	case SIOCIFATTACH:

		/*
		 * Notify the protocols that an interface is up and 
		 * attached to a NDD...
		 */
		if (!(so->so_state & SS_PRIV))
			return(EACCES);
		(*so->so_proto->pr_ctlinput)(PRC_IFATTACH, ifp, (caddr_t)NULL);
		break;

	case SIOCIFDETACH:
		if (!(so->so_state & SS_PRIV))
			return(EACCES);
		if_detach(ifp);
		break;

	case SIOCGIFMTU:
		ifr->ifr_mtu = ifp->if_mtu;
		break;

	case SIOCSX25XLATE:
	case SIOCDX25XLATE:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		if (ifp->if_ioctl) 
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		break;
#endif	/* _AIX */
#ifdef IP_MULTICAST
	case SIOCADDMULTI:
	case SIOCDELMULTI:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		if (ifp->if_ioctl)
		  return (*ifp->if_ioctl)(ifp, cmd, data);
		else
		  return (EOPNOTSUPP);
#endif IP_MULTICAST

	default:
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
#ifndef COMPAT_43
		return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
			cmd, data, ifp));
#else
	    {
		int ocmd = cmd;

		switch (cmd) {

		case SIOCSIFDSTADDR:
		case SIOCSIFADDR:
		case SIOCSIFBRDADDR:
		case SIOCSIFNETMASK:
#if BYTE_ORDER != BIG_ENDIAN
			if (ifr->ifr_addr.sa_family == 0 &&
			    ifr->ifr_addr.sa_len < 16) {
				ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
				ifr->ifr_addr.sa_len = 16;
			}
#else
			if (ifr->ifr_addr.sa_len == 0)
				ifr->ifr_addr.sa_len = 16;
#endif
			break;

#ifdef	OSIOCGIFADDR
		case OSIOCGIFADDR:
			cmd = SIOCGIFADDR;
			break;

		case OSIOCGIFDSTADDR:
			cmd = SIOCGIFDSTADDR;
			break;

		case OSIOCGIFBRDADDR:
			cmd = SIOCGIFBRDADDR;
			break;

		case OSIOCGIFNETMASK:
			cmd = SIOCGIFNETMASK;
#endif
		}
		error =  ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
		    (struct mbuf *)cmd,(struct mbuf *)data,(struct mbuf *)ifp));
#ifdef	OSIOCGIFADDR
		switch (ocmd) {

		case OSIOCGIFADDR:
		case OSIOCGIFDSTADDR:
		case OSIOCGIFBRDADDR:
		case OSIOCGIFNETMASK:
			((struct osockaddr *)&ifr->ifr_addr)->sa_family =
							ifr->ifr_addr.sa_family;
		}
#endif
		return (error);

	    }
#endif
	}
	return (0);
}

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
ifconf(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet;
	register struct ifaddr *ifa;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;

	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_next) {
		sprintf(ifr.ifr_name, "%s%d", ifp->if_name, ifp->if_unit);
		if ((ifa = ifp->if_addrlist) == 0) {
			bzero((caddr_t)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp, sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		} else 
		    for ( ; space > sizeof (ifr) && ifa; ifa = ifa->ifa_next) {
			register struct sockaddr *sa = ifa->ifa_addr;
#if	defined(COMPAT_43) && defined(OSIOCGIFCONF)
			if (cmd == OSIOCGIFCONF || cmd == CSIOCGIFCONF) {
				struct osockaddr *osa =
					 (struct osockaddr *)&ifr.ifr_addr;
				ifr.ifr_addr = *sa;
				osa->sa_family = sa->sa_family;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				ifr.ifr_addr = *sa;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else {
				space -= sa->sa_len - sizeof(*sa);
				if (space < sizeof (ifr))
					break;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr.ifr_name));
				if (error == 0)
				    error = copyout((caddr_t)sa,
				      (caddr_t)&ifrp->ifr_addr, sa->sa_len);
				ifrp = (struct ifreq *)
					(sa->sa_len + (caddr_t)&ifrp->ifr_addr);
			}
			if (error)
				break;
			space -= sizeof (ifr);
		}
	}
	ifc->ifc_len -= space;
	return (error);
}
