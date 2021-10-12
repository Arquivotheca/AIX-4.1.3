static char sccsid[] = "@(#)34	1.4.1.2  src/bos/kernext/xns/ns_pcb.c, sysxxns, bos411, 9428A410j 1/19/94 15:43:16";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: ns_pcballoc
 *		ns_pcbbind
 *		ns_pcbconnect
 *		ns_pcbdetach
 *		ns_pcbdisconnect
 *		ns_pcblookup
 *		ns_pcbnotify
 *		ns_rtchange
 *		ns_setpeeraddr
 *		ns_setsockaddr
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
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *      Base:   ns_pcb.c        7.11 (Berkeley) 6/27/91
 */

#include "net/net_globals.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>
#include <net/nh.h>

#include <netns/ns.h>
#include <netns/ns_if.h>
#include <netns/ns_pcb.h>

struct	ns_addr zerons_addr;

ns_pcballoc(so, head)
	struct socket *so;
	struct nspcb *head;
{
	struct mbuf *m;
	register struct nspcb *nsp;

	m = m_getclr(M_DONTWAIT, MT_PCB);
	if (m == NULL)
		return (ENOBUFS);
	nsp = mtod(m, struct nspcb *);
	nsp->nsp_socket = so;
	insque(nsp, head);
	so->so_pcb = (caddr_t)nsp;
	return (0);
}
	
ns_pcbbind(nsp, nam)
	register struct nspcb *nsp;
	struct mbuf *nam;
{
	register struct sockaddr_ns *sns;
	u_short lport = 0;

	if (nsp->nsp_lport || !ns_nullhost(nsp->nsp_laddr))
		return (EINVAL);
	if (nam == 0)
		goto noname;
	sns = mtod(nam, struct sockaddr_ns *);
	if (nam->m_len != sizeof (*sns))
		return (EINVAL);
	if (!ns_nullhost(sns->sns_addr)) {
		int tport = sns->sns_port;

		sns->sns_port = 0;		/* yech... */
		if (ifa_ifwithaddr((struct sockaddr *)sns) == 0)
			return (EADDRNOTAVAIL);
		sns->sns_port = tport;
	}
	lport = sns->sns_port;
	if (lport) {
		u_short aport = ntohs(lport);

		if (aport < NSPORT_RESERVED &&
		    !(nsp->nsp_socket->so_state & SS_PRIV))
			return (EACCES);
		if (ns_pcblookup(&zerons_addr, lport, 0))
			return (EADDRINUSE);
	}
	nsp->nsp_laddr = sns->sns_addr;
noname:
	if (lport == 0)
		do {
			if (nspcb.nsp_lport++ < NSPORT_RESERVED)
				nspcb.nsp_lport = NSPORT_RESERVED;
			lport = htons(nspcb.nsp_lport);
		} while (ns_pcblookup(&zerons_addr, lport, 0));
	nsp->nsp_lport = lport;
	return (0);
}

/*
 * Connect from a socket to a specified address.
 * Both address and port must be specified in argument sns.
 * If don't have a local address for this socket yet,
 * then pick one.
 */
ns_pcbconnect(nsp, nam)
	struct nspcb *nsp;
	struct mbuf *nam;
{
	struct ns_ifaddr *ia;
	register struct sockaddr_ns *sns = mtod(nam, struct sockaddr_ns *);
	register struct ns_addr *dst;
	register struct route *ro;
	struct ifnet *ifp;

