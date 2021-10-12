static char sccsid[] = "@(#)28	1.7  src/bos/kernext/xns/ns.c, sysxxns, bos411, 9428A410j 3/21/94 15:46:33";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: ns_control
 *		ns_iaonnetof
 *		ns_ifinit
 *		ns_ifscrub
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
 *      Base:   ns.c    7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/user.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>
#include <net/af.h>
#include <net/netisr.h>

#include <netns/ns.h>
#include <netns/ns_if.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <netinet/if_ether.h>

#ifdef NS

struct ns_ifaddr *ns_ifaddr;
int ns_interfaces;
extern struct sockaddr_ns ns_netmask, ns_hostmask;

/*
 * Generic internet control operations (ioctl's).
 */
/* ARGSUSED */
ns_control(so, cmd, data, ifp)
	struct socket *so;
	int cmd;
	caddr_t data;
	register struct ifnet *ifp;
{
	register struct ifreq *ifr = (struct ifreq *)data;
	register struct ns_aliasreq *ifra = (struct ns_aliasreq *)data;
	register struct ns_ifaddr *ia;
	struct ifaddr *ifa;
	struct ns_ifaddr *oia;
	struct mbuf *m;
	int error, dstIsNew, hostIsNew;

	/*
	 * Find address for this interface, if it exists.
	 */
	if (ifp == 0)
		return (EADDRNOTAVAIL);
	for (ia = ns_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == ifp)
			break;

	switch (cmd) {

	case SIOCGIFADDR:
		if (ia == (struct ns_ifaddr *)0)
			return (EADDRNOTAVAIL);
		*(struct sockaddr_ns *)&ifr->ifr_addr = ia->ia_addr;
		return (0);


	case SIOCGIFBRDADDR:
		if (ia == (struct ns_ifaddr *)0)
			return (EADDRNOTAVAIL);
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			return (EINVAL);
		*(struct sockaddr_ns *)&ifr->ifr_dstaddr = ia->ia_broadaddr;
		return (0);

	case SIOCGIFDSTADDR:
		if (ia == (struct ns_ifaddr *)0)
			return (EADDRNOTAVAIL);
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		*(struct sockaddr_ns *)&ifr->ifr_dstaddr = ia->ia_dstaddr;
		return (0);
	}

        if (!(so->so_state & SS_PRIV))
                return (EACCES);

	switch (cmd) {
	case SIOCAIFADDR:
	case SIOCDIFADDR:
		if (ifra->ifra_addr.sns_family == AF_NS)
		    for (oia = ia; ia; ia = ia->ia_next) {
			if (ia->ia_ifp == ifp  &&
			    ns_neteq(ia->ia_addr.sns_addr,
				  ifra->ifra_addr.sns_addr))
			    break;
		    }
		if (cmd == SIOCDIFADDR && ia == 0)
			return (EADDRNOTAVAIL);
		/* FALLTHROUGH */

	case SIOCSIFADDR:
	case SIOCSIFDSTADDR:
		if (ia == (struct ns_ifaddr *)0) {
			m = m_getclr(M_WAIT, MT_IFADDR);
			if (m == (struct mbuf *)NULL)
				return (ENOBUFS);
			if (ia = ns_ifaddr) {
				for ( ; ia->ia_next; ia = ia->ia_next)
					;
				ia->ia_next = mtod(m, struct ns_ifaddr *);
			} else
				ns_ifaddr = mtod(m, struct ns_ifaddr *);
			ia = mtod(m, struct ns_ifaddr *);
			if (ifa = ifp->if_addrlist) {
				for ( ; ifa->ifa_next; ifa = ifa->ifa_next)
					;
				ifa->ifa_next = (struct ifaddr *) ia;
			} else
				ifp->if_addrlist = (struct ifaddr *) ia;
			ia->ia_ifp = ifp;
			ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;

			ia->ia_ifa.ifa_netmask =
				(struct sockaddr *)&ns_netmask;

			ia->ia_ifa.ifa_dstaddr =
				(struct sockaddr *)&ia->ia_dstaddr;
			if (ifp->if_flags & IFF_BROADCAST) {
				ia->ia_broadaddr.sns_family = AF_NS;
				ia->ia_broadaddr.sns_len = sizeof(ia->ia_addr);
				ia->ia_broadaddr.sns_addr.x_host = ns_broadhost;
			}
			ns_interfaces++;
		}
	}

	switch (cmd) {
		int error;

	case SIOCSIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		if (ia->ia_flags & IFA_ROUTE) {
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
			ia->ia_flags &= ~IFA_ROUTE;
		}
		if (ifp->if_ioctl) {
			error = (*ifp->if_ioctl)(ifp, SIOCSIFDSTADDR, 
				(caddr_t) ia);
			if (error)
				return (error);
		}
		*(struct sockaddr *)&ia->ia_dstaddr = ifr->ifr_dstaddr;
		return (0);

	case SIOCSIFADDR:
		return (ns_ifinit(ifp, ia,
				(struct sockaddr_ns *)&ifr->ifr_addr, 1));

	case SIOCDIFADDR:
		ns_ifscrub(ifp, ia);
		if ((ifa = ifp->if_addrlist) == (struct ifaddr *)ia)
			ifp->if_addrlist = ifa->ifa_next;
		else {
			while (ifa->ifa_next &&
			       (ifa->ifa_next != (struct ifaddr *)ia))
				    ifa = ifa->ifa_next;
			if (ifa->ifa_next)
			    ifa->ifa_next = ((struct ifaddr *)ia)->ifa_next;
			else
				printf("Couldn't unlink nsifaddr from ifp\n");
		}
		oia = ia;
		if (oia == (ia = ns_ifaddr)) {
			ns_ifaddr = ia->ia_next;
		} else {
			while (ia->ia_next && (ia->ia_next != oia)) {
				ia = ia->ia_next;
			}
			if (ia->ia_next)
			    ia->ia_next = oia->ia_next;
			else
				printf("Didn't unlink nsifadr from list\n");
		}
		(void) m_free(dtom(oia));
		if (0 == --ns_interfaces) {
			/*
			 * We reset to virginity and start all over again
			 */
			ns_thishost = ns_zerohost;
		}
		return (0);
	
	case SIOCAIFADDR:
		dstIsNew = 0; hostIsNew = 1;
		if (ia->ia_addr.sns_family == AF_NS) {
			if (ifra->ifra_addr.sns_len == 0) {
				ifra->ifra_addr = ia->ia_addr;
				hostIsNew = 0;
			} else if (ns_neteq(ifra->ifra_addr.sns_addr,
					 ia->ia_addr.sns_addr))
				hostIsNew = 0;
		}
		if ((ifp->if_flags & IFF_POINTOPOINT) &&
		    (ifra->ifra_dstaddr.sns_family == AF_NS)) {
			if (hostIsNew == 0)
				ns_ifscrub(ifp, ia);
			ia->ia_dstaddr = ifra->ifra_dstaddr;
			dstIsNew  = 1;
		}
		if (ifra->ifra_addr.sns_family == AF_NS &&
					    (hostIsNew || dstIsNew))
			error = ns_ifinit(ifp, ia, &ifra->ifra_addr, 0);
		return (error);

	default:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		return ((*ifp->if_ioctl)(ifp, cmd, data));
	}
}

