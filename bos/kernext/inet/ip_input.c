static char sccsid[] = "@(#)89	1.44  src/bos/kernext/inet/ip_input.c, sysxinet, bos411, 9428A410j 6/21/94 10:41:00";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: imin
 *		ip_dooptions
 *		ip_drain
 *		ip_forward
 *		ip_freef
 *		ip_init
 *		ip_reass
 *		ip_rtaddr
 *		ip_slowtimo
 *		ip_srcroute
 *		ip_stripoptions
 *		ipintr
 *		save_rte
 *		setiproutemask
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
 *      Base:   ip_input.c      7.14 (Berkeley) 9/20/89
 *      Merged: ip_input.c      7.16 (Berkeley) 6/28/90
 */

#include <net/net_globals.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/nettrace.h>
#include <sys/syspest.h>
#include <sys/malloc.h>

#include <net/if.h>
#include <net/route.h>
#include <net/netisr.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp.h>
#include <net/spl.h>

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

#ifdef	NSIP
#include <netns/ns.h>
#endif

#ifndef imin
#	define	imin(a, b)	(int) (((int) (a)) < ((int) (b)) ? (a) : (b))
#endif

int	nonlocsrcroute = NONLOCSRCROUTE;
int	nonlocsrcroute_dflt = NONLOCSRCROUTE;
int	rfc1122addrchk = RFC1122ADDRCHK;
int	rfc1122addrchk_dflt = RFC1122ADDRCHK;
int	ipforwarding = IPFORWARDING;
int	ipforwarding_dflt = IPFORWARDING;
int	ipsendredirects = IPSENDREDIRECTS;
int	ipsendredirects_dflt = IPSENDREDIRECTS;
int	directed_broadcast = DIRECTED_BROADCAST;
int	directed_broadcast_dflt = DIRECTED_BROADCAST;


struct ifqueue ipintrq;				/* IP input  queue	*/
int	ipqmaxlen = IFQ_MAXLEN * 2;
int	ipqmaxlen_dflt = IFQ_MAXLEN * 2;
struct	in_ifaddr *in_ifaddr;			/* first inet address */

#if     NETSYNC_LOCK
simple_lock_data_t	ip_frag_lock;
simple_lock_data_t      inifaddr_lock;
simple_lock_data_t      ip_misc_lock;
#endif

/*
 * We need to save the IP options in case a protocol wants to respond
 * to an incoming packet over the same route if the packet got here
 * using IP source routing.  This allows connection establishment and
 * maintenance when the remote end is on a network that is not known
 * to us.
#if     NETISR_THREAD
 * Fix me fix me fix me. ip_stripoptions() is structured to do what
 * we need: extract the options if desired and so they can be passed
 * back down. The global save area is not workable.
#endif
 */
int	ip_nhops = 0;
static	struct ip_srcrt ip_srcrt;

extern	int if_index;
#ifdef IP_IFMATRIX
u_long	*ip_ifmatrix;
#endif

static void
setiproutemask(dst, mask)
        struct sockaddr_in *dst, *mask;
{
        in_sockmaskof(dst->sin_addr, mask);
}

struct ipq ipq;
int     ipfragqmaxlen;                          /* max length */
u_short ip_id;
struct ipstat ipstat;
int ipfragttl = IPFRAGTTL;
int ipfragttl_dflt = IPFRAGTTL;

/*
 * IP initialization: fill in IP protocol switch table.
 * All protocols not implemented in kernel go to raw IP protocol handler.
 */
void
ip_init()
{
	register struct protosw CONST *pr;
	register int i;
	NETSTAT_LOCK_DECL()

	pr = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
	if (pr == 0)
		panic("ip_init");

	for (i = 0; i < IPPROTO_MAX; i++) {
		ip_protox[i].protosw = pr;
		ip_protox[i].protox_flags = 0;
	}

	for (pr = inetdomain.dom_protosw;
	    pr < inetdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_domain->dom_family == PF_INET &&
		    pr->pr_protocol != IPPROTO_RAW)  {
			ip_protox[pr->pr_protocol].protosw = pr;
			ip_protox[pr->pr_protocol].protox_flags = PROTOX_USED;
	}
	ipq.next = ipq.prev = &ipq;
	ip_id    = ntohl(iptime());
        in_ifaddr = NULL;
        in_interfaces = 0;
        INIFADDR_LOCKINIT();
        IFQ_LOCKINIT(&ipintrq);
	ipintrq.ifq_maxlen = ipqmaxlen;
        IPFRAG_LOCKINIT();
        ipfragqmaxlen = 4096;
        NETSTAT_LOCKINIT(&ipstat.ips_lock);
        NETSTAT_LOCKINIT(&icmpstat.icps_lock);
#ifdef IP_IFMATRIX
	/*
	 * this is a problem since we are assuming that the number of
	 * interfaces will never grow.  if two more are attached then
	 * we'll violate the array bounds below.  if this code is to
	 * be used then we'll have to deal with the possibility that
	 * if_index will increase after initialization.
	 */
	i = (if_index + 1) * (if_index + 1) * sizeof (u_long);
	NET_MALLOC(ip_ifmatrix, u_long *, i, M_RTABLE, M_WAITOK);
#endif

        /* SNMP BASE begin */
	NETSTAT_LOCK(&ipstat.ips_lock);
        ipstat.ipInHdrErrors = 0;
        ipstat.ipInAddrErrors = 0;
        ipstat.ipInDiscards = 0;
#ifdef IP_MULTICAST
        ipstat.ipInMAddrErrors = 0;
#endif /* IP_MULTICAST */
	NETSTAT_UNLOCK(&ipstat.ips_lock);
        /* SNMP BASE end */

	rtinithead(AF_INET, 32, setiproutemask);
	(void) netisr_add(NETISR_IP, ipintr, &ipintrq, &inetdomain);

        IFQ_LOCKINIT(&pfctlinputq);
        pfctlinputq.ifq_maxlen = 50;
        (void) netisr_add(NETISR_PFCTLINPUT, dequeue_pfctlinput, 
		&pfctlinputq, 0);

}

