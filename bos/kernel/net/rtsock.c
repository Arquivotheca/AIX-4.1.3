static char sccsid[] = "@(#)00	1.13  src/bos/kernel/net/rtsock.c, sysnet, bos411, 9433A411a 8/12/94 11:00:21";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: kinfo_rtable
 *		m_copyback
 *		route_configure
 *		route_funnel
 *		route_output
 *		route_sanity
 *		route_unfunnel
 *		route_usrreq
 *		rt_dumpentry
 *		rt_missmsg
 *		rt_setmetrics
 *		rt_walk
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
 * Copyright (c) 1988 Regents of the University of California.
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
 *	Base:	rtsock.c	7.3a (Berkeley) 6/25/89
 *	Merged:	rtsock.c	7.12 (Berkeley) 6/28/90
 *	Merged:	rtsock.c	7.18 (Berkeley) 6/27/91
 */


#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#ifndef	_AIX_FULLOSF
#include "net/if.h"
#endif	/* _AIX_FULLOSF */
#include "net/route.h"
#include "net/raw_cb.h"

#ifdef  _AIX
#define min     MIN
#endif  /* _AIX */

LOCK_ASSERTL_DECL

CONST struct sockaddr route_dst = { 2, PF_ROUTE };
CONST struct sockaddr route_src = { 2, PF_ROUTE };

/*ARGSUSED*/
route_usrreq(so, req, m, nam, control)
	register struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register int error = 0;
	register struct rawcb *rp = sotorawcb(so);

	LOCK_ASSERT("route_usrreq", SOCKET_ISLOCKED(so));

	if (req == PRU_ATTACH) {
		NET_MALLOC(rp, struct rawcb *, sizeof(*rp), M_PCB, M_WAITOK);
		bzero((caddr_t)rp, sizeof *rp);
		so->so_pcb = (caddr_t)rp;
	}
	if (req == PRU_DETACH && rp) {
		unsigned af = rp->rcb_proto.sp_protocol;
		if (af < AF_MAX)
			route_cb.count[af]--;
		route_cb.any_count--;
	}
	error = raw_usrreq(so, req, m, nam, control);
	rp = sotorawcb(so);
	if (req == PRU_ATTACH && rp) {
		int af = rp->rcb_proto.sp_protocol;
		if (error) {
			NET_FREE(rp, M_PCB);
			return (error);
		}
		if (af < AF_MAX)
			route_cb.count[af]++;
		rp->rcb_faddr = (struct sockaddr *)&route_src;
		route_cb.any_count++;
		soisconnected(so);
		so->so_options |= SO_USELOOPBACK;
	}
	return (error);
}

/*ARGSUSED*/
route_output(m, so)
	register struct mbuf *m;
	struct socket *so;
{
	register struct rt_msghdr *rtm = 0;
	register struct rtentry *rt = 0;
	struct rtentry *saved_nrt = 0;
	struct sockaddr *dst = 0, *gate = 0, *netmask = 0, *genmask = 0;
	struct sockaddr *ifpaddr = 0, *ifaaddr = 0;
	struct sockproto route_proto;
	caddr_t cp, lim;
	int len, error = 0;
	struct ifnet *ifp = 0;
	struct ifaddr *ifa = 0;
	RAW_LOCK_DECL()
	IFAREFCNT_LOCK_DECL()
#undef	senderr
#define	senderr(e) { error = (e); goto flush; }

	LOCK_ASSERT("route_output so", SOCKET_ISLOCKED(so));


