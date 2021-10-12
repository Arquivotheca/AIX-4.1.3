static char sccsid[] = "@(#)72	1.20.1.7  src/bos/kernext/inet/ip_icmp.c, sysxinet, bos411, 9428A410j 5/27/94 15:08:09";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: icmp_error
 *		icmp_input
 *		icmp_reflect
 *		icmp_send
 *		ifptoia
 *		iptime
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *      Base:   (Berkeley)
 *      Merged: ip_icmp.c       7.14 (Berkeley) 6/28/90
 */

#include <net/net_globals.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <net/spl.h>
#include <net/netisr.h>

LOCK_ASSERTL_DECL

/*
 * ICMP routines: error generation, receive packet processing, and
 * routines to turnaround packets back to the originator, and
 * host table maintenance routines.
 */

struct icmpstat icmpstat;
int maxttl = MAXTTL;
int maxttl_dflt = MAXTTL;

#ifdef	_AIX
int   icmpaddressmask = 0;
int   icmpaddressmask_dflt = 0;
#endif

/*
 * Generate an error packet of type error
 * in response to bad packet ip.
 */
/*VARARGS3*/
void
icmp_error(n, type, code, dest)
	struct mbuf *n;
	int type, code;
	struct in_addr dest;
{
	register struct ip *oip = mtod(n, struct ip *), *nip;
	register unsigned oiplen = (oip->ip_vhl & 0x0f) << 2;
	register struct icmp *icp;
	register struct mbuf *m;
	unsigned icmplen;
	NETSTAT_LOCK_DECL()

#if	INETPRINTFS
	if (inetprintfs > 1) 
		printf("icmp_error(%x, %d, %d)\n", oip, type, code);
#endif
        if (type != ICMP_REDIRECT) {
                NETSTAT_LOCK(&icmpstat.icps_lock);
                icmpstat.icps_error++;
                NETSTAT_UNLOCK(&icmpstat.icps_lock);
        }
	/*
	 * Don't send error if not the first fragment of message.
	 * Don't error if the old packet protocol was ICMP
	 * error message, only known informational types.
	 * MUST use specific return dst from src, don't error if not.
	 */
	if (oip->ip_off &~ (IP_MF|IP_DF))
		goto freeit;
	if (oip->ip_p == IPPROTO_ICMP && type != ICMP_REDIRECT &&
	  n->m_len >= oiplen + ICMP_MINLEN &&
	  !ICMP_INFOTYPE(((struct icmp *)((caddr_t)oip + oiplen))->icmp_type)) {
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_oldicmp++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		goto freeit;
	}
	if (!oip->ip_src.s_addr ||		       /* zero addr  */
	    (oip->ip_src.s_addr == INADDR_LOOPBACK) || /* loopback   */
	    in_broadcast(oip->ip_src) ||        	/* b'cast     */
	    (n->m_flags & (M_BCAST | M_MCAST)) ||      /* link-layer */
	    IN_MULTICAST(oip->ip_src.s_addr) ||        /* class D    */
	    IN_BADCLASS(oip->ip_src.s_addr ))          /* class E    */
		goto freeit;

	/*
	 * First, formulate icmp message
	 */
	m = m_gethdr(M_DONTWAIT, MT_HEADER);
	if (m == NULL)
		goto freeit;
	icmplen = oiplen + MIN(8, oip->ip_len);
	m->m_len = icmplen + ICMP_MINLEN;
	MH_ALIGN(m, m->m_len);
	icp = mtod(m, struct icmp *);
	if ((u_int)type > ICMP_MAXTYPE)
		panic("icmp_error");
	NETSTAT_LOCK(&icmpstat.icps_lock);
	icmpstat.icps_outhist[type]++;
	NETSTAT_UNLOCK(&icmpstat.icps_lock);
	icp->icmp_type = type;
	if (type == ICMP_REDIRECT)
		icp->icmp_gwaddr = dest;
	else
		icp->icmp_void = 0;
	if (type == ICMP_PARAMPROB) {
		icp->icmp_pptr = code;
		code = 0;
	}
	icp->icmp_code = code;
	bcopy((caddr_t)oip, (caddr_t)&icp->icmp_ip, icmplen);
	nip = &icp->icmp_ip;
	nip->ip_len = htons((u_short)(nip->ip_len + oiplen));