#if     !NETISR_THREAD
struct  sockaddr_in ipaddr = { sizeof(ipaddr), AF_INET };
#else
/* Following struct protected by route lock, sort of. */
#endif
struct  route ipforward_rt;
u_long	last_dst = 0;

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 */
void
ipintr()
{
	register struct mbuf *m;
	IFQ_LOCK_DECL()

	/*
	 * Get next datagram off input queue and process.
	 */
	for (;;) {
		IF_DEQUEUE(&ipintrq, m);
		if (m == (struct mbuf *) NULL)
			return;
		ipintr_noqueue(NULL, m);
	}
}

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 */
void
ipintr_noqueue(nddp, m)
        struct ndd *nddp;
        struct mbuf *m;
{
	register struct ip *ip;
	register struct in_ifaddr *ia;
	register u_long sum;
	int hlen, s;
	NETSTAT_LOCK_DECL()
	IFMULTI_LOCK_DECL()

#ifdef	DIAGNOSTIC
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("ipintr no HDR");
#endif
	/*
	 * If no IP addresses have been set yet but the interfaces
	 * are receiving, can't do anything with incoming packets yet.
	 */
	if (in_ifaddr == NULL){
                /* SNMP BASE begin */
		NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ipInDiscards++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
                /* SNMP BASE end */
		goto bad;
	}
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_total++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);

	if (m->m_len < sizeof (struct ip) &&
	    (m = m_pullup(m, sizeof (struct ip))) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_toosmall++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto next;
	}
	ip = mtod(m, struct ip *);
	if ((ip->ip_vhl & 0xf0) != IPVERSION << 4) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ipInAddrErrors++;	/* SNMP BASE */
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}
	hlen = (ip->ip_vhl & 0x0f) << 2;

        if (hlen < sizeof(struct ip)) {
                NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_badhlen++;
                NETSTAT_UNLOCK(&ipstat.ips_lock);
                goto bad;
        }
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, hlen)) == 0) {
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_badhlen++;
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			goto next;
		}
		ip = mtod(m, struct ip *);
	}

        if (m->m_pkthdr.rcvif != &loif) {
                /* Inline in_cksum over IP header. */
#define ADDSUM(n)       case n: sum +=  *(u_short *)((caddr_t)ip + n - 2) + \
                                        *(u_short *)((caddr_t)ip + n - 4)
                sum = 0;
                switch (hlen) {
                        ADDSUM(60); ADDSUM(56); ADDSUM(52); ADDSUM(48);
                        ADDSUM(44); ADDSUM(40); ADDSUM(36); ADDSUM(32);
                        ADDSUM(28); ADDSUM(24); ADDSUM(20); ADDSUM(16);
                        ADDSUM(12); ADDSUM( 8); ADDSUM( 4);
                }
                sum = (sum & 0xffff) + (sum >> 16);
                sum += sum >> 16;
                if (~sum & 0xffff) {
                        NETSTAT_LOCK(&ipstat.ips_lock);
                        ipstat.ips_badsum++;
                        NETSTAT_UNLOCK(&ipstat.ips_lock);
                        goto bad;
                }
        } else
                ip->ip_sum = 0;

        /*
         * Convert fields to host representation.
         * Check length and protect against overflow.
         */
        NTOHS(ip->ip_len);
        NTOHS(ip->ip_off);
        if ((int)ip->ip_len < hlen ||
            ((int)(ip->ip_off & 0x1fff) << 3) +
             (int)ip->ip_len - hlen > IP_MAXPACKET) {
                NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_badlen++;
                NETSTAT_UNLOCK(&ipstat.ips_lock);
                goto bad;
        }
        NTOHS(ip->ip_id);


	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	if (m->m_pkthdr.len < (long)ip->ip_len) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_tooshort++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}
	if (m->m_pkthdr.len > (long)ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else
			m_adj(m, (int)((long)ip->ip_len - m->m_pkthdr.len));
	}

	/*
	 * Process options and, if not destined for us,
	 * ship it on.  ip_dooptions returns 1 when an
	 * error was detected (causing an icmp message
	 * to be sent and the original packet to be freed).
	 */
	ip_nhops = 0;		/* for source routed packets */
	if (hlen > sizeof (struct ip) && ip_dooptions(m))
		goto next;

	if ((ip->ip_dst.s_addr == last_dst)
#ifdef DIRECTED_BROADCAST
		&& !directed_broadcast
#endif
		)
		    goto ours;

	/*
	 * Check our list of addresses, to see if the packet is for us.
	 */
	m->m_flags |= M_WCARD;  /* Accepting a broadcast? */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