	if (m == 0 || m->m_len < sizeof(long))
		return (ENOBUFS);
	if ((m = m_pullup(m, sizeof(long))) == 0)
		return (ENOBUFS);
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("route_output");
	len = m->m_pkthdr.len;
	if (len < sizeof(*rtm) ||
	    len != mtod(m, struct rt_msghdr *)->rtm_msglen)
		senderr(EINVAL);
	R_Malloc(rtm, struct rt_msghdr *, len);
	if (rtm == 0)
		senderr(ENOBUFS);
	m_copydata(m, 0, len, (caddr_t)rtm);
	if (rtm->rtm_version != RTM_VERSION)
		senderr(EPROTONOSUPPORT);
	rtm->rtm_pid = so->so_pgid;
	lim = len + (caddr_t) rtm;
	cp = (caddr_t) (rtm + 1);
	if (rtm->rtm_addrs & RTA_DST) {
		dst = (struct sockaddr *)cp;
		cp += RT_ROUNDUP(dst);
	} else
		senderr(EINVAL);
	if ((rtm->rtm_addrs & RTA_GATEWAY) && cp < lim)  {
		gate = (struct sockaddr *)cp;
		cp += RT_ROUNDUP(gate);
	}
	if ((rtm->rtm_addrs & RTA_NETMASK) && cp < lim)  {
		netmask = (struct sockaddr *)cp;
		cp += RT_ROUNDUP(netmask);
	}
	if ((rtm->rtm_addrs & RTA_GENMASK) && cp < lim)  {
		struct radix_node *t;
		genmask = (struct sockaddr *)cp;
		cp += RT_ROUNDUP(genmask);
		t = rn_addmask((caddr_t)genmask, 1, 2);
		if (t && Bcmp(genmask, t->rn_key, *(u_char *)genmask) == 0)
			genmask = (struct sockaddr *)(t->rn_key);
		else
			senderr(ENOBUFS);
	}
	if ((rtm->rtm_addrs & RTA_IFP) && cp < lim)  {
		ifpaddr = (struct sockaddr *)cp;
		cp += RT_ROUNDUP(ifpaddr);
	}
	if ((rtm->rtm_addrs & RTA_IFA) && cp < lim)  {
		ifaaddr = (struct sockaddr *)cp;
	}