	/*
	 * Now, copy old ip header (without options)
	 * in front of icmp message.
	 */
	if (m->m_data - sizeof(struct ip) < m->m_pktdat)
		panic("icmp len");
	m->m_data -= sizeof(struct ip);
	m->m_len += sizeof(struct ip);
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = n->m_pkthdr.rcvif;
	nip = mtod(m, struct ip *);
	bcopy((caddr_t)oip, (caddr_t)nip, sizeof(struct ip));
	nip->ip_len = m->m_len;
	nip->ip_vhl &= ~0x0f;
	nip->ip_vhl |= sizeof(struct ip) >> 2;
	nip->ip_p = IPPROTO_ICMP;
	icmp_reflect(m);

freeit:
	m_freem(n);
}

CONST static struct sockproto icmproto = { AF_INET, IPPROTO_ICMP };
/* This is a template for other local sockaddr_in's (ip, icmp, udp, etc) */
/* Global versions would need to be locked between threads. */
CONST struct sockaddr_in in_zeroaddr = { sizeof (struct sockaddr_in), AF_INET };
#if     !NETISR_THREAD
static struct sockaddr_in icmpsrc = { sizeof (struct sockaddr_in), AF_INET };
static struct sockaddr_in icmpdst = { sizeof (struct sockaddr_in), AF_INET };
static struct sockaddr_in icmpgw = { sizeof (struct sockaddr_in), AF_INET };
struct sockaddr_in icmpmask = { 8, 0 };
#endif

#ifndef BCASTPING
#define BCASTPING 0
#endif

int bcastping = BCASTPING;
int bcastping_dflt = BCASTPING;

struct ifqueue pfctlinputq;
int pfctlinputqlen=50;

void
queue_pfctlinput(int cmd, struct sockaddr_in *sin)
{
	struct mbuf	*m;
	IFQ_LOCK_DECL()

	m = m_gethdr(M_DONTWAIT, MT_HEADER);
	if (!m)
		return;
	*mtod(m, int *) = cmd;
	bcopy((caddr_t)sin, (caddr_t)(mtod(m, int *)+1), sizeof(*sin));

	IFQ_LOCK(&pfctlinputq);
	if (IF_QFULL(&pfctlinputq)) {
		IF_DROP(&pfctlinputq);
		m_free(m);
	} else {
		IF_ENQUEUE_NOLOCK(&pfctlinputq, m);
	}
	IFQ_UNLOCK(&pfctlinputq);
	schednetisr(NETISR_PFCTLINPUT);
}

void
dequeue_pfctlinput()
{
	struct mbuf	*m;
	struct sockaddr_in sin;
	IFQ_LOCK_DECL()
	int cmd;

	do {
		IF_DEQUEUE(&pfctlinputq, m);
		if (m) {
			cmd = *mtod(m, int *);
			bcopy((caddr_t)(mtod(m, int *)+1), (caddr_t)&sin, 
				sizeof(sin));
			pfctlinput(cmd, (struct sockaddr *)&sin);
			m_freem(m);
		}
	} while (m);
}

/*
 * Process a received ICMP message.
 */
void
icmp_input(m, hlen)
	register struct mbuf *m;
	int hlen;
{
	register struct icmp *icp;
	register struct ip *ip = mtod(m, struct ip *);
	int icmplen = ip->ip_len;
	register int i;
	struct in_ifaddr *ia;
	void (*ctlfunc)();
	int code;
#if     NETISR_THREAD
        struct sockaddr_in icmpsrc, icmpdst, icmpgw;
#endif
	NETSTAT_LOCK_DECL()

	/*
	 * Locate icmp structure in mbuf, and check
	 * that not corrupted and of at least minimum length.
	 */
#if     INETPRINTFS
	if (inetprintfs > 1)
		printf("icmp_input from %x, len %d\n", ip->ip_src, icmplen);
#endif
	if (icmplen < ICMP_MINLEN) {
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_tooshort++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		goto freeit;
	}
	i = hlen + MIN(icmplen, ICMP_ADVLENMIN);
 	if (m->m_len < i && (m = m_pullup(m, i)) == 0)  {
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_tooshort++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		return;
	}
 	ip = mtod(m, struct ip *);
	m->m_len -= hlen;
	m->m_data += hlen;
	icp = mtod(m, struct icmp *);
	if (in_cksum(m, icmplen)) {
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_checksum++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		goto freeit;
	}
	m->m_len += hlen;
	m->m_data -= hlen;
#if     NETISR_THREAD
        icmpsrc = in_zeroaddr;
        icmpdst = in_zeroaddr;
        icmpgw = in_zeroaddr;
#endif

#if	INETPRINTFS
	/*
	 * Message type specific processing.
	 */
	if (inetprintfs > 1)
		printf("icmp_input, type %d code %d\n", icp->icmp_type,
		    icp->icmp_code);
#endif
	if (icp->icmp_type > ICMP_MAXTYPE)
		goto raw;
	NETSTAT_LOCK(&icmpstat.icps_lock);
	icmpstat.icps_inhist[icp->icmp_type]++;
	NETSTAT_UNLOCK(&icmpstat.icps_lock);
	code = icp->icmp_code;
	switch (icp->icmp_type) {

	case ICMP_UNREACH:
		if (code > 5)
			goto badcode;
		code += PRC_UNREACH_NET;
		goto deliver;

	case ICMP_TIMXCEED:
		if (code > 1)
			goto badcode;
		code += PRC_TIMXCEED_INTRANS;
		goto deliver;

	case ICMP_PARAMPROB:
		if (code)
			goto badcode;
		code = PRC_PARAMPROB;
		goto deliver;

	case ICMP_SOURCEQUENCH:
		if (code)
			goto badcode;
		code = PRC_QUENCH;
	deliver:
		/*
		 * Problem with datagram; advise higher level routines.
		 */
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp) ||
		    (icp->icmp_ip.ip_vhl & 0x0f) < (sizeof(struct ip) >> 2)) {
			NETSTAT_LOCK(&icmpstat.icps_lock);
			icmpstat.icps_badlen++;
			NETSTAT_UNLOCK(&icmpstat.icps_lock);
			goto freeit;
		}
		icp->icmp_ip.ip_len = ntohs((u_short)icp->icmp_ip.ip_len);
