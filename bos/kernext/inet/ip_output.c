static char sccsid[] = "@(#)90	1.27.1.13  src/bos/kernext/inet/ip_output.c, sysxinet, bos41B, 412_41B_sync 12/8/94 16:10:37";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: DEBUGMSG
 *		ip_ctloutput
 *		ip_freemoptions
 *		ip_getmoptions
 *		ip_insertoptions
 *		ip_mloopback
 *		ip_optcopy
 *		ip_output
 *		ip_pcbopts
 *		ip_setmoptions
 *		localaddr_notcast
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
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 *      Base:   ip_output.c     7.16 (Berkeley) 9/20/89
 *      Merged: ip_output.c     7.21 (Berkeley) 6/28/90
 *      Merged: ip_output.c     7.23 (Berkeley) 11/12/90
 */

#include <net/net_globals.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/nettrace.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <net/spl.h>

LOCK_ASSERTL_DECL

/*
 * last_non_broadcast is a NON LOCKED cache of the last ip non-broadcast
 * ipaddr we sent to.  If we hit the cache, we save a pretty expensive set
 * of tests in ip_output to determine if we are sending to a broadcast
 * addr (compare for each IF up).  We don't THINK we need a lock since the
 * worst thing that could happen is we have a cache miss.  This is still 
 * better than a lock, right?
 */
u_long last_non_broadcast = 0;

u_short get_ip_id()
{
	return(fetch_and_add(&ip_id,1));
}

/*
 * IP output.  The packet in mbuf chain m contains a skeletal IP
 * header (with len, off, ttl, proto, tos, src, dst).
 * The mbuf chain containing the packet will be freed.
 * The mbuf opt, if present, will not be freed.
 */
#ifdef IP_MULTICAST
#define DEBUGMSG(x) 
ip_output(m0, opt, ro, flags, mopts)
#else
ip_output(m0, opt, ro, flags)
#endif /* IP_MULTICAST */
	struct mbuf *m0;
	struct mbuf *opt;
	struct route *ro;
#ifdef IP_MULTICAST
        struct mbuf *mopts;
#endif /* IP_MULTICAST */
	int flags;
{
	register struct ip *ip, *mhip;
	register struct ifnet *ifp;
	register struct mbuf *m = m0;
	register int hlen = sizeof (struct ip);
	int len, mhlen, off, error = 0;
	struct route iproute;
	struct sockaddr_in *dst;
	struct in_ifaddr *ia;
	ROUTE_LOCK_DECL()
	NETSTAT_LOCK_DECL()
	IFMULTI_LOCK_DECL()
	INIFADDR_LOCK_DECL()

#ifdef	DIAGNOSTIC
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("ip_output no HDR");
#endif
	if (opt) {
		m0 = m = ip_insertoptions(m, opt, &len);
		hlen = len;
	}
	ip = mtod(m, struct ip *);
	/*
	 * Fill in IP header.
	 */
        if ((flags & (IP_FORWARDING|IP_RAWOUTPUT)) == 0) {
                ip->ip_vhl = (IPVERSION << 4) | (hlen >> 2);
                ip->ip_off &= IP_DF;
                ip->ip_id = htons((fetch_and_add(&ip_id,1)));
                NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_localout++;
                NETSTAT_UNLOCK(&ipstat.ips_lock);
        } else {
                hlen = (ip->ip_vhl & 0x0f) << 2;
        }

	/*
	 * Route packet.
	 */
	if (ro == 0) {
		ro = &iproute;
		bzero((caddr_t)ro, sizeof (*ro));
	}
	dst = (struct sockaddr_in *)&ro->ro_dst;
	/*
	 * If there is a cached route,
	 * check that it is to the same destination
	 * and is still up.  If not, free it and try again.
	 */
	ROUTE_WRITE_LOCK();
	if (ro->ro_rt && ((ro->ro_rt->rt_flags & RTF_UP) == 0 ||
	   dst->sin_addr.s_addr != ip->ip_dst.s_addr)) {
		rtfree_nolock(ro->ro_rt);
		ro->ro_rt = (struct rtentry *)0;
	}
	if (ro->ro_rt == 0) {
		dst->sin_family = AF_INET;
		dst->sin_len = sizeof(*dst);
#ifdef IP_MULTICAST
		if ( IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) &&
		   ((flags & IP_MULTICASTOPTS) && mopts != NULL) ) {
			struct ip_moptions *imo;
			imo = mtod(mopts, struct ip_moptions *);
			if (imo->imo_multicast_ifp != NULL) {
				IFP_TO_IA(imo->imo_multicast_ifp, ia);
				*dst = *(IA_SIN(ia));
			} else 
				dst->sin_addr = ip->ip_dst;

		} else 
#endif /* IP_MULTICAST */
		dst->sin_addr = ip->ip_dst;
	}
	/*
	 * If routing to interface only,
	 * short circuit routing lookup.
	 */
	if (flags & IP_ROUTETOIF) {
		ROUTE_WRITE_UNLOCK();
		ia = (struct in_ifaddr *)ifa_ifwithdstaddr((struct sockaddr *)dst);
		if (ia == 0)
			ia = in_iaonnetof(in_netof(ip->ip_dst));
		if (ia == 0) {
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_cantforward++;		
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			error = ENETUNREACH;
			goto bad;
		}
		ifp = ia->ia_ifp;
	} else {
		if (ro->ro_rt == 0)
			rtalloc_nolock(ro);
		if (ro->ro_rt == 0) {
			ROUTE_WRITE_UNLOCK();
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_cantforward++;		
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			error = EHOSTUNREACH;
			goto bad;
		}
		ifp = ro->ro_rt->rt_ifp;
                /*
                 * If an interface is specified, try to use its ifaddr
                 * as our identity. To emit the packet on the specified
                 * interface requires possible gateway lookup, TBD.
                 */
                if (m->m_pkthdr.rcvif == 0 || m->m_pkthdr.rcvif == ifp ||
                    (ia = ifptoia(m->m_pkthdr.rcvif)) == 0)
                        ia = (struct in_ifaddr *)ro->ro_rt->rt_ifa;

		ro->ro_rt->rt_use++;
		if (ro->ro_rt->rt_flags & RTF_GATEWAY)
			dst = (struct sockaddr_in *)ro->ro_rt->rt_gateway;
		ROUTE_WRITE_UNLOCK();
	}