/*
* Delete any previous route for an old address.
*/
ns_ifscrub(ifp, ia)
	register struct ifnet *ifp;
	register struct ns_ifaddr *ia; 
{
	if (ia->ia_flags & IFA_ROUTE) {
		if (ifp->if_flags & IFF_POINTOPOINT) {
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
		} else
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, 0);
		ia->ia_flags &= ~IFA_ROUTE;
	}
}
/*
 * Initialize an interface's internet address
 * and routing table entry.
 */
ns_ifinit(ifp, ia, sns, scrub)
	register struct ifnet *ifp;
	register struct ns_ifaddr *ia;
	register struct sockaddr_ns *sns;
{
	struct sockaddr_ns oldaddr;
	register union ns_host *h = &ia->ia_addr.sns_addr.x_host;
	int error;

	/*
	 * Set up new addresses.
	 */
	oldaddr = ia->ia_addr;
	ia->ia_addr = *sns;
	/*
	 * The convention we shall adopt for naming is that
	 * a supplied address of zero means that "we don't care".
	 * if there is a single interface, use the address of that
	 * interface as our 6 byte host address.
	 * if there are multiple interfaces, use any address already
	 * used.
	 *
	 * Give the interface a chance to initialize
	 * if this is its first address,
	 * and to validate the address if necessary.
	 */
	if (ns_hosteqnh(ns_thishost, ns_zerohost)) {
		if (ifp->if_ioctl &&
		     (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR, 
				(caddr_t)ia))) {
			ia->ia_addr = oldaddr;
			return (error);
		}
		ns_thishost = *h;
        } else if (ns_hosteqnh(sns->sns_addr.x_host, ns_zerohost)
            || ns_hosteqnh(sns->sns_addr.x_host, ns_thishost)) {
                *h = ns_thishost;
                if (ifp->if_ioctl &&
                     (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR, (caddr_t)ia))){
                        ia->ia_addr = oldaddr;
                        return (error);
                }
                if (!ns_hosteqnh(ns_thishost,*h)) {
                        ia->ia_addr = oldaddr;
                        return (EINVAL);
                }
        } else {
                ia->ia_addr = oldaddr;
                return (EINVAL);
        }
	/*
	 * Add route for the network.
	 */
	if (scrub) {
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&oldaddr;
		ns_ifscrub(ifp, ia);
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
	}
	if (ifp->if_flags & IFF_POINTOPOINT)
		rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_HOST|RTF_UP);
	else {
		ia->ia_broadaddr.sns_addr.x_net = ia->ia_net;
		rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_UP);
	}
	ia->ia_flags |= IFA_ROUTE;
	return(0);
}