#if	INETPRINTFS
		if (inetprintfs > 1)
			printf("deliver to protocol %d\n", icp->icmp_ip.ip_p);
#endif
		icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
/* LWR */
		if (*((ip_protox[icp->icmp_ip.ip_p].protosw)->pr_ctlinput))
			(*((ip_protox[icp->icmp_ip.ip_p].protosw)->pr_ctlinput)) (code, (struct sockaddr *)&icmpsrc,
			    (caddr_t) &icp->icmp_ip);
		break;

	badcode:
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_badcode++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		break;

	case ICMP_ECHO:
		/* MUST use specific return dst from src */
		if (!ip->ip_src.s_addr ||		      /* zero addr  */
		    in_broadcast(ip->ip_src) ||		      /* b'cast     */
		    (!bcastping && (m->m_flags & M_BCAST)) || /* link-layer */
		    IN_MULTICAST(ip->ip_src.s_addr) ||        /* class D    */
		    IN_BADCLASS(ip->ip_src.s_addr ))          /* class E    */
			goto freeit;
		icp->icmp_type = ICMP_ECHOREPLY;
		goto reflect;

	case ICMP_TSTAMP:
		/* MUST use specific return dst from src */
		if (!ip->ip_src.s_addr ||		      /* zero addr  */
		    in_broadcast(ip->ip_src) ||		      /* b'cast     */
		    (m->m_flags & M_BCAST) ||                 /* link-layer */
		    IN_MULTICAST(ip->ip_src.s_addr) ||        /* class D    */
		    IN_BADCLASS(ip->ip_src.s_addr ))          /* class E    */
			goto freeit;
		if (icmplen < ICMP_TSLEN) {
			NETSTAT_LOCK(&icmpstat.icps_lock);
			icmpstat.icps_badlen++;
			NETSTAT_UNLOCK(&icmpstat.icps_lock);
			break;
		}
		icp->icmp_type = ICMP_TSTAMPREPLY;
		icp->icmp_rtime = iptime();
		icp->icmp_ttime = icp->icmp_rtime;	/* bogus, do later! */
		goto reflect;
		
	case ICMP_IREQ:
#define	satosin(sa)	((struct sockaddr_in *)(sa))
		if (in_netof(ip->ip_src) == 0 &&
		    (ia = ifptoia(m->m_pkthdr.rcvif)))
			ip->ip_src = in_makeaddr(in_netof(IA_SIN(ia)->sin_addr),
			    in_lnaof(ip->ip_src));
		icp->icmp_type = ICMP_IREQREPLY;
		goto reflect;

	case ICMP_MASKREQ:
	      if (icmpaddressmask) {
                if (icmplen < ICMP_MASKLEN ||
                    (ia = ifptoia(m->m_pkthdr.rcvif)) == 0 ||
                    ((ia->ia_ifp->if_flags & IFF_POINTOPOINT) == 0 &&
                     !(ipforwarding)))
                        break;
		icp->icmp_type = ICMP_MASKREPLY;
		icp->icmp_mask = ia->ia_sockmask.sin_addr.s_addr;
		if (ip->ip_src.s_addr == 0) {
			if (ia->ia_ifp->if_flags & IFF_BROADCAST)
			    ip->ip_src = satosin(&ia->ia_broadaddr)->sin_addr;
			else if (ia->ia_ifp->if_flags & IFF_POINTOPOINT)
			    ip->ip_src = satosin(&ia->ia_dstaddr)->sin_addr;
		}
	      } else goto freeit;