#define	satosin(sa)	((struct sockaddr_in *)(sa))

		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr) {
			m->m_flags &= ~M_WCARD; /* A direct address */
			goto ours;
		}
		if (
#ifdef	DIRECTED_BROADCAST
		    /*
		     * If directed_broadcast is true, then the receive-ifp
		     * must match the in_ifaddr ifp to allow these checks
		     * on the broadcast address.
		     */
		    (!directed_broadcast || (ia->ia_ifp == m->m_pkthdr.rcvif))&&
#endif
		    (ia->ia_ifp->if_flags & IFF_BROADCAST)) {
			u_long t;

			if (satosin(&ia->ia_broadaddr)->sin_addr.s_addr ==
			    ip->ip_dst.s_addr)
				goto ours;
			if (ip->ip_dst.s_addr == ia->ia_netbroadcast.s_addr)
				goto ours;
			/*
			 * Look for all-0's host part (old broadcast addr),
			 * either for subnet or net.
			 */
			t = ntohl(ip->ip_dst.s_addr);
			if (t == ia->ia_subnet)
				goto ours;
			if (t == ia->ia_net)
				goto ours;
		}
	}
#ifdef IP_MULTICAST
#define DEBUGMSG(x) 
	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) {
		struct in_multi *inm;
		struct ifnet *ifp = m->m_pkthdr.rcvif;
#ifdef MROUTE
		extern struct socket *ip_mrouter;

		if (ip_mrouter) {
			/*
			 * If we are acting as a multicast router, all
			 * incoming multicast packets are passed to the
			 * kernel-level multicast forwarding function.
			 * The packet is returned (relatively) intact; if
			 * ip_mforward() returns a non-zero value, the packet
			 * must be discarded, else it may be accepted below.
			 *
			 * (The IP ident field is put in the same byte order
			 * as expected when ip_mforward() is called from
			 * ip_output().)
			 */
			ip->ip_id = htons(ip->ip_id);
			if (ip_mforward(ip, ifp) != 0) {
				m_freem(m);
				goto next;
			}
			ip->ip_id = ntohs(ip->ip_id);

			/*
			 * The process-level routing demon needs to receive
			 * all multicast IGMP packets, whether or not this
			 * host belongs to their destination groups.
			 */
			if (ip->ip_p == IPPROTO_IGMP)
				goto ours;
		}
#endif /* MROUTE */
		/*
		 * See if we belong to the destination multicast group on the
		 * arrival interface.
		 */
		DEBUGMSG(("ip_input on %s for %x\n", ifp->if_name, ip->ip_dst.s_addr));
		IN_LOOKUP_MULTI(ip->ip_dst, ifp, inm);
		if (inm == NULL) {
		  DEBUGMSG(("ip_input on %s for %x, no such group\n", ifp->if_name, ip->ip_dst.s_addr));
		  m_freem(m);
		  NETSTAT_LOCK(&ipstat.ips_lock);
		  ipstat.ipInMAddrErrors++;	/* SNMP BASE */
		  NETSTAT_UNLOCK(&ipstat.ips_lock);
		  goto next;
		}
		goto ours;
	}
#endif /* IP_MULTICAST */
	if (ip->ip_dst.s_addr == (u_long)INADDR_BROADCAST)
		goto ours;
	if (ip->ip_dst.s_addr == INADDR_ANY)
		goto ours;

	/*
	 * Not for us; forward if possible and desirable.
	 */
	if (ipforwarding == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantforward++;
		ipstat.ipInAddrErrors++;	/* SNMP BASE */
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		m_freem(m);
	} else {
		m->m_flags &= ~M_WCARD;
		ip_forward(m, 0);
	}
	goto next;

ours:

	/*
	 * Only update the cache with unicast datagrams.
	 */
	if (!IN_MULTICAST(ntohl(ip->ip_dst.s_addr)))
		last_dst = ip->ip_dst.s_addr;

	/*
	 * Adjust ip_len to not reflect header.
	 * If offset or IP_MF are set, must reassemble.
	 * Otherwise, nothing need be done.
	 * (We could look in the reassembly queue to see
	 * if the packet was previously fragmented,
	 * but it's not worth the time; just let them time out.)
	 */
	ip->ip_len -= hlen;
	if (ip->ip_off & (IP_MF | 0x1fff)) {
		/*
		 * Attempt reassembly; if it succeeds, proceed.
		 */
		m = ip_reass(m);

		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_fragments++;
		if (m) ipstat.ips_reassembled++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		if (m == NULL)
			goto next;
		ip = mtod(m, struct ip *);
	}

	/*
	 * Switch out to protocol's input routine.
	 */
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_delivered++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	(*((ip_protox[ip->ip_p].protosw)->pr_input))(m, hlen);
	return;
bad:
	m_freem(m);
next:
	return;
}

/*
 * Take incoming datagram fragment and try to
 * reassemble it into whole datagram.
 */