/*
 * Return address info for specified internet network.
 */
struct ns_ifaddr *
ns_iaonnetof(dst)
	register struct ns_addr *dst;
{
	register struct ns_ifaddr *ia;
	register struct ns_addr *compare;
	register struct ifnet *ifp;
	struct ns_ifaddr *ia_maybe = 0;
	union ns_net net = dst->x_net;
	for (ia = ns_ifaddr; ia; ia = ia->ia_next) {
		if (ifp = ia->ia_ifp) {
			if (ifp->if_flags & IFF_POINTOPOINT) {
				compare = &satons_addr(ia->ia_dstaddr);
				if (ns_hosteq(*dst, *compare))
					return (ia);
				if (ns_neteqnn(net, ia->ia_net))
					ia_maybe = ia;
			} else {
				if (ns_neteqnn(net, ia->ia_net))
					return (ia);
			}
		}
	}
	return (ia_maybe);
}

void
ns_ifdetach(ifp)
struct ifnet *ifp;
{
	struct ns_8022	filter;

        if (ifp->if_ioctl) {
		bzero(&filter, sizeof(filter));
		switch (ifp->if_type) {
			default:
				filter.filtertype = NS_8022_LLC_DSAP;
				filter.dsap = DSAP_XNS;
				(*ifp->if_ioctl)(ifp, IFIOCTL_DEL_FILTER, 
					(caddr_t)&filter);
				break;
			case IFT_ETHER:
				filter.ethertype = ETHERTYPE_NS;
				filter.filtertype = NS_ETHERTYPE;
				(*ifp->if_ioctl)(ifp, IFIOCTL_DEL_FILTER, 
					(caddr_t)&filter);
				break;
		}
	}
}

int
ns_ifattach(ifp)
struct ifnet *ifp;
{
	struct 		if_filter f;
	int 		error;

	if (ifp->if_ioctl) {
		bzero(&f, sizeof(f));
		f.user.isr = nsintr;
		f.user.protoq = &nsintrq;
		f.user.netisr = NETISR_NS;
		f.user.pkt_format = NS_PROTO;
		f.user.ifp = ifp;
		switch (ifp->if_type) {
			default:
				f.filter.filtertype = NS_8022_LLC_DSAP;
				f.filter.dsap = DSAP_XNS;
				error = (*ifp->if_ioctl)(ifp, 
					IFIOCTL_ADD_FILTER, (caddr_t)&f);
				break;
			case IFT_ETHER:
				f.filter.filtertype = NS_ETHERTYPE;
				f.filter.ethertype = ETHERTYPE_NS;
        			error = (*ifp->if_ioctl)(ifp, 
					IFIOCTL_ADD_FILTER, (caddr_t)&f);
				break;
		}
	}
}
#endif