	switch (rtm->rtm_type) {
	case RTM_ADD:
		if (gate == 0)
			senderr(EINVAL);
		error = rtrequest(RTM_ADD, dst, gate, netmask,
					rtm->rtm_flags, &saved_nrt);
		if (error == 0 && saved_nrt) {
			rt_setmetrics(rtm->rtm_inits,
				&rtm->rtm_rmx, &saved_nrt->rt_rmx);
			saved_nrt->rt_refcnt--;
			saved_nrt->rt_genmask = genmask;
		}
		break;

	case RTM_DELETE:
		error = rtrequest(RTM_DELETE, dst, gate, netmask,
				rtm->rtm_flags, (struct rtentry **)0);
		break;

	case RTM_GET:
	case RTM_CHANGE:
	case RTM_LOCK:
		rt = rtalloc1(dst, 0);
		if (rt == 0)
			senderr(ESRCH);
		if (rtm->rtm_type != RTM_GET) {
			if (Bcmp(dst, rt_key(rt), dst->sa_len) != 0)
				senderr(ESRCH);
			if (rt->rt_nodes->rn_dupedkey &&
			    (netmask == 0 ||
			     Bcmp(netmask, rt_mask(rt), netmask->sa_len)))
				senderr(ETOOMANYREFS);
		}
		switch(rtm->rtm_type) {

		case RTM_GET:
			dst = rt_key(rt); len = sizeof(*rtm);
			len += RT_ROUNDUP(dst);
			rtm->rtm_addrs |= RTA_DST;
			if (gate = rt->rt_gateway) {
				len += RT_ROUNDUP(gate);
				rtm->rtm_addrs |= RTA_GATEWAY;
			} else
				rtm->rtm_addrs &= ~RTA_GATEWAY;
			if (netmask = rt_mask(rt)) {
				len += RT_ROUNDUP(netmask);
				rtm->rtm_addrs |= RTA_NETMASK;
			} else
				rtm->rtm_addrs &= ~RTA_NETMASK;
			if (genmask = rt->rt_genmask) {
				len += RT_ROUNDUP(genmask);
				rtm->rtm_addrs |= RTA_GENMASK;
			} else
				rtm->rtm_addrs &= ~RTA_GENMASK;
			if (rtm->rtm_addrs & (RTA_IFP | RTA_IFA)) {
				if (rt->rt_ifp == 0)
					goto badif;
				for (ifa = rt->rt_ifp->if_addrlist;
				     ifa && ifa->ifa_addr->sa_family != AF_LINK;
				     ifa = ifa->ifa_next)
					;
				if (ifa && rt->rt_ifa) {
					ifpaddr = ifa->ifa_addr;
					len += RT_ROUNDUP(ifpaddr);
					ifaaddr = rt->rt_ifa->ifa_addr;
					len += RT_ROUNDUP(ifaaddr);
					rtm->rtm_addrs |= RTA_IFP | RTA_IFA;
				} else {
				badif:	ifpaddr = 0;
					rtm->rtm_addrs &= ~(RTA_IFP | RTA_IFA);
				}
			}
			if (len > rtm->rtm_msglen) {
				struct rt_msghdr *new_rtm;
				R_Malloc(new_rtm, struct rt_msghdr *, len);
				if (new_rtm == 0)
					senderr(ENOBUFS);
				Bcopy(rtm, new_rtm, rtm->rtm_msglen);
				Free(rtm); rtm = new_rtm;
			}
			rtm->rtm_msglen = len;
			rtm->rtm_flags = rt->rt_flags;
			rtm->rtm_rmx = rt->rt_rmx;
			cp = (caddr_t) (1 + rtm);
			len = RT_ROUNDUP(dst); 
			Bcopy(dst, cp, len); cp += len;
			if (gate) {
			    len = RT_ROUNDUP(gate);
			    Bcopy(gate, cp, len); cp += len;
			}
			if (netmask) {
			    len = RT_ROUNDUP(netmask);
			    Bcopy(netmask, cp, len); cp += len;
			}
			if (genmask) {
			    len = RT_ROUNDUP(genmask);
			    Bcopy(genmask, cp, len); cp += len;
			}
			if (ifpaddr) {
			    len = RT_ROUNDUP(ifpaddr);
			    Bcopy(ifpaddr, cp, len); cp += len;
			    len = RT_ROUNDUP(ifaaddr);
			    Bcopy(ifaaddr, cp, len); cp += len;
			}
			break;

		case RTM_CHANGE:
			if (gate &&
			    gate->sa_len > (len = rt->rt_gateway->sa_len))
				senderr(EDQUOT);
			/* new gateway could require new ifaddr, ifp;
			   flags may also be different; ifp may be specified
			   by ll sockaddr when protocol address is ambiguous */
			if (ifpaddr && (ifa = ifa_ifwithnet(ifpaddr)) &&
			    (ifp = ifa->ifa_ifp))
				ifa = ifaof_ifpforaddr(ifaaddr ? ifaaddr : gate,
							ifp);
			else if ((ifaaddr && (ifa = ifa_ifwithaddr(ifaaddr))) ||
				 (ifa = ifa_ifwithroute(rt->rt_flags,
							rt_key(rt), gate)))
				ifp = ifa->ifa_ifp;
			if (ifa) {
				register struct ifaddr *oifa = rt->rt_ifa;
				if (oifa != ifa) {
					if (oifa && oifa->ifa_rtrequest)
					      (*oifa->ifa_rtrequest)(RTM_DELETE,
							rt, gate);
					IFAREFCNT_LOCK()
					IFAFREE(rt->rt_ifa);
					rt->rt_ifa = ifa;
					ifa->ifa_refcnt++;
					IFAREFCNT_UNLOCK()
					rt->rt_ifp = ifp;
				}
			}
			if (gate)
				Bcopy(gate, rt->rt_gateway, len);
			rt_setmetrics(rtm->rtm_inits, &rtm->rtm_rmx,
					&rt->rt_rmx);
			if (rt->rt_ifa && rt->rt_ifa->ifa_rtrequest)
				rt->rt_ifa->ifa_rtrequest(RTM_ADD, rt, gate);
			if (genmask)
				rt->rt_genmask = genmask;
			/*
			 * Fall into
			 */
		case RTM_LOCK:
			rt->rt_rmx.rmx_locks |=
				(rtm->rtm_inits & rtm->rtm_rmx.rmx_locks);
			rt->rt_rmx.rmx_locks &= ~(rtm->rtm_inits);
			break;
		}
		break;

	default:
		senderr(EOPNOTSUPP);
	}

flush:
	if (rtm) {
		if (error)
			rtm->rtm_errno = error;
		else 
			rtm->rtm_flags |= RTF_DONE;
	}
	if (rt)
		rtfree(rt);
    {
	register struct rawcb *rp = 0;
	/*
	 * Check to see if we don't want our own messages.
	 */
	if ((so->so_options & SO_USELOOPBACK) == 0) {
		if (route_cb.any_count <= 1) {
			if (rtm)
				 Free(rtm);
			m_freem(m);
			return (error);
		}
		/* There is another listener, so construct message */
		rp = sotorawcb(so);
	}
	if (rtm) {
		m_copyback(m, 0, rtm->rtm_msglen, (caddr_t)rtm);
		Free(rtm);
	}
/* tmt - Evaluate rp handling here... */
	if (rp)
		route_proto.sp_family = 0; /* Avoid us */
	else
		route_proto.sp_family = PF_ROUTE;
	if (dst)
		route_proto.sp_protocol = dst->sa_family;
	else
		route_proto.sp_protocol = AF_UNSPEC;
	SOCKET_UNLOCK(so);		/* avoid deadlock */
	(void) raw_input(m, &route_proto,
		(struct sockaddr *)&route_src, (struct sockaddr *)&route_dst);
	SOCKET_LOCK(so);
    }
	return (error);
}