#ifdef IP_MULTICAST
	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) {
		struct ip_moptions *imo;
		struct in_multi *inm;
#ifdef MROUTE
		extern struct socket *ip_mrouter;
#endif
		/*
		 * IP destination address is multicast.  Make sure "dst"
		 * still points to the address in "ro".  (It may have been
		 * changed to point to a gateway address, above.)
		 */
		dst = (struct sockaddr_in *)&ro->ro_dst;
		/*
		 * See if the caller provided any multicast options
		 */
		if ((flags & IP_MULTICASTOPTS) && mopts != NULL) {
			imo = mtod(mopts, struct ip_moptions *);
			ip->ip_ttl = imo->imo_multicast_ttl;
			if (imo->imo_multicast_ifp != NULL) {
				ifp = imo->imo_multicast_ifp;
				dst->sin_addr = ip->ip_dst;
			}
		}
		else {
			imo = NULL;
			ip->ip_ttl = IP_DEFAULT_MULTICAST_TTL;
		}
		/*
		 * Confirm that the outgoing interface supports multicast.
		 */
		if ((ifp->if_flags & IFF_MULTICAST) == 0) {
			error = ENETUNREACH;
			goto bad;
		}
		/*
		 * If source address not specified yet, use 
		 * FIRST INTERNET address
		 * of outgoing interface.
		 */
		if (ip->ip_src.s_addr == INADDR_ANY) {
			register struct in_ifaddr *ia;

			for (ia = in_ifaddr; ia; ia = ia->ia_next)
				if (ia->ia_ifp == ifp) {
					ip->ip_src = IA_SIN(ia)->sin_addr;
					break;
				}
		}

		DEBUGMSG(("ip_output on %s for %x from %x\n", 
			ifp->if_name, ip->ip_dst.s_addr, ip->ip_src.s_addr));
		if (flags & IP_IFMULTI_NOLOCK) {
			LOCK_ASSERT("ip_output ifmulti", IFMULTI_ISLOCKED(ifp));
			IN_LOOKUP_MULTI_NOLOCK(ip->ip_dst, ifp, inm);
		}
		else
			IN_LOOKUP_MULTI(ip->ip_dst, ifp, inm);
	DEBUGMSG(("ip_output found record %x multicast options %x (%d, %d)\n", 
			inm, imo, (imo)?imo->imo_multicast_loop:-1, 
			(imo)?imo->imo_multicast_ttl:-1));

		m->m_flags |= M_MCAST;

		if (inm != NULL &&
		   (imo == NULL || imo->imo_multicast_loop)) {
			/*
			 * If we belong to the destination multicast group
			 * on the outgoing interface, and the caller did not
			 * forbid loopback, loop back a copy.
			 */
			ip_mloopback(ifp, m, dst);
		}
#ifdef MROUTE
		else if (ip_mrouter && (flags & IP_FORWARDING) == 0) {
			/*
			 * If we are acting as a multicast router, perform
			 * multicast forwarding as if the packet had just
			 * arrived on the interface to which we are about
			 * to send.  The multicast forwarding function
			 * recursively calls this function, using the
			 * IP_FORWARDING flag to prevent infinite recursion.
			 *
			 * Multicasts that are looped back by ip_mloopback(),
			 * above, will be forwarded by the ip_input() routine,
			 * if necessary.
			 */
			if (ip_mforward(ip, ifp) != 0) {
				m_freem(m);
				goto done;
			}
		}
#endif
		/*
		 * Multicasts with a time-to-live of zero may be looped-
		 * back, above, but must not be transmitted on a network.
		 * Also, multicasts addressed to the loopback interface
		 * are not sent -- the above call to ip_mloopback() will
		 * loop back a copy if this host actually belongs to the
		 * destination group on the loopback interface.
		 */
		if (ip->ip_ttl == 0 || ifp == &loif) {
			m_freem(m);
			DEBUGMSG(("ip_output ttl = %d on %s %x\n", ip->ip_ttl, ifp->if_name));
			goto done;
		}
		goto sendit;
	}