struct mbuf *
ip_reass(m)
	register struct mbuf *m;
{
	register struct ipq *fp;
	register struct ipasfrag *ip, *q;
	struct ipasfrag *p;
	struct mbuf *t;
	int hlen, i, nfrag;
	NETSTAT_LOCK_DECL()
	IPFRAG_LOCK_DECL()

	hlen = (mtod(m, struct ip *)->ip_vhl & 0x0f) << 2;
	/*
	 * Make room for the additional ipasfrag pointers.
	 * If pullup needed, pull the header exactly, only
	 * the first frag will keep it.
	 * Hide ipasfrag headers from code below.
	 */
	i = (int)&((struct ipasfrag *)NULL)->ipf_ip;
	if (M_LEADINGSPACE(m) >= i) {
		ip = (struct ipasfrag *)(mtod(m, caddr_t) - i);
		m->m_len -= hlen;
		m->m_data += hlen;
	} else {
		t = m_get(M_DONTWAIT, m->m_type);
		if (t == NULL)
			goto dropfrag1;
		M_COPY_PKTHDR(t, m);
		ip = mtod(t, struct ipasfrag *);
		bcopy(mtod(m, caddr_t), (caddr_t)&ip->ipf_ip, hlen);
		m->m_len -= hlen;
		m->m_data += hlen;
		t->m_len = 0;
		t->m_data += hlen + i;
		t->m_next = m;
		m = t;
	}
	ip->ipf_mbuf = m;

	/*
	 * Set ipf_mff if more fragments are expected,
	 * convert offset to bytes and lose flags.
	 */
	ip->ipf_mff = (ip->ipf_off & IP_MF);
	ip->ipf_off <<= 3;

	/*
	 * Look for queue of fragments
	 * of this datagram.
	 */
	IPFRAG_LOCK();
	for (nfrag = 0, fp = ipq.next; fp != &ipq; nfrag++, fp = fp->next)
		if (ip->ipf_id == fp->ipq_id && ip->ipf_p == fp->ipq_p &&
		    ip->ipf_src.s_addr == fp->ipq_src.s_addr &&
		    ip->ipf_dst.s_addr == fp->ipq_dst.s_addr)
			break;

	/*
	 * If first fragment to arrive, create a reassembly queue.
	 * Enforce a limit on queued fragments. Could scan fragq
	 * for sending host but fairness suggests complex algorithm.
	 * For now, just kill the oldest.
	 */
	if (fp == &ipq) {
		NET_MALLOC(fp, struct ipq *, sizeof *fp, M_FTABLE, M_NOWAIT);
		if (fp == NULL)
			goto dropfrag;
		if (nfrag >= ipfragqmaxlen && ipq.next != &ipq) {
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_fragdropped++;
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			ip_freef(ipq.next);
		}
		fp->ipq_ttl = IPFRAGTTL;
		fp->ipq_p = ip->ipf_p;
		fp->ipq_id = ip->ipf_id;
		fp->ipq_src = ip->ipf_src;
		fp->ipq_dst = ip->ipf_dst;
		insque(fp, &ipq);
		fp->ipq_next = fp->ipq_prev = (struct ipasfrag *)&fp->ipq_next;
		q = fp->ipq_next;
	} else {
		/*
		 * Find a segment which begins after this one does.
		 */
		for (q = fp->ipq_next; q->ipf_off <= ip->ipf_off &&
		      (q = q->ipf_next) != (struct ipasfrag *)&fp->ipq_next; )
			;

		/*
		 * If there is a preceding segment, it may provide some of
		 * our data already.  If so, drop the data from the incoming
		 * segment.  If it provides all of our data, drop us.  In
		 * the likely case it's appending the queue, do so now.
		 */
		if (q->ipf_prev != (struct ipasfrag *)&fp->ipq_next) {
			i = q->ipf_prev->ipf_off + q->ipf_prev->ipf_len;
			if (i == (int)ip->ipf_off) {
				if (q == (struct ipasfrag *)&fp->ipq_next) {
					q = q->ipf_prev;
					q->ipf_len += ip->ipf_len;
					q->ipf_mff = ip->ipf_mff;
					m_cat(q->ipf_mbuf, m);
					goto check;
				}
			} else if (i > (int)ip->ipf_off) {
				i -= ip->ipf_off;
				if (i >= (int)ip->ipf_len)
					goto dropfrag;
				m_adj(m, i);
				ip->ipf_off += i;
				ip->ipf_len -= i;
			}
		}

		/*
		 * While we overlap succeeding segments trim them or,
		 * if they are completely covered, dequeue them.
		 */
		while (q != (struct ipasfrag *)&fp->ipq_next) {
			i = ip->ipf_off + ip->ipf_len;
			if (i <= (int)q->ipf_off)
				break;
			i -= q->ipf_off;
			if (i < (int)q->ipf_len) {
				q->ipf_len -= i;
				q->ipf_off += i;
				m_adj(q->ipf_mbuf, i);
				break;
			}
			p = q->ipf_next;
			remque(q);
			m_freem(q->ipf_mbuf);
			q = p;
		}
	}

	/*
	 * Stick new segment in its place;
	 * check for complete reassembly.
	 */
	insque(ip, q->ipf_prev);
check:
	i = 0;
	ip = q = fp->ipq_next;
	do {
		if ((int)q->ipf_off != i) {
			IPFRAG_UNLOCK();
			return (0);
		}
		i += q->ipf_len;
	} while ((q = q->ipf_next) != (struct ipasfrag *)&fp->ipq_next);
	if (q->ipf_prev->ipf_mff) {
		IPFRAG_UNLOCK();
		return (0);
	}
	remque(fp);
	IPFRAG_UNLOCK();

	/*
	 * Reassembly is complete; concatenate and compress fragments,
	 * discard fragment reassembly header.
	 */
	m = ip->ipf_mbuf;
	q = ip->ipf_next;
	while (q != (struct ipasfrag *)&fp->ipq_next) {
		t = q->ipf_mbuf;
		q = q->ipf_next;
		m_cat(m, t);
	}
	FREE(fp, M_FTABLE);

	/*
	 * Create header for new ip packet by
	 * modifying header of first packet;
	 * Make header visible.
	 */
	ip->ipf_len = i;			/* note ip->ip_off == 0 */
	ip->ipf_mff = 0;
	hlen = (ip->ipf_vhl & 0x0f) << 2;
	m->m_len += hlen;
	m->m_data -= hlen;
	m->m_pkthdr.len = i + hlen;
	return m;

dropfrag:
	if (fp && fp->ipq_next == (struct ipasfrag *)&fp->ipq_next) {
		remque(fp);
		FREE(fp, M_FTABLE);
	}
	IPFRAG_UNLOCK();
dropfrag1:
	m_freem(m);
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_fragdropped++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	return (0);
}


