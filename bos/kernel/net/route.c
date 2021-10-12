static char sccsid[] = "@(#)91	1.15.1.11  src/bos/kernel/net/route.c, sysnet, bos411, 9428A410j 5/16/94 16:03:35";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: SA
 *		ifa_ifwithroute
 *		rt_maskedcopy
 *		rtalloc
 *		rtalloc1
 *		rtfree
 *		rtinit
 *		rtinithead
 *		rtioctl
 *		rtredirect
 *		rtrequest
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
 *	Base:	route.c	7.12 (Berkeley) 5/4/89
 *	Merged:	route.c	7.17 (Berkeley) 6/28/90
 *	Merged:	route.c	7.22 (Berkeley) 6/27/91
 */
 
#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/ioctl.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"
#include "net/raw_cb.h"

#include "net/net_malloc.h"

#ifdef	_AIX
#define	min	MIN
#endif 	/* _AIX */

LOCK_ASSERTL_DECL

#define	SA(p)	((struct sockaddr *)(p))

int	rttrash;		/* routes not in table but not freed */

struct	radix_node_head *rnheads[AF_MAX];
static	void (*setroutemask[AF_MAX])(struct sockaddr *, struct sockaddr *);

struct	route_cb route_cb;
struct	rtstat	rtstat;
#if	NETSYNC_LOCK
simple_lock_data_t	route_lock;
#endif

void
rtinithead(family, count, fcn)
	void (*fcn)(struct sockaddr *, struct sockaddr *);
{
	register struct radix_node_head *rnh;
	ROUTE_LOCK_DECL()

	if ((unsigned)family >= AF_MAX)
		panic("rtinithead");
	ROUTE_WRITE_LOCK();
	for (rnh = radix_node_head; rnh && (family != rnh->rnh_af); )
		rnh = rnh->rnh_next;
	if (rnh == 0) {
		rn_inithead(&rnheads[family], count, family);
		setroutemask[family] = fcn;
	}
	ROUTE_WRITE_UNLOCK();
}

/*
 * Packet routing routines.
 */
void
rtalloc(ro)
	register struct route *ro;
{
	ROUTE_LOCK_DECL()

	ROUTE_WRITE_LOCK();
	rtalloc_nolock(ro);
	ROUTE_WRITE_UNLOCK();
}

void
rtalloc_nolock(ro)
	register struct route *ro;
{
	LOCK_ASSERT("rtalloc_nolock", ROUTE_ISLOCKED());
	if (ro->ro_rt && ro->ro_rt->rt_ifp && (ro->ro_rt->rt_flags & RTF_UP))
		return;				 /* XXX */
	ro->ro_rt = rtalloc1_nolock(&ro->ro_dst, 1);
}

struct rtentry *
rtalloc1(dst, report)
	register struct sockaddr *dst;
	int  report;
{
	struct rtentry *newrt = 0;
	ROUTE_LOCK_DECL()

	ROUTE_WRITE_LOCK();
	newrt = rtalloc1_nolock(dst, report);
	ROUTE_WRITE_UNLOCK();
	return (newrt);
}

struct rtentry *
rtalloc1_nolock(dst, report)
	register struct sockaddr *dst;
	int  report;
{
	register struct radix_node_head *rnh;
	register struct rtentry *rt;
	register struct radix_node *rn;
	struct rtentry *newrt = 0;
	int err = 0, msgtype = RTM_MISS;
	NETSTAT_LOCK_DECL()

	LOCK_ASSERT("rtalloc1_nolock", ROUTE_ISLOCKED());

	for (rnh = radix_node_head; rnh && (dst->sa_family != rnh->rnh_af); )
		rnh = rnh->rnh_next;
	if (rnh && rnh->rnh_treetop &&
	    (rn = rn_match((caddr_t)dst, rnh->rnh_treetop)) &&
	    ((rn->rn_flags & RNF_ROOT) == 0)) {
		newrt = rt = (struct rtentry *)rn;
		if (report && (rt->rt_flags & RTF_CLONING)) {
			if ((err = rtrequest_nolock(RTM_RESOLVE, dst, SA(0),
					      SA(0), 0, &newrt)) ||
			    ((rt->rt_flags & RTF_XRESOLVE)
			      && (msgtype = RTM_RESOLVE)))	/* intended! */
			    goto miss;
#ifdef	_AIX_FULLOSF
			if (err = rtrequest(RTM_RESOLVE, dst, SA(0),
						SA(0), 0, &newrt))
				goto miss;
#endif	/* _AIX_FULLOSF */
		} else
			rt->rt_refcnt++;
	} else {
		NETSTAT_LOCK(&rtstat.rts_lock);
		rtstat.rts_unreach++;
		NETSTAT_UNLOCK(&rtstat.rts_lock);
	miss:	if (report)
			rt_missmsg(msgtype, dst, SA(0), SA(0), SA(0), 0, err);
	}
	return (newrt);
}