#endif /* IP_MULTICAST */
	/*
	 * If source address not specified yet, use address
	 * of outgoing interface.
	 */
	if (ip->ip_src.s_addr == INADDR_ANY)
		ip->ip_src = IA_SIN(ia)->sin_addr;

	/*
	 * When a host sends any datagram, the IP source address must
	 * be one of it's own IP addresses (but not a broadcast or
	 * multicast address).
	 */
	if (rfc1122addrchk && !localaddr_notcast(ip->ip_src.s_addr)) {
			error = EFAULT;
			goto bad;
		}

	/*
	 * Look for broadcast address and
	 * verify user is allowed to send
	 * such a packet.
	 */
	if (last_non_broadcast != dst->sin_addr.s_addr) {
		if (in_broadcast(dst->sin_addr)) {
			if ((ifp->if_flags & IFF_BROADCAST) == 0) {
				error = EADDRNOTAVAIL;
				goto bad;
			}
			if ((flags & IP_ALLOWBROADCAST) == 0) {
				error = EACCES;
				goto bad;
			}
			/* don't allow broadcast messages to be fragmented */
			if ((u_long) ip->ip_len > ifp->if_mtu) {
				error = EMSGSIZE;
				goto bad;
			}
			m->m_flags |= M_BCAST;
		} else
			last_non_broadcast = dst->sin_addr.s_addr;
	}
#ifdef IP_MULTICAST
sendit:
#endif /* IP_MULTICAST */

	/*
	 * If small enough for interface, can just send directly.
	 */
	if ((u_long) ip->ip_len <= ifp->if_mtu) {
		ip->ip_len = htons((u_short)ip->ip_len);
		ip->ip_off = htons((u_short)ip->ip_off);
		ip->ip_sum = 0;
		if (!(ifp->if_flags & IFF_LOOPBACK))
			if (hlen == 20) {
				register int sum = 0;

				sum +=	*(u_short *)((caddr_t)ip);
				sum +=	*(u_short *)((caddr_t)ip + 2);
				sum +=	*(u_short *)((caddr_t)ip + 4);
				sum +=	*(u_short *)((caddr_t)ip + 6);
				sum +=	*(u_short *)((caddr_t)ip + 8);
				sum +=	*(u_short *)((caddr_t)ip + 10);
				sum +=	*(u_short *)((caddr_t)ip + 12);
				sum +=	*(u_short *)((caddr_t)ip + 14);
				sum +=	*(u_short *)((caddr_t)ip + 16);
				sum +=	*(u_short *)((caddr_t)ip + 18);
				sum = (sum & 0xffff) + (sum >> 16);
				sum += sum >> 16;
				ip->ip_sum = (~sum & 0xffff);
			} else
				ip->ip_sum = in_cksum(m, hlen);
		error = (*ifp->if_output)(ifp, m,
				(struct sockaddr *)dst, ro->ro_rt);
		goto done;
	}

	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_fragmented++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	/*
	 * Too large for interface; fragment if possible.
	 * Must be able to put at least 8 bytes per fragment.
	 */
	if (ip->ip_off & IP_DF) {
		NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_cantfrag++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		error = EMSGSIZE;
		goto bad;
	}
	len = ((int) ifp->if_mtu - hlen) &~ 7;
	if (len < 8) {
		NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_cantfrag++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		error = EMSGSIZE;
		goto bad;
	}

	/*
	 * Loop through length of segment, make new header
	 * and copy data of each part, then send.
	 * XXX Be sure original header is not copied by reference.
	 */
	m = m_copym(m0, 0, hlen + len, M_DONTWAIT);
	if (m == 0 || ((m->m_flags & M_EXT) && (m = m_pullup(m, hlen)) == 0)) {
	    NETSTAT_LOCK(&ipstat.ips_lock);
	    ipstat.ips_odropped++;
	    NETSTAT_UNLOCK(&ipstat.ips_lock);
	    error = ENOBUFS;
	    goto bad;
	}
	m0->m_flags &= ~M_PKTHDR;
	mhip = mtod(m, struct ip *);
	mhip->ip_off |= IP_MF;
	off = mhlen = hlen;
	for (;;) {
		m->m_pkthdr.len = mhlen + len;
		m->m_pkthdr.rcvif = (struct ifnet *)0;
		mhip->ip_len = htons((u_short)m->m_pkthdr.len);
		HTONS(mhip->ip_off);
		mhip->ip_sum = 0;
		mhip->ip_sum = in_cksum(m, mhlen);
		if (error = (*ifp->if_output)(ifp, m,
		    (struct sockaddr *)dst, ro->ro_rt))
			break;
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_ofragments++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		if (m0 == NULL || (off += len) >= (int)ip->ip_len)
			break;
		MGETHDR(m, M_DONTWAIT, MT_HEADER);
		if (m == 0) {
		    NETSTAT_LOCK(&ipstat.ips_lock);
		    ipstat.ips_odropped++;
		    NETSTAT_UNLOCK(&ipstat.ips_lock);
		    error = ENOBUFS;
		    break;
		}
		m->m_data += max_linkhdr;
		mhip = mtod(m, struct ip *);
		*mhip = *ip;
		if (hlen > sizeof (struct ip)) {
			mhlen = ip_optcopy(ip, mhip) + sizeof (struct ip);
			mhip->ip_vhl = (IPVERSION << 4) | (mhlen >> 2);
		}
		m->m_len = mhlen;
		mhip->ip_off = ((off - hlen) >> 3) + (ip->ip_off & ~IP_MF);
		if (ip->ip_off & IP_MF)
			mhip->ip_off |= IP_MF;
		if (off + len >= (int)ip->ip_len) {
			len = (int)ip->ip_len - off;
			m_adj(m0, off);
			m->m_next = m0;
			m0 = NULL;
		} else {
			mhip->ip_off |= IP_MF;
			m->m_next = m_copym(m0, off, len, M_DONTWAIT);
			if (m->m_next == 0) {
				m_freem(m);
				NETSTAT_LOCK(&ipstat.ips_lock);
				ipstat.ips_odropped++;
				NETSTAT_UNLOCK(&ipstat.ips_lock);
				error = ENOBUFS;
				break;
			}
		}
	}