void
rt_setmetrics(which, in, out)
	u_long which;
	register struct rt_metrics *in, *out;
{
#define metric(f, e) if (which & (f)) out->e = in->e;
	metric(RTV_RPIPE, rmx_recvpipe);
	metric(RTV_SPIPE, rmx_sendpipe);
	metric(RTV_SSTHRESH, rmx_ssthresh);
	metric(RTV_RTT, rmx_rtt);
	metric(RTV_RTTVAR, rmx_rttvar);
	metric(RTV_HOPCOUNT, rmx_hopcount);
	metric(RTV_MTU, rmx_mtu);
	metric(RTV_EXPIRE, rmx_expire);
#undef metric
}

/*
 * Copy data from a buffer back into the indicated mbuf chain,
 * starting "off" bytes from the beginning, extending the mbuf
 * chain if necessary.
 */
void
m_copyback(m0, off, len, cp)
	struct	mbuf *m0;
	register int off;
	register int len;
	caddr_t cp;

{
	register int mlen;
	register struct mbuf *m = m0, *n;
	int totlen = 0;

	if (m0 == 0)
		return;
	while (off > (mlen = m->m_len)) {
		off -= mlen;
		totlen += mlen;
		if (m->m_next == 0) {
			n = m_getclr(M_DONTWAIT, m->m_type);
			if (n == 0)
				goto out;
			n->m_len = min(MLEN, len + off);
			m->m_next = n;
		}
		m = m->m_next;
	}
	while (len > 0) {
		mlen = min (m->m_len - off, len);
		bcopy(cp, off + mtod(m, caddr_t), (unsigned)mlen);
		cp += mlen;
		len -= mlen;
		mlen += off;
		off = 0;
		totlen += mlen;
		if (len == 0)
			break;
		if (m->m_next == 0) {
			n = m_get(M_DONTWAIT, m->m_type);
			if (n == 0)
				break;
			n->m_len = min(MLEN, len);
			m->m_next = n;
		}
		m = m->m_next;
	}
out:	if (((m = m0)->m_flags & M_PKTHDR) && (m->m_pkthdr.len < totlen))
		m->m_pkthdr.len = totlen;
}

/* 
 * The miss message and losing message are very similar.
 */

void
rt_missmsg(type, dst, gate, mask, src, flags, error)
	register struct sockaddr *dst;
	struct sockaddr *gate, *mask, *src;
{
	register struct rt_msghdr *rtm;
	register struct mbuf *m;
	struct sockproto route_proto;
	int dlen = RT_ROUNDUP(dst);
	int len = dlen + sizeof(*rtm);