void
rtfree(rt)
	register struct rtentry *rt;
{
	ROUTE_LOCK_DECL()

	ROUTE_WRITE_LOCK();
	rtfree_nolock(rt);
	ROUTE_WRITE_UNLOCK();
}

void
rtfree_nolock(rt)
	register struct rtentry *rt;
{
	IFAREFCNT_LOCK_DECL()

	LOCK_ASSERT("rtfree_nolock", ROUTE_ISLOCKED());
	if (rt == 0)
		panic("rtfree");
	rt->rt_refcnt--;
	if (rt->rt_refcnt <= 0 && (rt->rt_flags & RTF_UP) == 0) {
		rttrash--;
		if (rt->rt_nodes->rn_flags & (RNF_ACTIVE | RNF_ROOT))
			panic ("rtfree 2");
		IFAREFCNT_LOCK()
		IFAFREE(rt->rt_ifa);
		IFAREFCNT_UNLOCK()
		NET_FREE(rt, M_RTABLE);
	}
}

void
ifafree(ifa)
	register struct ifaddr *ifa;
{
	if (ifa == NULL)
		panic("ifafree");
	if (ifa->ifa_refcnt == 0)
		NET_FREE(ifa, M_IFADDR);
	else
		ifa->ifa_refcnt--;
}

/*
 * Force a routing table entry to the specified
 * destination to go through the given gateway.
 * Normally called as a result of a routing redirect
 * message from the network layer.
 */
void
rtredirect(dst, gateway, netmask, flags, src, rtp)
	struct sockaddr *dst, *gateway, *netmask, *src;
	int flags;
	struct rtentry **rtp;
{
	register struct rtentry *rt = 0;
	int error = 0;
	short *stat = 0;
	struct ifaddr *ifa;
	ROUTE_LOCK_DECL()
	NETSTAT_LOCK_DECL()

	ROUTE_WRITE_LOCK();
	/* verify the gateway is directly reachable */
	ifa = ifa_ifwithnet(gateway);
	if (ifa == 0) {
		error = ENETUNREACH;
		goto done;
	}
	rt = rtalloc1_nolock(dst, 0);
	/*
	 * If the redirect isn't from our current router for this dst,
	 * it's either old or wrong.  If it redirects us to ourselves,
	 * we have a routing loop, perhaps as a result of an interface
	 * going down recently.
	 * 10/12/93: skip "confirm redirect is from current router" 
	 *           if using own interface as gateway (for proxy ARP).
	 */
#define	equal(a1, a2) (bcmp((caddr_t)(a1), (caddr_t)(a2), (a1)->sa_len) == 0)
	if (!(flags & RTF_DONE) && rt &&
	    ((!equal(rt->rt_gateway, rt->rt_ifa->ifa_addr) &&
	      !equal(src, rt->rt_gateway)) || rt->rt_ifa != ifa))
		error = EINVAL;
	else if (ifa_ifwithaddr(gateway))
		error = EHOSTUNREACH;
	if (error)
		goto done;
	/*
	 * Create a new entry if we just got back a wildcard entry
	 * or the the lookup failed.  This is necessary for hosts
	 * which use routing redirects generated by smart gateways
	 * to dynamically build the routing tables.
	 */
	if ((rt == 0) || (rt_mask(rt) && rt_mask(rt)->sa_len < 2))
		goto create;
	/*
	 * Don't listen to the redirect if it's
	 * for a route to an interface. 
	 */
	if (rt->rt_flags & RTF_GATEWAY) {
		if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
			/*
			 * Changing from route to net => route to host.
			 * Create new route, rather than smashing route to net.
			 */
		create:
			flags |=  RTF_GATEWAY | RTF_DYNAMIC;
			error = rtrequest_nolock((int)RTM_ADD, dst, gateway,
#ifdef	_AIX
				    netmask, flags, (struct rtentry **)0);
#else	/* _AIX */
				    SA(0), flags, (struct rtentry **)0);
#endif	/* _AIX */
			stat = &rtstat.rts_dynamic;
		} else {
			/*
			 * Smash the current notion of the gateway to
			 * this destination.  Should check about netmask!!!
			 */
			if (gateway->sa_len <= rt->rt_gateway->sa_len) {
				Bcopy(gateway, rt->rt_gateway, gateway->sa_len);
				rt->rt_flags |= RTF_MODIFIED;
				flags |= RTF_MODIFIED;
				stat = &rtstat.rts_newgateway;
			} else
				error = ENOSPC;
		}
	} else
		error = EHOSTUNREACH;