bad:
	if (m0)
		m_freem(m0);
done:
	if (ro == &iproute && (flags & IP_ROUTETOIF) == 0 && ro->ro_rt)
		RTFREE(ro->ro_rt);
	return (error);
}

/*
 * Insert IP options into preformed packet.
 * Adjust IP destination as required for IP source routing,
 * as indicated by a non-zero in_addr at the start of the options.
 */
struct mbuf *
ip_insertoptions(m, opt, phlen)
	register struct mbuf *m;
	struct mbuf *opt;
	int *phlen;
{
	register struct ipoption *p = mtod(opt, struct ipoption *);
	struct mbuf *n;
	register struct ip *ip = mtod(m, struct ip *);
	unsigned optlen;

	optlen = opt->m_len - sizeof(p->ipopt_dst);
	if (optlen + (int)ip->ip_len > IP_MAXPACKET)
		return (m);		/* XXX should fail */
	if (p->ipopt_dst.s_addr)
		ip->ip_dst = p->ipopt_dst;
	if (m->m_flags & M_EXT || m->m_data - optlen < m->m_pktdat) {
		MGETHDR(n, M_DONTWAIT, MT_HEADER);
		if (n == 0)
			return (m);
		n->m_pkthdr.len = m->m_pkthdr.len + optlen;
		m->m_len -= sizeof(struct ip);
		m->m_data += sizeof(struct ip);
		n->m_next = m;
		m = n;
		m->m_len = optlen + sizeof(struct ip);
		m->m_data += max_linkhdr;
		bcopy((caddr_t)ip, mtod(m, caddr_t), sizeof(struct ip));
	} else {
		m->m_data -= optlen;
		m->m_len += optlen;
		m->m_pkthdr.len += optlen;
		ovbcopy((caddr_t)ip, mtod(m, caddr_t), sizeof(struct ip));
	}
	ip = mtod(m, struct ip *);
	bcopy((caddr_t)p->ipopt_list, (caddr_t)(ip + 1), (unsigned)optlen);
	*phlen = sizeof(struct ip) + optlen;
	ip->ip_len += optlen;
	return (m);
}

/*
 * Copy options from ip to jp,
 * omitting those not copied during fragmentation.
 */
ip_optcopy(ip, jp)
	struct ip *ip, *jp;
{
	register u_char *cp, *dp;
	int opt, optlen, cnt;

	cp = (u_char *)(ip + 1);
	dp = (u_char *)(jp + 1);
	cnt = ((ip->ip_vhl & 0x0f) << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else
			optlen = cp[IPOPT_OLEN];
		/* bogus lengths should have been caught by ip_dooptions */
		if (optlen > cnt)
			optlen = cnt;
		if (IPOPT_COPIED(opt)) {
			bcopy((caddr_t)cp, (caddr_t)dp, (unsigned)optlen);
			dp += optlen;
		}
	}
	for (optlen = dp - (u_char *)(jp+1); optlen & 0x3; optlen++)
		*dp++ = IPOPT_EOL;
	return (optlen);
}