	if (nam->m_len != sizeof (*sns))
		return (EINVAL);
	if (sns->sns_family != AF_NS)
		return (EAFNOSUPPORT);
	if (sns->sns_port==0 || ns_nullhost(sns->sns_addr))
		return (EADDRNOTAVAIL);
	/*
	 * If we haven't bound which network number to use as ours,
	 * we will use the number of the outgoing interface.
	 * This depends on having done a routing lookup, which
	 * we will probably have to do anyway, so we might
	 * as well do it now.  On the other hand if we are
	 * sending to multiple destinations we may have already
	 * done the lookup, so see if we can use the route
	 * from before.  In any case, we only
	 * chose a port number once, even if sending to multiple
	 * destinations.
	 */
	ro = &nsp->nsp_route;
	dst = &satons_addr(ro->ro_dst);
        if (nsp->nsp_socket->so_options & SO_DONTROUTE)
                goto flush;
        if (!ns_neteq(nsp->nsp_lastdst, sns->sns_addr))
                goto flush;
        if (!ns_hosteq(nsp->nsp_lastdst, sns->sns_addr)) {
                if (ro->ro_rt && ! (ro->ro_rt->rt_flags & RTF_HOST)) {
                        /* can patch route to avoid rtalloc */
                        *dst = sns->sns_addr;
                } else {
        flush:
                        if (ro->ro_rt)
                                RTFREE(ro->ro_rt);
                        ro->ro_rt = (struct rtentry *)0;
                        nsp->nsp_laddr.x_net = ns_zeronet;
                }
        }/* else cached route is ok; do nothing */
	nsp->nsp_lastdst = sns->sns_addr;
	if ((nsp->nsp_socket->so_options & SO_DONTROUTE) == 0 && /*XXX*/
	    (ro->ro_rt == (struct rtentry *)0 ||
	     ro->ro_rt->rt_ifp == (struct ifnet *)0)) {
		    /* No route yet, so try to acquire one */
		    ro->ro_dst.sa_family = AF_NS;
		    ro->ro_dst.sa_len = sizeof(ro->ro_dst);
		    *dst = sns->sns_addr;
		    dst->x_port = 0;
		    rtalloc(ro);
	}
	if (ns_neteqnn(nsp->nsp_laddr.x_net, ns_zeronet)) {
		/* 
		 * If route is known or can be allocated now,
		 * our src addr is taken from the i/f, else punt.
		 */

		ia = (struct ns_ifaddr *)0;
		/*
		 * If we found a route, use the address
		 * corresponding to the outgoing interface
		 */
		if (ro->ro_rt && (ifp = ro->ro_rt->rt_ifp))
			for (ia = ns_ifaddr; ia; ia = ia->ia_next)
				/* a single hardware intf. can support
					multiple protocol */
				if (ia->ia_ifp == ifp &&
				  ns_neteq(sns->sns_addr,ia->ia_addr.sns_addr))
					break;
		if (ia == 0) {
			u_short fport = sns->sns_addr.x_port;
			sns->sns_addr.x_port = 0;
			ia = (struct ns_ifaddr *)
				ifa_ifwithdstaddr((struct sockaddr *)sns);
			sns->sns_addr.x_port = fport;
			if (ia == 0)
				ia = ns_iaonnetof(&sns->sns_addr);
			if (ia == 0)
				ia = ns_ifaddr;
			if (ia == 0)
				return (EADDRNOTAVAIL);
		}
		nsp->nsp_laddr.x_net = satons_addr(ia->ia_addr).x_net;
	}
	if (ns_pcblookup(&sns->sns_addr, nsp->nsp_lport, 0))
		return (EADDRINUSE);
	if (ns_nullhost(nsp->nsp_laddr)) {
		if (nsp->nsp_lport == 0)
			(void) ns_pcbbind(nsp, (struct mbuf *)0);
		nsp->nsp_laddr.x_host = ns_thishost;
	}
	nsp->nsp_faddr = sns->sns_addr;
	/* Includes nsp->nsp_fport = sns->sns_port; */
	return (0);
}

ns_pcbdisconnect(nsp)
	struct nspcb *nsp;
{

	nsp->nsp_faddr = zerons_addr;
	if (nsp->nsp_socket->so_state & SS_NOFDREF)
		ns_pcbdetach(nsp);
}

ns_pcbdetach(nsp)
	struct nspcb *nsp;
{
	struct socket *so = nsp->nsp_socket;