	if (route_cb.any_count == 0)
		return;
	m = m_gethdr(M_DONTWAIT, MT_DATA);
	if (m == 0)
		return;
	m->m_pkthdr.len = m->m_len = min(len, MHLEN);
	m->m_pkthdr.rcvif = 0;
	rtm = mtod(m, struct rt_msghdr *);
	bzero((caddr_t)rtm, sizeof(*rtm)); /*XXX assumes sizeof(*rtm) < MHLEN*/
	rtm->rtm_flags = RTF_DONE | flags;
	rtm->rtm_msglen = len;
	rtm->rtm_version = RTM_VERSION;
	rtm->rtm_type = type;
	rtm->rtm_addrs = RTA_DST;
	if (type == RTM_OLDADD || type == RTM_OLDDEL) {
		rtm->rtm_pid = getpid();
	}
	m_copyback(m, sizeof (*rtm), dlen, (caddr_t)dst);
	if (gate) {
		dlen = RT_ROUNDUP(gate);
		m_copyback(m, len ,  dlen, (caddr_t)gate);
		len += dlen;
		rtm->rtm_addrs |= RTA_GATEWAY;
	}
	if (mask) {
		dlen = RT_ROUNDUP(mask);
		m_copyback(m, len ,  dlen, (caddr_t)mask);
		len += dlen;
		rtm->rtm_addrs |= RTA_NETMASK;
	}
	if (src) {
		dlen = RT_ROUNDUP(src);
		m_copyback(m, len ,  dlen, (caddr_t)src);
		len += dlen;
		rtm->rtm_addrs |= RTA_AUTHOR;
	}
	if (m->m_pkthdr.len != len) {
		m_freem(m);
		return;
	}
	rtm->rtm_errno = error;
	rtm->rtm_msglen = len;
	route_proto.sp_family = PF_ROUTE;
	route_proto.sp_protocol = dst->sa_family;
	(void) raw_input(m, &route_proto,
		(struct sockaddr *)&route_src, (struct sockaddr *)&route_dst);
}


#ifdef	_AIX_FULLOSF
#include "kinfo.h"
#else	/* _AIX_FULLOSF */
#include "sys/kinfo.h"
#endif	/* _AIX_FULLOSF */

struct walkarg {
	int	w_op, w_arg;
	int	w_given, w_needed;
	caddr_t	w_where;
	struct	{
		struct rt_msghdr m_rtm;
		char	m_sabuf[128];
	} w_m;
#define w_rtm w_m.m_rtm
};

/*
 * This is used in dumping the kernel table via getkinfo().
 */
rt_dumpentry(rn, w)
	struct radix_node *rn;
	register struct walkarg *w;
{
	register struct sockaddr *sa;
	int n, error;

    for (; rn; rn = rn->rn_dupedkey) {
	int count = 0, size = sizeof(w->w_rtm);
	register struct rtentry *rt = (struct rtentry *)rn;

	if (rn->rn_flags & RNF_ROOT)
		continue;
	if (w->w_op == KINFO_RT_FLAGS && !(rt->rt_flags & w->w_arg))
		continue;
#define next(a, l) {size += (l); w->w_rtm.rtm_addrs |= (a); }
	w->w_rtm.rtm_addrs = 0;
	if (sa = rt_key(rt))
		next(RTA_DST, RT_ROUNDUP(sa));
	if (sa = rt->rt_gateway)
		next(RTA_GATEWAY, RT_ROUNDUP(sa));
	if (sa = rt_mask(rt))
		next(RTA_NETMASK, RT_ROUNDUP(sa));
	if (sa = rt->rt_genmask)
		next(RTA_GENMASK, RT_ROUNDUP(sa));
	w->w_needed += size;
	if (w->w_where == NULL || w->w_needed > 0)
		continue;
	w->w_rtm.rtm_msglen = size;
	w->w_rtm.rtm_flags = rt->rt_flags;
	w->w_rtm.rtm_use = rt->rt_use;
	w->w_rtm.rtm_rmx = rt->rt_rmx;
	w->w_rtm.rtm_index = rt->rt_ifp->if_index;
#undef next
#define next(l) {n = (l); Bcopy(sa, cp, n); cp += n;}
	if (size <= sizeof(w->w_m)) {
		register caddr_t cp = (caddr_t)(w->w_m.m_sabuf);
		if (sa = rt_key(rt))
			next(RT_ROUNDUP(sa));
		if (sa = rt->rt_gateway)
			next(RT_ROUNDUP(sa));
		if (sa = rt_mask(rt))
			next(RT_ROUNDUP(sa));
		if (sa = rt->rt_genmask)
			next(RT_ROUNDUP(sa));
#undef next
#define next(s, l) {n = (l); \
    if (error = copyout((caddr_t)(s), w->w_where, n)) return (error); \
    w->w_where += n;}

		next(&w->w_m, size); /* Copy rtmsg and sockaddrs back */
		continue;
	}
	next(&w->w_rtm, sizeof(w->w_rtm));
	if (sa = rt_key(rt))
		next(sa, RT_ROUNDUP(sa));
	if (sa = rt->rt_gateway)
		next(sa, RT_ROUNDUP(sa));
	if (sa = rt_mask(rt))
		next(sa, RT_ROUNDUP(sa));
	if (sa = rt->rt_genmask)
		next(sa, RT_ROUNDUP(sa));
    }
	return (0);