/*
 * IP socket option processing.
 */
ip_ctloutput(op, so, level, optname, mp)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **mp;
{
	register struct inpcb *inp;
	register struct mbuf *m = *mp;
	register int optval;
	int error = 0;

	inp = sotoinpcb(so);
	if (inp == (struct inpcb *) NULL) {
		return(ENOTCONN);
	}

	LOCK_ASSERT("ip_ctloutput", INPCB_ISLOCKED(inp));
	if (level != IPPROTO_IP)
		error = EINVAL;
	else switch (op) {

	case PRCO_SETOPT:
		switch (optname) {
		case IP_OPTIONS:
		case IP_RETOPTS:
			return(ip_pcbopts(&inp->inp_options, m));

		case IP_TOS:
		case IP_TTL:
		case IP_RECVOPTS:
		case IP_RECVRETOPTS:
		case IP_RECVDSTADDR:
			if (m->m_len != sizeof(int))
				error = EINVAL;
			else {
				optval = *mtod(m, int *);
				switch (optname) {

				case IP_TOS:
					inp->inp_ip.ip_tos = optval;
					break;

				case IP_TTL:
					inp->inp_ip.ip_ttl = optval;
					break;
#define	OPTSET(bit) \
	if (optval) \
		inp->inp_flags |= bit; \
	else \
		inp->inp_flags &= ~bit;

				case IP_RECVOPTS:
					OPTSET(INP_RECVOPTS);
					break;

				case IP_RECVRETOPTS:
					OPTSET(INP_RECVRETOPTS);
					break;

				case IP_RECVDSTADDR:
					OPTSET(INP_RECVDSTADDR);
					break;
				}
			}
			break;
#undef OPTSET
#ifdef IP_MULTICAST
                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_setmoptions(so, optname, 
				&inp->inp_moptions, m);
                        break;
#endif /* IP_MULTICAST */

		default:
			error = EINVAL;
			break;
		}
		if (m)
			(void)m_free(m);
		break;

	case PRCO_GETOPT:
		switch (optname) {
		case IP_OPTIONS:
		case IP_RETOPTS:
			*mp = m = m_get(M_DONTWAIT, MT_SOOPTS);
			if (m == NULL) {
				error = ENOBUFS;
				break;
			}	
			if (inp->inp_options) {
				m->m_len = inp->inp_options->m_len;
				bcopy(mtod(inp->inp_options, caddr_t),
				    mtod(m, caddr_t), (unsigned)m->m_len);
			} else
				m->m_len = 0;
			break;

		case IP_TOS:
		case IP_TTL:
		case IP_RECVOPTS:
		case IP_RECVRETOPTS:
		case IP_RECVDSTADDR:
			*mp = m = m_get(M_WAIT, MT_SOOPTS);
			m->m_len = sizeof(int);
			switch (optname) {

			case IP_TOS:
				optval = inp->inp_ip.ip_tos;
				break;

			case IP_TTL:
				optval = inp->inp_ip.ip_ttl;
				break;

#define	OPTBIT(bit)	(inp->inp_flags & bit ? 1 : 0)

			case IP_RECVOPTS:
				optval = OPTBIT(INP_RECVOPTS);
				break;

			case IP_RECVRETOPTS:
				optval = OPTBIT(INP_RECVRETOPTS);
				break;

			case IP_RECVDSTADDR:
				optval = OPTBIT(INP_RECVDSTADDR);
				break;
			}
			*mtod(m, int *) = optval;
			break;
#ifdef IP_MULTICAST
                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_getmoptions(optname, inp->inp_moptions, mp);
                        break;
#endif /* IP_MULTICAST */

		default:
			error = EINVAL;
			break;
		}
		break;
	}
	return (error);
}

/*
 * Set up IP options in pcb for insertion in output packets.
 * Store in mbuf with pointer in pcbopt, adding pseudo-option
 * with destination address if source routed.
 */