done:
	if (rt) {
		if (rtp && !error)
			*rtp = rt;
		else
			rtfree_nolock(rt);
	}
	ROUTE_WRITE_UNLOCK();
	NETSTAT_LOCK(&rtstat.rts_lock);
	if (error)
		rtstat.rts_badredirect++;
	else
		(stat && (*stat)++);
	NETSTAT_UNLOCK(&rtstat.rts_lock);
	rt_missmsg(RTM_REDIRECT, dst, gateway, netmask, src, flags, error);
}

/*
* Routing table ioctl interface.
*/
rtioctl(so, req, data)
	struct socket *so;
	int req;
	caddr_t data;
{
#ifndef COMPAT_43
	return (EOPNOTSUPP);
#else
	register struct ortentry *entry = (struct ortentry *)data;
	int error;
	struct sockaddr *netmask = 0, protomask;

	if (req == SIOCADDRT)
		req = RTM_ADD;
	else if (req == SIOCDELRT)
		req = RTM_DELETE;
	else
		return (EINVAL);

	LOCK_ASSERT("rtioctl", SOCKET_ISLOCKED(so));

	if (!(so->so_state & SS_PRIV))
		return (EACCES);
#if BYTE_ORDER != BIG_ENDIAN
	if (entry->rt_dst.sa_family == 0 && entry->rt_dst.sa_len < 16) {
		entry->rt_dst.sa_family = entry->rt_dst.sa_len;
		entry->rt_dst.sa_len = 16;
	}
	if (entry->rt_gateway.sa_family == 0 && entry->rt_gateway.sa_len < 16) {
		entry->rt_gateway.sa_family = entry->rt_gateway.sa_len;
		entry->rt_gateway.sa_len = 16;
	}
#else
	if (entry->rt_dst.sa_len == 0)
		entry->rt_dst.sa_len = 16;
	if (entry->rt_gateway.sa_len == 0)
		entry->rt_gateway.sa_len = 16;
#endif
	if ((entry->rt_flags & RTF_HOST) == 0 &&
	    (unsigned)(entry->rt_dst.sa_family) < AF_MAX &&
	    setroutemask[entry->rt_dst.sa_family]) {
		bzero((caddr_t)&protomask, sizeof protomask);
		(*setroutemask[entry->rt_dst.sa_family])
			(&entry->rt_dst, &protomask);
		netmask = &protomask;
	}
	error = rtrequest(req, &(entry->rt_dst), &(entry->rt_gateway), netmask,
				entry->rt_flags, (struct rtentry **)0);
	rt_missmsg((req == RTM_ADD ? RTM_OLDADD : RTM_OLDDEL),
		   &(entry->rt_dst), &(entry->rt_gateway),
		   netmask, SA(0), entry->rt_flags, error);
	return (error);
#endif
}

struct ifaddr *
ifa_ifwithroute(flags, dst, gateway)
	int	flags;
	struct sockaddr	*dst, *gateway;
{
	register struct ifaddr *ifa;