reflect:
		ip->ip_len += hlen;	/* since ip_input deducts this */
		NETSTAT_LOCK(&icmpstat.icps_lock);
		icmpstat.icps_reflect++;
		icmpstat.icps_outhist[icp->icmp_type]++;
		NETSTAT_UNLOCK(&icmpstat.icps_lock);
		m->m_flags &= ~(M_BCAST | M_MCAST);     /* link-layer */
		icmp_reflect(m);
		return;

	case ICMP_REDIRECT:
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			NETSTAT_LOCK(&icmpstat.icps_lock);
			icmpstat.icps_badlen++;
			NETSTAT_UNLOCK(&icmpstat.icps_lock);
			break;
		}
		/*
		 * Short circuit routing redirects to force
		 * immediate change in the kernel's routing
		 * tables.  The message is also handed to anyone
		 * listening on a raw socket (e.g. the routing
		 * daemon for use in updating its tables).
		 */
		icmpgw.sin_addr = ip->ip_src;
		icmpdst.sin_addr = icp->icmp_gwaddr;
#if	INETPRINTFS
		if (inetprintfs > 1)
			printf("redirect dst %x to %x\n", icp->icmp_ip.ip_dst,
				icp->icmp_gwaddr);
#endif
		if (code == ICMP_REDIRECT_NET || code == ICMP_REDIRECT_TOSNET) {
#if     NETISR_THREAD
                        struct sockaddr_in icmpmask;
                        icmpmask = in_zeroaddr;
#endif
			icmpsrc.sin_addr =
			 in_makeaddr(in_netof(icp->icmp_ip.ip_dst), INADDR_ANY);
			in_sockmaskof(icp->icmp_ip.ip_dst, &icmpmask);
			rtredirect((struct sockaddr *)&icmpsrc,
			  (struct sockaddr *)&icmpdst,
			  (struct sockaddr *)&icmpmask, RTF_GATEWAY,
			  (struct sockaddr *)&icmpgw, (struct rtentry **)0);
			icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
			queue_pfctlinput(PRC_REDIRECT_NET,
			  (struct sockaddr *)&icmpsrc);
		} else {
			icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
			rtredirect((struct sockaddr *)&icmpsrc,
			  (struct sockaddr *)&icmpdst,
			  (struct sockaddr *)0, RTF_GATEWAY | RTF_HOST,
			  (struct sockaddr *)&icmpgw, (struct rtentry **)0);
			queue_pfctlinput(PRC_REDIRECT_HOST,
			  (struct sockaddr *)&icmpsrc);
		}
		break;

	/*
	 * No kernel processing for the following;
	 * just fall through to send to raw listener.
	 */
	case ICMP_ECHOREPLY:
	case ICMP_TSTAMPREPLY:
	case ICMP_IREQREPLY:
	case ICMP_MASKREPLY:
	default:
		break;
	}

raw:
	icmpsrc.sin_addr = ip->ip_src;
	icmpdst.sin_addr = ip->ip_dst;
	(void) raw_input(m, (struct sockproto *)&icmproto, 
		(struct sockaddr *)&icmpsrc, (struct sockaddr *)&icmpdst);
	return;

freeit:
	m_freem(m);
}

/*
 * Reflect the ip packet back to the source
 */
void
icmp_reflect(m)
	struct mbuf *m;
{
	register struct ip *ip = mtod(m, struct ip *);
	register struct in_ifaddr *ia;
	struct in_addr t;
	struct mbuf *opts = 0;
	int optlen = ((ip->ip_vhl & 0x0f) << 2) - sizeof(struct ip);