ip_pcbopts(pcbopt, m)
	struct mbuf **pcbopt;
	register struct mbuf *m;
{
	register cnt, optlen;
	register u_char *cp;
	u_char opt;

	/* turn off any old options */
	if (*pcbopt)
		(void)m_free(*pcbopt);
	*pcbopt = 0;
	if (m == (struct mbuf *)0 || m->m_len == 0) {
		/*
		 * Only turning off any previous options.
		 */
		if (m)
			(void)m_free(m);
		return (0);
	}

	/*
	 * IP first-hop destination address will be stored before
	 * actual options; move other options back
	 * and clear it when none present.
	 */
	if (m->m_data + m->m_len + sizeof(struct in_addr) >= &m->m_dat[MLEN])
		goto bad;
	cnt = m->m_len;
	m->m_len += sizeof(struct in_addr);
	cp = mtod(m, u_char *) + sizeof(struct in_addr);
	ovbcopy(mtod(m, caddr_t), (caddr_t)cp, (unsigned)cnt);
	bzero(mtod(m, caddr_t), sizeof(struct in_addr));

	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= IPOPT_OLEN || optlen > cnt)
				goto bad;
		}
		switch (opt) {

		default:
			break;

		case IPOPT_LSRR:
		case IPOPT_SSRR:
			/*
			 * user process specifies route as:
			 *	->A->B->C->D
			 * D must be our final destination (but we can't
			 * check that since we may not have connected yet).
			 * A is first hop destination, which doesn't appear in
			 * actual IP option, but is stored before the options.
			 */
			if (optlen < IPOPT_MINOFF - 1 + sizeof(struct in_addr))
				goto bad;
			m->m_len -= sizeof(struct in_addr);
			cnt -= sizeof(struct in_addr);
			optlen -= sizeof(struct in_addr);
			cp[IPOPT_OLEN] = optlen;
			/*
			 * Move first hop before start of options.
			 */
			bcopy((caddr_t)&cp[IPOPT_OFFSET+1], mtod(m, caddr_t),
			    sizeof(struct in_addr));
			/*
			 * Then copy rest of options back
			 * to close up the deleted entry.
			 */
			ovbcopy((caddr_t)(&cp[IPOPT_OFFSET+1] +
			    sizeof(struct in_addr)),
			    (caddr_t)&cp[IPOPT_OFFSET+1],
			    (unsigned)cnt + sizeof(struct in_addr));
			break;
		}
	}
	if (m->m_len > MAX_IPOPTLEN + sizeof(struct in_addr))
		goto bad;
	*pcbopt = m;
	return (0);

bad:
	(void)m_free(m);
	return (EINVAL);
}

/*
 * Take given IP address and search in_ifaddr to see if we have a match.
 * Also see if given IP address is a broadcast address (multicast not
 * supported yet).  Return 1 if given IP address is a local address
 * and not a broadcast address, return 0 otherwise.
 *
 */

int
localaddr_notcast(in)
	struct in_addr in;
{
        register struct in_ifaddr *ia;
	u_long t;

	/*
	 * Look through the list of addresses for a match
	 * with a broadcast address. If no match, then try
	 * to match given address with local address.
	 */

	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
	    if (ia->ia_ifp->if_flags & IFF_BROADCAST) {
		if (ia->ia_broadaddr.sin_addr.s_addr == in.s_addr)
			return (0);
		/*
		 * Check for old-style (host 0) broadcast.
		 */
		if ((t = ntohl(in.s_addr)) == ia->ia_subnet || t == ia->ia_net)
			return (0);
	    }
	    if (in.s_addr == INADDR_BROADCAST || in.s_addr == INADDR_ANY)
		return (0);
	    if (in.s_addr == ia->ia_addr.sin_addr.s_addr)
		return (1);
	}

	return (0); /* address not our own */
}

#ifdef IP_MULTICAST
/*
 * Set the IP multicast options in response to user setsockopt().
 */
ip_setmoptions(struct socket *so, int optname, 
	struct mbuf **mopts, struct mbuf *m)
{
	int error = 0;
	struct ip_moptions *imo;
	u_char loop;
	int i;
	struct in_addr addr;
	struct ip_mreq *mreq;
	struct ifnet *ifp;
	struct route ro;
	struct sockaddr_in *dst;
	INIFADDR_LOCK_DECL()

 	DEBUGMSG(("ip_setmoptions(%x)\n", optname));
	if (*mopts == NULL) {
		/*
		 * No multicast option buffer attached to the pcb;
		 * allocate one and initialize to default values.
		 */
		MGET(*mopts, M_DONTWAIT, MT_IPMOPTS);
		if (*mopts == NULL)
			return (ENOBUFS);
		imo = mtod(*mopts, struct ip_moptions *);
		imo->imo_multicast_ifp   = NULL;
		imo->imo_multicast_ttl   = IP_DEFAULT_MULTICAST_TTL;
		imo->imo_multicast_loop  = IP_DEFAULT_MULTICAST_LOOP;
		imo->imo_num_memberships = 0;
	} else
	    imo = mtod(*mopts, struct ip_moptions *);