	if ((flags & RTF_GATEWAY) == 0) {
		/*
		 * If we are adding a route to an interface,
		 * and the interface is a pt to pt link
		 * we should search for the destination
		 * as our clue to the interface.  Otherwise
		 * we can use the local address.
		 */
		ifa = 0;
		if (flags & RTF_HOST) 
			ifa = ifa_ifwithdstaddr(dst);
		if (ifa == 0)
			ifa = ifa_ifwithaddr(gateway);
	} else {
		/*
		 * If we are adding a route to a remote net
		 * or host, the gateway may still be on the
		 * other end of a pt to pt link.
		 */
		ifa = ifa_ifwithdstaddr(gateway);
	}
	if (ifa == 0)
		ifa = ifa_ifwithnet(gateway);
	if (ifa == 0) {
		struct rtentry *rt = rtalloc1_nolock(dst, 0);
		if (rt == 0)
			return (0);
		rt->rt_refcnt--;
		if ((ifa = rt->rt_ifa) == 0)
			return (0);
	}
	if (ifa->ifa_addr->sa_family != dst->sa_family) {
		struct ifaddr *oifa = ifa;
		ifa = ifaof_ifpforaddr(dst, ifa->ifa_ifp);
		if (ifa == 0)
			ifa = oifa;
	}
	return (ifa);
}

rtrequest(req, dst, gateway, netmask, flags, ret_nrt)
	int req, flags;
	struct sockaddr *dst, *gateway, *netmask;
	struct rtentry **ret_nrt;
{
	int error = 0;
	ROUTE_LOCK_DECL()

	ROUTE_WRITE_LOCK();
	error = rtrequest_nolock(req, dst, gateway, netmask, flags, ret_nrt);
	ROUTE_WRITE_UNLOCK();
	return (error);
}

rtrequest_nolock(req, dst, gateway, netmask, flags, ret_nrt)
	int req, flags;
	struct sockaddr *dst, *gateway, *netmask;
	struct rtentry **ret_nrt;
{
	int len, error = 0;
	register struct rtentry *rt;
	register struct radix_node *rn;
	register struct radix_node_head *rnh;
	struct ifaddr *ifa;
	struct sockaddr *ndst;
	u_char af = dst->sa_family;
#ifdef	_AIX
	struct timestruc_t route_age;
#endif	/* _AIX */
#undef	senderr
#define	senderr(x) { error = (x); goto bad; }

	IFAREFCNT_LOCK_DECL()

	LOCK_ASSERT("rtrequest_nolock", ROUTE_ISLOCKED());
	for (rnh = radix_node_head; rnh && (af != rnh->rnh_af); )
		rnh = rnh->rnh_next;
	if (rnh == 0)
		senderr(ESRCH);
	if (flags & RTF_HOST)
		netmask = 0;
	switch (req) {
	case RTM_DELETE:
		if (ret_nrt && (rt = *ret_nrt)) {
			rtfree_nolock(rt);
			*ret_nrt = 0;
		}
		if ((rn = rn_delete((caddr_t)dst, (caddr_t)netmask, 
					rnh->rnh_treetop)) == 0)
			senderr(ESRCH);
		if (rn->rn_flags & (RNF_ACTIVE | RNF_ROOT))
			panic ("rtrequest delete");
		rt = (struct rtentry *)rn;
		rt->rt_flags &= ~RTF_UP;
		if ((ifa = rt->rt_ifa) && ifa->ifa_rtrequest)
			ifa->ifa_rtrequest(RTM_DELETE, rt, SA(0));
		rttrash++;
		if (rt->rt_refcnt <= 0)
			rtfree_nolock(rt);
		break;

	case RTM_RESOLVE:
		if (ret_nrt == 0 || (rt = *ret_nrt) == 0)
			senderr(EINVAL);
		ifa = rt->rt_ifa;
		flags = rt->rt_flags & ~RTF_CLONING;
		gateway = rt->rt_gateway;
		if ((netmask = rt->rt_genmask) == 0)
			flags |= RTF_HOST;
		goto makeroute;

	case RTM_ADD:
		if ((ifa = ifa_ifwithroute(flags, dst, gateway)) == 0)
			senderr(ENETUNREACH);
	makeroute:
		len = sizeof (*rt) + RT_ROUNDUP(gateway) + RT_ROUNDUP(dst);
		R_Malloc(rt, struct rtentry *, len);
		if (rt == 0)
			senderr(ENOBUFS);
		Bzero(rt, len);
		ndst = (struct sockaddr *)(rt + 1);
		if (netmask) {
			rt_maskedcopy(dst, ndst, netmask);
		} else
			Bcopy(dst, ndst, dst->sa_len);
		rn = rn_addroute((caddr_t)ndst, (caddr_t)netmask,
					rnh->rnh_treetop, rt->rt_nodes);
		if (rn == 0) {
			NET_FREE(rt, M_RTABLE);
			senderr(EEXIST);
		}
		IFAREFCNT_LOCK()
		ifa->ifa_refcnt++;
		IFAREFCNT_UNLOCK()
		rt->rt_ifa = ifa;
		rt->rt_ifp = ifa->ifa_ifp;
		rt->rt_flags = RTF_UP | flags;
		rt->rt_gateway = (struct sockaddr *)
					(rn->rn_key + RT_ROUNDUP(dst));
#ifdef	_AIX
		/* SNMP BASE begin*/
		/*
		 * ip_RouteAge is defined to be type int instead of
		 * type timeval in route.h because the reno compatibility.
		 */
		curtime (&route_age);
		rt->ipRouteAge = (int) route_age.tv_sec;
		/* SNMP BASE end*/
#endif	/* _AIX */
		Bcopy(gateway, rt->rt_gateway, gateway->sa_len);
		if (req == RTM_RESOLVE)
			rt->rt_rmx = (*ret_nrt)->rt_rmx; /* copy metrics */
		if (ifa->ifa_rtrequest)
			ifa->ifa_rtrequest(req, rt, SA(ret_nrt ? *ret_nrt : 0));
		if (ret_nrt) {
			*ret_nrt = rt;
			rt->rt_refcnt++;
		}
		break;
	}
bad:
	return (error);
}