/*
 * Free a fragment reassembly header and all
 * associated datagrams.
 */
void
ip_freef(fp)
	struct ipq *fp;
{
	register struct ipasfrag *q, *p;

	LOCK_ASSERT("ip_freef", lock_islocked(&ip_frag_lock));
        for (q = fp->ipq_next; q != (struct ipasfrag *)&fp->ipq_next; q = p) {
                p = q->ipf_next;
                m_freem(q->ipf_mbuf);
        }
        remque(fp);
        FREE(fp, M_FTABLE);
}

/*
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 */
void
ip_slowtimo()
{
	register struct ipq *fp;
	NETSTAT_LOCK_DECL()
	IPFRAG_LOCK_DECL()

        IPFRAG_LOCK();
        fp = ipq.next;
        if (fp) while (fp != &ipq) {
                --fp->ipq_ttl;
                fp = fp->next;
                if (fp->prev->ipq_ttl == 0) {
                        NETSTAT_LOCK(&ipstat.ips_lock);
                        ipstat.ips_fragtimeout++;
                        NETSTAT_UNLOCK(&ipstat.ips_lock);
                        ip_freef(fp->prev);
                }
        }
        IPFRAG_UNLOCK();
}

/*
 * Drain off some of our older datagram fragments.
 * We'll be called again if "some" aren't enough.
 */
void
ip_drain()
{
	NETSTAT_LOCK_DECL()
	IPFRAG_LOCK_DECL()
        int i = ipfragqmaxlen >> 2;

        IPFRAG_LOCK();
        while (ipq.next != &ipq && i-- >= 0) {
                NETSTAT_LOCK(&ipstat.ips_lock);
                ipstat.ips_fragdropped++;
                NETSTAT_UNLOCK(&ipstat.ips_lock);
                ip_freef(ipq.next);
        }
        IPFRAG_UNLOCK();
}

/*
 * Do option processing on a datagram,
 * possibly discarding it if bad options are encountered,
 * or forwarding it if source-routed.
 * Returns 1 if packet has been forwarded/freed,
 * 0 if the packet should be processed further.
 */
ip_dooptions(m)
	struct mbuf *m;
{
	NETSTAT_LOCK_DECL()
	register struct ip *ip = mtod(m, struct ip *);
	register u_char *cp;
	register struct ip_timestamp *ipt;
	register struct in_ifaddr *ia;
	int opt, optlen, cnt, off, code, type = ICMP_PARAMPROB, forward = 0;
	struct in_addr *sin;
	n_time ntime;
#if     NETISR_THREAD
        struct sockaddr_in ipaddr;

        ipaddr = in_zeroaddr;
#endif