	switch (optname) {

	case IP_MULTICAST_IF:
		/*
		 * Select the interface for outgoing multicast packets.
		 */
		if (m == NULL || m->m_len != sizeof(struct in_addr)) {
			error = EINVAL;
			break;
		}
		addr = *(mtod(m, struct in_addr *));
		/*
		 * INADDR_ANY is used to remove a previous selection.
		 * When no interface is selected, a default one is
		 * chosen every time a multicast packet is sent.
		 */
		if (addr.s_addr == INADDR_ANY) {
			imo->imo_multicast_ifp = NULL;
			break;
		}
		/*
		 * The selected interface is identified by its local
		 * IP address.  Find the interface and confirm that
		 * it supports multicasting.
		 */
		INADDR_TO_IFP(addr, ifp);
		if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {
		  DEBUGMSG(("ip_setmoptions(IP_MULTICAST_IF) no such interface\n", optname));
			error = EADDRNOTAVAIL;
			break;
		}
		imo->imo_multicast_ifp = ifp;
		break;

	case IP_MULTICAST_TTL:
		/*
		 * Set the IP time-to-live for outgoing multicast packets.
		 */
		if (m == NULL || m->m_len != sizeof(u_char)) {
			error = EINVAL;
			break;
		}
		imo->imo_multicast_ttl = *(mtod(m, u_char *));
		break;

	case IP_MULTICAST_LOOP:
		/*
		 * Set the loopback flag for outgoing multicast packets.
		 * Must be zero or one.
		 */
		if (m == NULL || m->m_len != sizeof(u_char) ||
		   (loop = *(mtod(m, u_char *))) > 1) {
			error = EINVAL;
			break;
		}
		imo->imo_multicast_loop = loop;
		break;

	case IP_ADD_MEMBERSHIP:
		/*
		 * Add a multicast group membership.
		 * Group must be a valid IP multicast address.
		 */
		if (m == NULL || m->m_len != sizeof(struct ip_mreq)) {
 		  DEBUGMSG(("ip_setmoptions(IP_ADD_MEMBERSHIP) argument length wrong m = %x m_len = %d reqd %d\n", m, (m) ? m->m_len : 0, sizeof(struct ip_mreq)));
			error = EINVAL;
			break;
		}
		mreq = mtod(m, struct ip_mreq *);
		if (!IN_MULTICAST(ntohl(mreq->imr_multiaddr.s_addr))) {
 		  DEBUGMSG(("ip_setmoptions(IP_ADD_MEMBERSHIP) no multicast address\n"));
			error = EINVAL;
			break;
		}
		/*
		 * If no interface address was provided, use the interface of
		 * the route to the given multicast address.
		 */
		if (mreq->imr_interface.s_addr == INADDR_ANY) {
			ro.ro_rt = NULL;
			dst = (struct sockaddr_in *)&ro.ro_dst;
			dst->sin_family = AF_INET;
			dst->sin_len    = sizeof(*dst);
			dst->sin_addr   = mreq->imr_multiaddr;
			rtalloc(&ro);
			if (ro.ro_rt == NULL) {
 			  DEBUGMSG(("ip_setmoptions(IP_ADD_MEMBERSHIP) no route\n"));
				error = EADDRNOTAVAIL;
				break;
			}
			ifp = ro.ro_rt->rt_ifp;
			rtfree(ro.ro_rt);
		} else {
			INADDR_TO_IFP(mreq->imr_interface, ifp);
		}
		/*
		 * See if we found an interface, and confirm that it
		 * supports multicast.
		 */
		if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {
		  DEBUGMSG(("ip_setmoptions(IP_ADD_MEMBERSHIP) no interface\n"));
			error = EADDRNOTAVAIL;
			break;
		}
		/*
		 * See if the membership already exists or if all the
		 * membership slots are full.
		 */
		for (i = 0; i < imo->imo_num_memberships; ++i) {
			if (imo->imo_membership[i]->inm_ifp == ifp &&
			    imo->imo_membership[i]->inm_addr.s_addr
						== mreq->imr_multiaddr.s_addr)
				break;
		}
		if (i < imo->imo_num_memberships) {
			error = EADDRINUSE;
			break;
		}
		if (i == IP_MAX_MEMBERSHIPS) {
			error = ETOOMANYREFS;
			break;
		}
		/*
		 * Everything looks good; add a new record to the multicast
		 * address list for the given interface.
		 */
		SOCKET_UNLOCK(so);
		if ((imo->imo_membership[i] =
		    in_addmulti(mreq->imr_multiaddr, ifp)) == NULL) {
			error = ENOBUFS;
			break;
		}
		SOCKET_LOCK(so);
		++imo->imo_num_memberships;
		break;

	case IP_DROP_MEMBERSHIP:
		/*
		 * Drop a multicast group membership.
		 * Group must be a valid IP multicast address.
		 */
		if (m == NULL || m->m_len != sizeof(struct ip_mreq)) {
			error = EINVAL;
			break;
		}
		mreq = mtod(m, struct ip_mreq *);
		if (!IN_MULTICAST(ntohl(mreq->imr_multiaddr.s_addr))) {
			error = EINVAL;
			break;
		}
		/*
		 * If an interface address was specified, get a pointer
		 * to its ifnet structure.
		 */
		if (mreq->imr_interface.s_addr == INADDR_ANY)
			ifp = NULL;
		else {
			INADDR_TO_IFP(mreq->imr_interface, ifp);
			if (ifp == NULL) {
				error = EADDRNOTAVAIL;
				break;
			}
		}
		/*
		 * Find the membership in the membership array.
		 */
		for (i = 0; i < imo->imo_num_memberships; ++i) {
			if ((ifp == NULL ||
			     imo->imo_membership[i]->inm_ifp == ifp) &&
			     imo->imo_membership[i]->inm_addr.s_addr
					    == mreq->imr_multiaddr.s_addr)
				break;
		}
		if (i == imo->imo_num_memberships) {
			error = EADDRNOTAVAIL;
			break;
		}
		/*
		 * Give up the multicast address record to which the
		 * membership points.
		 */
		SOCKET_UNLOCK(so);
		in_delmulti(imo->imo_membership[i]);
		SOCKET_LOCK(so);
		/*
		 * Remove the gap in the membership array.
		 */
		for (++i; i < imo->imo_num_memberships; ++i)
			imo->imo_membership[i-1] = imo->imo_membership[i];
		--imo->imo_num_memberships;
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	/*
	 * If all options have default values, no need to keep the mbuf.
	 */
	if (imo->imo_multicast_ifp   == NULL &&
	    imo->imo_multicast_ttl   == IP_DEFAULT_MULTICAST_TTL &&
	    imo->imo_multicast_loop  == IP_DEFAULT_MULTICAST_LOOP &&
	    imo->imo_num_memberships == 0) {
		m_free(*mopts);
		*mopts = NULL;
	}

	DEBUGMSG(("ip_setmoptions(%x) done with return code %d\n", optname, error));
	return error;
}

/*
 * Return the IP multicast options in response to user getsockopt().
 */
ip_getmoptions(int optname, struct mbuf *mopts,	struct mbuf **m)
{
	u_char *ttl;
	u_char *loop;
	struct in_addr *addr;
	struct ip_moptions *imo;
	struct in_ifaddr *ia;
	INIFADDR_LOCK_DECL()

	if(!(*m = m_get(M_WAIT, MT_IPMOPTS))) 
	    return ENOMEM;

	imo = (mopts == NULL) ? NULL : mtod(mopts, struct ip_moptions *);

	switch (optname) {

	case IP_MULTICAST_IF:
		addr = mtod(*m, struct in_addr *);
		(*m)->m_len = sizeof(struct in_addr);
		if (imo == NULL || imo->imo_multicast_ifp == NULL)
			addr->s_addr = INADDR_ANY;
		else {
			IFP_TO_IA(imo->imo_multicast_ifp, ia);
			addr->s_addr = (ia == NULL) ? INADDR_ANY
					: IA_SIN(ia)->sin_addr.s_addr;
		}
		return(0);

	case IP_MULTICAST_TTL:
		ttl = mtod(*m, u_char *);
		(*m)->m_len = sizeof(u_char);
		*ttl = (imo == NULL) ? IP_DEFAULT_MULTICAST_TTL
				     : imo->imo_multicast_ttl;
		return(0);

	case IP_MULTICAST_LOOP:
		loop = mtod(*m, u_char *);
		(*m)->m_len = sizeof(u_char);
		*loop = (imo == NULL) ? IP_DEFAULT_MULTICAST_LOOP
				      : imo->imo_multicast_loop;
		return(0);

	default:
		return(EOPNOTSUPP);
	}
}

/*
 * Discard the IP multicast options.
 */
void ip_freemoptions(struct mbuf *mopts)
{
	struct ip_moptions *imo;
	int i;

	if (mopts != NULL) {
		imo = mtod(mopts, struct ip_moptions *);
		for (i = 0; i < imo->imo_num_memberships; ++i)
			in_delmulti(imo->imo_membership[i]);
		m_free(mopts);
	}
}

/*
 * Routine called from ip_output() to loop back a copy of an IP multicast
 * packet to the input queue of a specified interface.  Note that this
 * calls the output routine of the loopback "driver", but with an interface
 * pointer that might NOT be &loif -- easier than replicating that code here.
 */
void ip_mloopback(struct ifnet *ifp, struct mbuf *m, struct sockaddr_in *dst)
{
	register struct ip *ip=mtod(m, struct ip *);
	struct mbuf *copym;

	DEBUGMSG(("ip_mloopback on %s for %x from %x\n", ifp->if_name, ip->ip_dst.s_addr, ip->ip_src.s_addr));
	copym = m_copy(m, 0, M_COPYALL);
	if (copym != NULL) {
		/*
		 * We don't bother to fragment if the IP length is greater
		 * than the interface's MTU.  Can this possibly matter?
		 */
		ip = mtod(copym, struct ip *);
		ip->ip_len = htons((u_short)ip->ip_len);
		ip->ip_off = htons((u_short)ip->ip_off);
		ip->ip_sum = 0;
	        ip->ip_sum = in_cksum(copym, (ip->ip_vhl&0x0f) << 2);
		(void) looutput(ifp, copym, (struct sockaddr *)dst, NULL);
	} else {
	  DEBUGMSG(("ip_mloopback copy failed\n", m, ifp, ifp->if_name, dst->sin_family, dst->sin_addr.s_addr));
	}
}

#endif