#undef next
}

kinfo_rtable(op, where, given, arg, needed)
	int	op, arg;
	caddr_t	where;
	int	*given, *needed;
{
	register struct radix_node_head *rnh;
	int	error = 0;
	u_char  af = ki_af(op);
	struct	walkarg w;
	ROUTE_LOCK_DECL()

	op &= 0xffff;
	if (op != KINFO_RT_DUMP && op != KINFO_RT_FLAGS)
		return (EINVAL);

	Bzero(&w, sizeof(w));
	if ((w.w_where = where) && given)
		w.w_given = *given;
	w.w_needed = 0 - w.w_given;
	w.w_arg = arg;
	w.w_op = op;
	w.w_rtm.rtm_version = RTM_VERSION;
	w.w_rtm.rtm_type = RTM_GET;

	ROUTE_READ_LOCK();
	for (rnh = radix_node_head; rnh; rnh = rnh->rnh_next) {
		if (rnh->rnh_af == 0)
			continue;
		if (af && af != rnh->rnh_af)
			continue;
		error = rt_walk(rnh->rnh_treetop, rt_dumpentry, &w);
		if (error)
			break;
	}
	ROUTE_READ_UNLOCK();
	w.w_needed += w.w_given;
	if (where && given)
		*given = w.w_where - where;
	else
		w.w_needed = (11 * w.w_needed) / 10;
	*needed = w.w_needed;
	return (error);
}

rt_walk(rn, f, w)
	register struct radix_node *rn;
	register int (*f)();
	struct walkarg *w;
{
	int error;
	for (;;) {
		while (rn->rn_b >= 0)
			rn = rn->rn_l;	/* First time through node, go left */
		if (error = (*f)(rn, w))
			return (error);	/* Process Leaf */
		while (rn->rn_p->rn_r == rn) {	/* if coming back from right */
			rn = rn->rn_p;		/* go back up */
			if (rn->rn_flags & RNF_ROOT)
				return 0;
		}
		rn = rn->rn_p->rn_r;		/* otherwise, go right*/
	}
}


#if	MACH && NETSYNC_SPL
#include "kern/parallel.h"
#endif

/*
 * Definitions of protocols supported in the ROUTE domain.
 */

extern	struct domain routedomain;		/* or at least forward */

route_configure()
{
	return domain_add(&routedomain);
}

#if	NETSYNC_SPL
static void
route_sanity()
{
	panic("route unfunnel");
}

static void
route_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = route_sanity;
	NETSPLX(dfp->object.spl);
	unix_release();
}

static void
route_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("route funnel");
	dfp->unfunnel = route_unfunnel;
	unix_master();
	NETSPL(dfp->object.spl,net);
}
#else
#define route_funnel	0
#endif

CONST struct protosw routesw[] = {
{ SOCK_RAW,	&routedomain,	0,		PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  0,		 route_output,	raw_ctlinput,	0,
  route_usrreq,	0,		0,
  raw_init,	0,		0,		0
}
};

struct domain routedomain =
    { PF_ROUTE, "route", 0, 0, 0,
      routesw, &routesw[sizeof(routesw)/sizeof(routesw[0])],
      0, 0, route_funnel, 0 };