	cp = (u_char *)(ip + 1);
	cnt = ((ip->ip_vhl & 0x0f) << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= 0 || optlen > cnt) {
				code = &cp[IPOPT_OLEN] - (u_char *)ip;
				goto bad;
			}
		}
		switch (opt) {

		default:
			break;

		/*
		 * Source routing with record.
		 * Find interface with current destination address.
		 * If none on this machine then drop if strictly routed,
		 * or do nothing if loosely routed.
		 * Record interface address and bring up next address
		 * component.  If strictly routed make sure next
		 * address is on directly accessible net.
		 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			ipaddr.sin_addr = ip->ip_dst;
			ia = (struct in_ifaddr *)
				ifa_ifwithaddr((struct sockaddr *)&ipaddr);
			if (ia == 0) {
				if (opt == IPOPT_SSRR) {
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				}
				/*
				 * Loose routing, and not at next destination
				 * yet; nothing to do except forward.
				 */
				break;
			}
			off--;			/* 0 origin */
			if (off > optlen - sizeof(struct in_addr)) {
				/*
				 * End of source route.  Should be for us.
				 */
				save_rte(cp, ip->ip_src);
				break;
			}
			/*
			 * locate outgoing interface
			 */
			bcopy((caddr_t)(cp + off), (caddr_t)&ipaddr.sin_addr,
			    sizeof(ipaddr.sin_addr));
			if (opt == IPOPT_SSRR) {
#define	INA	struct in_ifaddr *
#define	SA	struct sockaddr *
			    if ((ia = (INA)ifa_ifwithdstaddr((SA)&ipaddr)) == 0)
				ia = in_iaonnetof(in_netof(ipaddr.sin_addr));
			} else
				ia = ip_rtaddr(ipaddr.sin_addr);
			if (ia == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_SRCFAIL;
				goto bad;
			}
			if ( !nonlocsrcroute && 
			   (ia->ia_subnet != in_netof(ip->ip_dst))) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_SRCFAIL;
				goto bad;
			}
			ip->ip_dst = ipaddr.sin_addr;
			bcopy((caddr_t)&(IA_SIN(ia)->sin_addr),
			    (caddr_t)(cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			forward = 1;
			break;

		case IPOPT_RR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			/*
			 * If no space remains, ignore.
			 */
			off--;			/* 0 origin */
			if (off > optlen - sizeof(struct in_addr))
				break;
			bcopy((caddr_t)(&ip->ip_dst), (caddr_t)&ipaddr.sin_addr,
			    sizeof(ipaddr.sin_addr));
			/*
			 * locate outgoing interface; if we're the destination,
			 * use the incoming interface (should be same).
			 */
			if ((ia = (INA)ifa_ifwithaddr((SA)&ipaddr)) == 0 &&
			    (ia = ip_rtaddr(ipaddr.sin_addr)) == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_HOST;
				goto bad;
			}
			bcopy((caddr_t)&(IA_SIN(ia)->sin_addr),
			    (caddr_t)(cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			break;

		case IPOPT_TS:
			code = cp - (u_char *)ip;
			ipt = (struct ip_timestamp *)cp;
			if (ipt->ipt_len < 5)
				goto bad;
                        if (ipt->ipt_ptr > ipt->ipt_len - sizeof (long)) {
                                u_char oflw = (ipt->ipt_oflg & 0xf0) + 0x10;
                                ipt->ipt_oflg &= ~0xf0;
                                ipt->ipt_oflg |= oflw;
                                if (oflw == 0)
                                        goto bad;
                                break;
                        }
			sin = (struct in_addr *)(cp + ipt->ipt_ptr - 1);
			switch (ipt->ipt_oflg & 0x0f) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				ia = ifptoia(m->m_pkthdr.rcvif);
				bcopy((caddr_t)&IA_SIN(ia)->sin_addr,
				    (caddr_t)sin, sizeof(struct in_addr));
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			case IPOPT_TS_PRESPEC:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				bcopy((caddr_t)sin, (caddr_t)&ipaddr.sin_addr,
				    sizeof(struct in_addr));
				if (ifa_ifwithaddr((SA)&ipaddr) == 0)
					continue;
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			default:
				goto bad;
			}
			ntime = iptime();
			bcopy((caddr_t)&ntime, (caddr_t)cp + ipt->ipt_ptr - 1,
			    sizeof(n_time));
			ipt->ipt_ptr += sizeof(n_time);
		}
	}
	if (forward) {
		ip_forward(m, 1);
		return (1);
	} else
		return (0);
bad:
        /* SNMP BASE begin */
	NETSTAT_LOCK(&ipstat.ips_lock);
        ipstat.ipInHdrErrors++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
        /* SNMP BASE end */
	icmp_error(m, type, code, zeroin_addr);
	return (1);
}

/*
 * Given address of next destination (final or next hop),
 * return internet address info of interface to be used to get there.
 */
struct in_ifaddr *
ip_rtaddr(dst)
	 struct in_addr dst;
{
	register struct sockaddr_in *sin;
	ROUTE_LOCK_DECL()

	sin = (struct sockaddr_in *) &ipforward_rt.ro_dst;

	ROUTE_WRITE_LOCK();
	if (ipforward_rt.ro_rt == 0 || dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			rtfree_nolock(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = dst;

		rtalloc_nolock(&ipforward_rt);
	}
	ROUTE_WRITE_UNLOCK();
	if (ipforward_rt.ro_rt == 0)
		return ((struct in_ifaddr *)0);
	return ((struct in_ifaddr *) ipforward_rt.ro_rt->rt_ifa);
}

/*
 * Save incoming source route for use in replies,
 * to be picked up later by ip_srcroute if the receiver is interested.
 */
void
save_rte(option, dst)
	u_char *option;
	struct in_addr dst;
{
	unsigned olen;