	so->so_pcb = 0;
	sofree(so);
	if (nsp->nsp_route.ro_rt)
		rtfree(nsp->nsp_route.ro_rt);
	remque(nsp);
	(void) m_free(dtom(nsp));
}

ns_setsockaddr(nsp, nam)
	register struct nspcb *nsp;
	struct mbuf *nam;
{
	register struct sockaddr_ns *sns = mtod(nam, struct sockaddr_ns *);
	
	nam->m_len = sizeof (*sns);
	sns = mtod(nam, struct sockaddr_ns *);
	bzero((caddr_t)sns, sizeof (*sns));
	sns->sns_len = sizeof(*sns);
	sns->sns_family = AF_NS;
	sns->sns_addr = nsp->nsp_laddr;
}

ns_setpeeraddr(nsp, nam)
	register struct nspcb *nsp;
	struct mbuf *nam;
{
	register struct sockaddr_ns *sns = mtod(nam, struct sockaddr_ns *);
	
	nam->m_len = sizeof (*sns);
	sns = mtod(nam, struct sockaddr_ns *);
	bzero((caddr_t)sns, sizeof (*sns));
	sns->sns_len = sizeof(*sns);
	sns->sns_family = AF_NS;
	sns->sns_addr  = nsp->nsp_faddr;
}

/*
 * Pass some notification to all connections of a protocol
 * associated with address dst.  Call the
 * protocol specific routine to handle each connection.
 * Also pass an extra paramter via the nspcb. (which may in fact
 * be a parameter list!)
 */
ns_pcbnotify(dst, errno, notify, param)
	register struct ns_addr *dst;
	int errno;
	void (*notify)(struct nspcb *);
	long param;
{
	register struct nspcb *nsp, *oinp;

	for (nsp = (&nspcb)->nsp_next; nsp != (&nspcb);) {
		if (!ns_hosteq(*dst,nsp->nsp_faddr)) {
	next:
			nsp = nsp->nsp_next;
			continue;
		}
		if (nsp->nsp_socket == 0)
			goto next;
		if (errno) 
			nsp->nsp_socket->so_error = errno;
		oinp = nsp;
		nsp = nsp->nsp_next;
		oinp->nsp_notify_param = param;
		(*notify)(oinp);
	}
}

#ifdef notdef
/*
 * After a routing change, flush old routing
 * and allocate a (hopefully) better one.
 */
ns_rtchange(nsp)
	struct nspcb *nsp;
{
	if (nsp->nsp_route.ro_rt) {
		rtfree(nsp->nsp_route.ro_rt);
		nsp->nsp_route.ro_rt = 0;
		/*
		 * A new route can be allocated the next time
		 * output is attempted.
		 */
	}
	/* SHOULD NOTIFY HIGHER-LEVEL PROTOCOLS */
}
#endif

struct nspcb *
ns_pcblookup(
        struct ns_addr *faddr,
        u_short lport,
        int wildp)
{
	register struct nspcb *nsp, *match = 0;
	int matchwild = 3, wildcard;
	u_short fport;

	fport = faddr->x_port;
	for (nsp = (&nspcb)->nsp_next; nsp != (&nspcb); nsp = nsp->nsp_next) {
		if (nsp->nsp_lport != lport)
			continue;
		wildcard = 0;
		if (ns_nullhost(nsp->nsp_faddr)) {
			if (!ns_nullhost(*faddr))
				wildcard++;
		} else {
			if (ns_nullhost(*faddr))
				wildcard++;
			else {
				if (!ns_hosteq(nsp->nsp_faddr, *faddr))
					continue;
				if (nsp->nsp_fport != fport) {
					if (nsp->nsp_fport != 0)
						continue;
					else
						wildcard++;
				}
			}
		}
		if (wildcard && wildp==0)
			continue;
		if (wildcard < matchwild) {
			match = nsp;
			matchwild = wildcard;
			if (wildcard == 0)
				break;
		}
	}
	return (match);
}