	t = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	/*
	 * If the incoming packet was addressed directly to us,
	 * use dst as the src for the reply.  Otherwise (broadcast
	 * or anonymous), use the address which corresponds
	 * to the incoming interface.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
		if (t.s_addr == IA_SIN(ia)->sin_addr.s_addr)
			break;
		if ((ia->ia_ifp->if_flags & IFF_BROADCAST) &&
		    t.s_addr == satosin(&ia->ia_broadaddr)->sin_addr.s_addr)
			break;
	}
	if (ia == (struct in_ifaddr *)0)
		ia = ifptoia(m->m_pkthdr.rcvif);
	if (ia == (struct in_ifaddr *)0)
		ia = in_ifaddr;
	t = IA_SIN(ia)->sin_addr;
	ip->ip_src = t;
	ip->ip_ttl = maxttl;

	if (optlen > 0) {
		register u_char *cp;
		int opt, cnt;
		u_int len;

		/*
		 * Retrieve any source routing from the incoming packet;
		 * add on any record-route or timestamp options.
		 */
		cp = (u_char *) (ip + 1);
		if ((opts = ip_srcroute()) == 0 &&
		    (opts = m_gethdr(M_DONTWAIT, MT_HEADER))) {
			opts->m_len = sizeof(struct in_addr);
			mtod(opts, struct in_addr *)->s_addr = 0;
		}
		if (opts) {
#if	INETPRINTFS
		    if (inetprintfs > 1)
			    printf("icmp_reflect optlen %d rt %d => ",
				optlen, opts->m_len);
#endif
		    for (cnt = optlen; cnt > 0; cnt -= len, cp += len) {
			    opt = cp[IPOPT_OPTVAL];
			    if (opt == IPOPT_EOL)
				    break;
			    if (opt == IPOPT_NOP)
				    len = 1;
			    else {
				    len = cp[IPOPT_OLEN];
				    if (len <= 0 || len > cnt)
					    break;
			    }
			    /*
			     * should check for overflow, but it "can't happen"
			     */
			    if (opt == IPOPT_RR || opt == IPOPT_TS) {
				    bcopy((caddr_t)cp,
					mtod(opts, caddr_t) + opts->m_len, len);
				    opts->m_len += len;
			    }
		    }
		    if (opts->m_len % 4 != 0) {
			    *(mtod(opts, caddr_t) + opts->m_len) = IPOPT_EOL;
			    opts->m_len++;
		    }
#if	INETPRINTFS
		    if (inetprintfs > 1)
			    printf("%d\n", opts->m_len);
#endif
		}
		/*
		 * Now strip out original options by copying rest of first
		 * mbuf's data back, and adjust the IP length.
		 */
		ip->ip_len -= optlen;
                ip->ip_vhl &= ~0x0f;
                ip->ip_vhl |= sizeof(struct ip) >> 2;
		m->m_len -= optlen;
		if (m->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= optlen;
		optlen += sizeof(struct ip);
		bcopy((caddr_t)ip + optlen, (caddr_t)(ip + 1),
			 (unsigned)(m->m_len - sizeof(struct ip)));
	}
	icmp_send(m, opts);
	if (opts)
		(void)m_free(opts);
}

struct in_ifaddr *
ifptoia(ifp)
	struct ifnet *ifp;
{
	register struct in_ifaddr *ia;

	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == ifp)
			return (ia);
	return ((struct in_ifaddr *)0);
}

/*
 * Send an icmp packet back to the ip level,
 * after supplying a checksum.
 */
void
icmp_send(m, opts)
	register struct mbuf *m;
	struct mbuf *opts;
{
	register struct ip *ip = mtod(m, struct ip *);
	register int hlen;
	register struct icmp *icp;

	hlen = (ip->ip_vhl & 0x0f) << 2;
	m->m_data += hlen;
	m->m_len -= hlen;
	icp = mtod(m, struct icmp *);
	icp->icmp_cksum = 0;
	icp->icmp_cksum = in_cksum(m, ip->ip_len - hlen);
	m->m_data -= hlen;
	m->m_len += hlen;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("icmp_send dst %x src %x\n", ip->ip_dst, ip->ip_src);
#endif
#ifdef IP_MULTICAST
	(void) ip_output(m, opts, (struct route *)0, 0, (struct mbuf *)0);
#else
	(void) ip_output(m, opts, (struct route *)0, 0);
#endif
}

n_time
iptime()
{
	struct timeval atv;
	u_long t;
        struct timestruc_t ct;
        extern void curtime();

#ifdef	_AIX
        curtime(&ct);
        ct.tv_nsec = ct.tv_nsec/1000;
        t = ((ct.tv_sec % (24*60*60) * 1000) + (ct.tv_nsec/1000));
#else
	microtime(&atv);
	t = (atv.tv_sec % (24*60*60)) * 1000 + atv.tv_usec / 1000;
#endif
	return (htonl(t));
}