	olen = option[IPOPT_OLEN];
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("save_rte: olen %d\n", olen);
#endif
	if (olen > sizeof(ip_srcrt) - (1 + sizeof(dst)))
		return;
	bcopy((caddr_t)option, (caddr_t)ip_srcrt.srcopt, olen);
	ip_nhops = (olen - IPOPT_OFFSET - 1) / sizeof(struct in_addr);
	ip_srcrt.dst = dst;
}

/*
 * Retrieve incoming source route for use in replies,
 * in the same form used by setsockopt.
 * The first hop is placed before the options, will be removed later.
 */
struct mbuf *
ip_srcroute()
{
	register struct in_addr *p, *q;
	register struct mbuf *m = NULL;
	int issource;

	if ((ip_nhops == 0) || 
           ((ip_nhops == 1) && (ip_srcrt.route[0].s_addr == ip_srcrt.dst.s_addr)))
		return ((struct mbuf *)0);

	m = m_get(M_DONTWAIT, MT_SOOPTS);
	if (m == 0)
		return ((struct mbuf *)0);

#define OPTSIZ	(sizeof(ip_srcrt.nop) + sizeof(ip_srcrt.srcopt))

	/* get rid of original source in option hop list */
        if (issource = (ip_srcrt.route[0].s_addr == ip_srcrt.dst.s_addr) ? sizeof(struct in_addr) : 0)
		ip_srcrt.srcopt[IPOPT_OLEN] -= sizeof(u_long);

	/* length is (nhops+1)*sizeof(addr) + sizeof(nop + srcrt header) */
	m->m_len = ip_nhops * sizeof(struct in_addr) + sizeof(struct in_addr) +
	    OPTSIZ - issource;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("ip_srcroute: nhops %d mlen %d", ip_nhops, m->m_len);
#endif
	/*
	 * First save first hop for return route
	 */
	p = &ip_srcrt.route[ip_nhops - 1];
	*(mtod(m, struct in_addr *)) = *p--;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf(" hops %X", ntohl(*mtod(m, struct in_addr *)));
#endif
	/*
	 * Copy option fields and padding (nop) to mbuf.
	 */
	ip_srcrt.nop = IPOPT_NOP;
	ip_srcrt.srcopt[IPOPT_OFFSET] = IPOPT_MINOFF;
	bcopy((caddr_t)&ip_srcrt.nop,
	    mtod(m, caddr_t) + sizeof(struct in_addr), OPTSIZ);
	q = (struct in_addr *)(mtod(m, caddr_t) +
	    sizeof(struct in_addr) + OPTSIZ);
#undef OPTSIZ
	/*
	 * Record return path as an IP source route,
	 * reversing the path (pointers are now aligned).
	 */
	while (p >= ip_srcrt.route) {
#if	INETPRINTFS
		if (inetprintfs > 1)
			printf(" %X", ntohl(*q));
#endif
		*q++ = *p--;
	}
	/*
	 * Last hop goes to final destination.
	 */
	if (!issource)
		*q = ip_srcrt.dst;

#if	INETPRINTFS
	if (inetprintfs > 1)
		printf(" %X\n", ntohl(*q));
#endif
	return (m);
}

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 * XXX should be deleted; last arg currently ignored.
 */
void
ip_stripoptions(m, mopt)
	register struct mbuf *m;
	struct mbuf *mopt;
{
	register int i;
	struct ip *ip = mtod(m, struct ip *);
	register caddr_t opts;
	int olen;