void
rt_maskedcopy(src, dst, netmask)
	struct sockaddr *src, *dst, *netmask;
{
	register u_char *cp1 = (u_char *)src;
	register u_char *cp2 = (u_char *)dst;
	register u_char *cp3 = (u_char *)netmask;
	u_char *cplim = cp2 + *cp3;
	u_char *cplim2 = cp2 + *cp1;

#ifdef _AIX
	/* Don't know why this is here, but afraid to remove it! */
	if (!netmask) {
		bcopy(cp1,cp2,sizeof(struct sockaddr));
		return;
	}
#endif /* _AIX */
	*cp2++ = *cp1++; *cp2++ = *cp1++; /* copies sa_len & sa_family */
	cp3 += 2;
	if (cplim > cplim2)
		cplim = cplim2;
	while (cp2 < cplim)
		*cp2++ = *cp1++ & *cp3++;
	if (cp2 < cplim2)
		bzero((caddr_t)cp2, (unsigned)(cplim2 - cp2));
}
/*
 * Set up a routing table entry, normally
 * for an interface.
 */
rtinit(ifa, cmd, flags)
	register struct ifaddr *ifa;
	int cmd, flags;
{
	register struct rtentry *rt;
	register struct sockaddr *dst;
	register struct sockaddr *deldst;
	struct mbuf *m = 0;
	int error;

	IFAREFCNT_LOCK_DECL()

	dst = flags & RTF_HOST ? ifa->ifa_dstaddr : ifa->ifa_addr;
	if (ifa->ifa_flags & IFA_ROUTE) {
		if ((rt = ifa->ifa_rt) && (rt->rt_flags & RTF_UP) == 0) {
			RTFREE(rt);
			ifa->ifa_rt = 0;
		}
	}
	if (cmd == RTM_DELETE) {
		if ((flags & RTF_HOST) == 0 && ifa->ifa_netmask) {
			m = m_get(M_WAIT, MT_SONAME);
			deldst = mtod(m, struct sockaddr *);
			rt_maskedcopy(dst, deldst, ifa->ifa_netmask);
			dst = deldst;
		}
		if (rt = rtalloc1(dst, 0)) {
			rt->rt_refcnt--;
			if (rt->rt_ifa != ifa) {
				if (m)
					(void) m_free(m);
				return (flags & RTF_HOST ? EHOSTUNREACH
							: ENETUNREACH);
			}
		}
	}
	error = rtrequest(cmd, dst, ifa->ifa_addr, ifa->ifa_netmask,
			flags | ifa->ifa_flags, &ifa->ifa_rt);
	if (m)
		(void) m_free(m);
	if (cmd == RTM_ADD && error == 0 && (rt = ifa->ifa_rt)
						&& rt->rt_ifa != ifa) {
		IFAREFCNT_LOCK()
		IFAFREE(rt->rt_ifa);
		rt->rt_ifa = ifa;
		rt->rt_ifp = ifa->ifa_ifp;
		ifa->ifa_refcnt++;
		IFAREFCNT_UNLOCK()
	}
	return (error);
}

#include "net/radix.c"