	olen = ((ip->ip_vhl & 0x0f) << 2) - sizeof (struct ip);
	opts = (caddr_t)(ip + 1);
	i = m->m_len - (sizeof (struct ip) + olen);
	bcopy(opts  + olen, opts, (unsigned)i);
	m->m_len -= olen;
	if (m->m_flags & M_PKTHDR)
		m->m_pkthdr.len -= olen;
	ip->ip_vhl = (IPVERSION << 4) | sizeof(struct ip) >> 2;
}

CONST u_char inetctlerrmap[PRC_NCMDS] = {
	0,		0,		0,		0,
	0,		EMSGSIZE,	EHOSTDOWN,	EHOSTUNREACH,
	EHOSTUNREACH,	EHOSTUNREACH,	ECONNREFUSED,	ECONNREFUSED,
	EMSGSIZE,	EHOSTUNREACH,	0,		0,
	0,		0,		0,		0,
	ENOPROTOOPT
};

/*
 * Forward a packet.  If some error occurs return the sender
 * an icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repertoire
 * of codes and types.
 *
 * If not forwarding, just drop the packet.  This could be confusing
 * if ipforwarding was zero but some routing protocol was advancing
 * us as a gateway to somewhere.  However, we must let the routing
 * protocol deal with that.
 *
 * The srcrt parameter indicates whether the packet is being forwarded
 * via a source route.
 */
void
ip_forward(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	register struct ip *ip = mtod(m, struct ip *);
	register struct sockaddr_in *sin;
	register struct rtentry *rt;
	int error, type = 0, code;
	struct mbuf *mcopy;
	struct in_addr dest;
	ROUTE_LOCK_DECL()
	NETSTAT_LOCK_DECL()
#if     NETISR_THREAD
        struct route ipforward_tmp;
#endif

	dest.s_addr = 0;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("forward: src %x dst %x ttl %x\n", ip->ip_src,
			ip->ip_dst, ip->ip_ttl);
#endif
	if (m_broadcast(m) || in_canforward(ip->ip_dst) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantforward++;
		ipstat.ipInAddrErrors++;	/* SNMP BASE */
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		m_freem(m);
		return;
	}
	ip->ip_id = htons(ip->ip_id);
	if (ip->ip_ttl <= IPTTLDEC) {
		icmp_error(m, ICMP_TIMXCEED, ICMP_TIMXCEED_INTRANS, dest);
		return;
	}
	ip->ip_ttl -= IPTTLDEC;

	sin = (struct sockaddr_in *)&ipforward_rt.ro_dst;
	ROUTE_WRITE_LOCK();
	if ((rt = ipforward_rt.ro_rt) == 0 ||
	    ip->ip_dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			rtfree_nolock(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = ip->ip_dst;

		rtalloc_nolock(&ipforward_rt);
		if (ipforward_rt.ro_rt == 0) {
			ROUTE_WRITE_UNLOCK();
			icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_HOST, dest);
			return;
		}
		rt = ipforward_rt.ro_rt;
	}
	ROUTE_WRITETOREAD_LOCK();

	/*
	 * Save the IP header plus 64 bits of the packet in case
	 * we need to generate an ICMP message to the src.
	 */
	code = ((ip->ip_vhl & 0x0f) << 2) + 8;  /* borrow code for length */
	if (code > (int)ip->ip_len) code = (int)ip->ip_len;
	mcopy = m_copym(m, 0, code, M_DONTWAIT);


#ifdef IP_IFMATRIX
	ip_ifmatrix[rt->rt_ifp->if_index +
	     if_index * m->m_pkthdr.rcvif->if_index]++;
#endif
	/*
	 * If forwarding packet using same interface that it came in on,
	 * perhaps should send a redirect to sender to shortcut a hop.
	 * Only send redirect if source is sending directly to us,
	 * and if packet was not source routed (or has any options).
	 * Also, don't send redirect if forwarding using a default route
	 * or a route modified by a redirect.
	 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
	if (rt->rt_ifp == m->m_pkthdr.rcvif &&
	    (rt->rt_flags & (RTF_DYNAMIC|RTF_MODIFIED)) == 0 &&
	    satosin(rt_key(rt))->sin_addr.s_addr != 0 &&
	    ipsendredirects && !srcrt) {
		struct in_ifaddr *ia;
		u_long src = ntohl(ip->ip_src.s_addr);
		u_long dst = ntohl(ip->ip_dst.s_addr);

		if ((ia = ifptoia(m->m_pkthdr.rcvif)) &&
		   (src & ia->ia_subnetmask) == ia->ia_subnet) {
		    if (rt->rt_flags & RTF_GATEWAY)
			dest = satosin(rt->rt_gateway)->sin_addr;
		    else
			dest = ip->ip_dst;
		    /*
		     * If the destination is reached by a route to host,
		     * is on a subnet of a local net, or is directly
		     * on the attached net (!), use host redirect.
		     * (We may be the correct first hop for other subnets.)
		     */
#define	RTA(rt)	((struct in_ifaddr *)(rt->rt_ifa))
		    type = ICMP_REDIRECT;
		    if ((rt->rt_flags & RTF_HOST) ||
		        (rt->rt_flags & RTF_GATEWAY) == 0)
			    code = ICMP_REDIRECT_HOST;
		    else if (RTA(rt)->ia_subnetmask != RTA(rt)->ia_netmask &&
		        (dst & RTA(rt)->ia_netmask) ==  RTA(rt)->ia_net)
			    code = ICMP_REDIRECT_HOST;
		    else
			    code = ICMP_REDIRECT_NET;
#if	INETPRINTFS
		    if (inetprintfs > 1)
		        printf("redirect (%d) to %x\n", code, dest.s_addr);
#endif
		}
	}

#if     NETISR_THREAD
        /*
         * Sigh. Unfortunately, we can't rely on ipforward_rt staying around
         * while we call ip_output(). We need to make a copy to save in case
         * somebody else mungs it. However note #define hack.
	 * Bump the ref count so the route doesn't disappear.
         */
        ipforward_tmp = ipforward_rt;
	ipforward_tmp.ro_rt->rt_refcnt++;
#define ipforward_rt    ipforward_tmp
#endif
        ROUTE_READ_UNLOCK();
#ifdef IP_MULTICAST
	error = ip_output(m, (struct mbuf *)0, &ipforward_rt, 
		IP_FORWARDING
#ifdef DIRECTED_BROADCAST
		| (directed_broadcast ? IP_ALLOWBROADCAST : 0)
#endif
			,(struct mbuf *)0);
#else
	error = ip_output(m, (struct mbuf *)0, &ipforward_rt, 
		IP_FORWARDING
#ifdef DIRECTED_BROADCAST
		| (directed_broadcast ? IP_ALLOWBROADCAST : 0)
#endif
		);
#endif
#if     NETISR_THREAD

	/* 
	 * Now free the route if no more references...
	 */
	if (ipforward_tmp.ro_rt != NULL)
		rtfree(ipforward_tmp.ro_rt);
#endif
	NETSTAT_LOCK(&ipstat.ips_lock);
	if (error) {
		ipstat.ips_cantforward++;
		ipstat.ipInAddrErrors++;	/* SNMP BASE */
	} else {
		ipstat.ips_forward++;
		if (type)
			ipstat.ips_redirectsent++;
		else {
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			if (mcopy)
				m_freem(mcopy);
			return;
		}
	}
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	if (mcopy == NULL)
		return;
	switch (error) {

	case 0:				/* forwarded, but need redirect */
		/* type, code set above */
		break;

	case ENETUNREACH:		/* shouldn't happen, checked above */
	case EHOSTUNREACH:
	case ENETDOWN:
	case EHOSTDOWN:
	default:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_HOST;
		break;

	case EMSGSIZE:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_NEEDFRAG;
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantfrag++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		break;

	case ENOBUFS:
		type = ICMP_SOURCEQUENCH;
		code = 0;
		break;
	}
	icmp_error(mcopy, type, code, dest);
}
