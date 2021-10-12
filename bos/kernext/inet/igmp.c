static char sccsid[] = "@(#)89	1.7  src/bos/kernext/inet/igmp.c, sysxinet, bos411, 9428A410j 3/15/94 16:52:08";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: DEBUGMSG
 *		igmp_fasttimo
 *		igmp_init
 *		igmp_input
 *		igmp_joingroup
 *		igmp_leavegroup
 *		igmp_sendreport
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
 * Internet Group Management Protocol (IGMP) routines.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * adapted to AIX by Matthias Kaiserswerth, 1992
 *
 * MULTICAST 1.1
 */

#if INETPRINTFS
#define DEBUGMSG(x)  { if (inetprintfs > 1) printf x ;}
#else
#define DEBUGMSG(x)  
#endif

#ifdef IP_MULTICAST
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/protosw.h>

#include <net/nh.h>		/*pdw*/
#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/igmp.h>
#include <netinet/igmp_var.h>


struct igmpstat igmpstat;
static struct sockproto   igmpproto = { AF_INET, IPPROTO_IGMP };
static struct sockaddr_in igmpsrc   = { sizeof(struct sockaddr_in), AF_INET };
static struct sockaddr_in igmpdst   = { sizeof(struct sockaddr_in), AF_INET };

static int                igmp_timers_are_running = 0;
static u_long             igmp_all_hosts_group;

void igmp_init()
{
	/*
	 * To avoid byte-swapping the same value over and over again.
	 */
	igmp_all_hosts_group = htonl(INADDR_ALLHOSTS_GROUP);
	NETSTAT_LOCKINIT(&igmpstat.igps_lock);
}

void igmp_input(register struct mbuf *m, int hlen)
{
	register struct ifnet *ifp;
	register struct igmp *igmp;
	register struct ip *ip = mtod(m, struct ip *);
	u_short igmplen = ip->ip_len;
	int minlen;
	struct in_multi *inm;
	struct in_ifaddr *ia;
	NETSTAT_LOCK_DECL()
	IFMULTI_LOCK_DECL()
	INIFADDR_LOCK_DECL()

	NETSTAT_LOCK(&igmpstat.igps_lock);
	++igmpstat.igps_rcv_total;
	NETSTAT_UNLOCK(&igmpstat.igps_lock);
	DEBUGMSG(("igmp_input for %s\n", m->m_pkthdr.rcvif->if_name));

	/*
	 * Validate lengths
	 */
	if (igmplen < IGMP_MINLEN) {
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_tooshort;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  DEBUGMSG(("igmp_input for %s too short\n", m->m_pkthdr.rcvif->if_name));
	  m_freem(m);
	  return;
	}
	minlen = hlen + IGMP_MINLEN;
	if (m->m_len < minlen && (m = m_pullup(m, minlen)) == 0) {
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_tooshort;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  DEBUGMSG(("igmp_input for %s too short after pullup\n", m->m_pkthdr.rcvif->if_name));
	  return;
	}

	/*
	 * Validate checksum
	 */
	ip = mtod(m, struct ip *);
	m->m_data += hlen;
	m->m_len -= hlen;
	igmp = mtod(m, struct igmp *);
	if (in_cksum(m, igmplen)) {
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_badsum;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  DEBUGMSG(("igmp_input for %s bad checksum\n", m->m_pkthdr.rcvif->if_name));
	  m_freem(m);
	  return;
	}

	ifp = m->m_pkthdr.rcvif;

	switch (igmp->igmp_type) {

	case IGMP_HOST_MEMBERSHIP_QUERY:
	  DEBUGMSG(("IGMP_HOST_MEMBERSHIP_QUERY for %x on %s\n", igmp->igmp_group.s_addr, ifp->if_name));
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_queries;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  
	  if (ifp == &loif)
	    break;
	  
	  if (ip->ip_dst.s_addr != igmp_all_hosts_group) {
	    NETSTAT_LOCK(&igmpstat.igps_lock);
	    ++igmpstat.igps_rcv_badqueries;
	    NETSTAT_UNLOCK(&igmpstat.igps_lock);
	    m_freem(m);
	    return;
	  }
	  
	  /*
	   * Start the timers in all of our membership records for
	   * the interface on which the query arrived, except those
	   * that are already running and those that belong to the
	   * "all-hosts" group.
	   */
	  IFMULTI_LOCK(ifp);
	  for(inm=ifp->if_multiaddrs; inm; inm = inm->inm_next) {
	    if (inm->inm_timer == 0 && 
		inm->inm_addr.s_addr != igmp_all_hosts_group) {
	      inm->inm_timer = IGMP_RANDOM_DELAY(inm->inm_addr);
	      igmp_timers_are_running = 1;
	    }
	  }
	  IFMULTI_UNLOCK(ifp);

	  break;

	case IGMP_HOST_MEMBERSHIP_REPORT:
	  DEBUGMSG(("IGMP_HOST_MEMBERSHIP_REPORT for %x on %s\n", igmp->igmp_group.s_addr, ifp->if_name));
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_reports;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);

	  if (ifp == &loif)
	    break;
	  
	  if (!IN_MULTICAST(ntohl(igmp->igmp_group.s_addr)) ||
	      igmp->igmp_group.s_addr != ip->ip_dst.s_addr) {
	    NETSTAT_LOCK(&igmpstat.igps_lock);
	    ++igmpstat.igps_rcv_badreports;
	    NETSTAT_UNLOCK(&igmpstat.igps_lock);
	    m_freem(m);
	    return;
	  }
	  
	  /*
	   * KLUDGE: if the IP source address of the report has an
	   * unspecified (i.e., zero) subnet number, as is allowed for
	   * a booting host, replace it with the correct subnet number
	   * so that a process-level multicast routing demon can
	   * determine which subnet it arrived from.  This is necessary
	   * to compensate for the lack of any way for a process to
	   * determine the arrival interface of an incoming packet.
	   */
	  if ((ntohl(ip->ip_src.s_addr) & IN_CLASSA_NET) == 0) {
	    IFP_TO_IA(ifp, ia);
	    if (ia) ip->ip_src.s_addr = htonl(ia->ia_subnet);
	  }
	  
	  /*
	   * If we belong to the group being reported, stop
	   * our timer for that group.
	   */
	  IN_LOOKUP_MULTI(igmp->igmp_group, ifp, inm);
	  if (inm != NULL) {
	    DEBUGMSG(("IGMP_HOST_MEMBERSHIP_REPORT we belong to the group %x\n", igmp->igmp_group.s_addr));
	    inm->inm_timer = 0;
	    NETSTAT_LOCK(&igmpstat.igps_lock);
	    ++igmpstat.igps_rcv_ourreports;
	    NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  }
	  
	  break;

	default:
	  DEBUGMSG(("igmp_input for %s unknown type %x\n", m->m_pkthdr.rcvif->if_name, igmp->igmp_type));
	  NETSTAT_LOCK(&igmpstat.igps_lock);
	  ++igmpstat.igps_rcv_badreports;
	  NETSTAT_UNLOCK(&igmpstat.igps_lock);
	  m_freem(m);
	  return;
	  break;
	}

	/*
	 * Pass all valid IGMP packets up to any process(es) listening
	 * on a raw IGMP socket.
	 */
	igmpsrc.sin_addr = ip->ip_src;
	igmpdst.sin_addr = ip->ip_dst;
	raw_input(m, &igmpproto,
		    (struct sockaddr *)&igmpsrc, (struct sockaddr *)&igmpdst);
}

void igmp_joingroup(struct in_multi *inm)
{
	DEBUGMSG(("igmp_joingroup %x on %s\n", inm->inm_addr.s_addr, inm->inm_ifp->if_name));
	if (inm->inm_addr.s_addr == igmp_all_hosts_group ||
	    inm->inm_ifp == &loif) {
		inm->inm_timer = 0;
	} else {
		igmp_sendreport(inm);
		inm->inm_timer = IGMP_RANDOM_DELAY(inm->inm_addr);
		igmp_timers_are_running = 1;
	}
}

void igmp_leavegroup(struct in_multi *inm)
{
	/*
	 * No action required on leaving a group.
	 */
}

void igmp_fasttimo()
{
	register struct in_multi *inm;
	struct in_multistep step;
	IFMULTI_LOCK_DECL()

	/*
	 * Quick check to see if any work needs to be done, in order
	 * to minimize the overhead of fasttimo processing.
	 */
	if (!igmp_timers_are_running)
		return;

	igmp_timers_are_running = 0;
	IN_FIRST_MULTI(step, inm);
	while (inm != NULL) {

		if (inm->inm_timer == 0) {
			/* do nothing */
		}
		else if (--inm->inm_timer == 0) {
			igmp_sendreport(inm);
		}
		else {
			igmp_timers_are_running = 1;
		}
		IN_NEXT_MULTI(step, inm);
	}
	UNLOCK_LAST_MULTI(step);
}

void igmp_sendreport(struct in_multi *inm)
{
	struct mbuf *m;
	struct igmp *igmp;
	struct ip *ip;
	struct mbuf *mopts;
	struct ip_moptions *imo;
#ifdef MROUTE
	extern struct socket *ip_mrouter;
#endif
	NETSTAT_LOCK_DECL();

	LOCK_ASSERT("igmp_sendreport ifmulti", IFMULTI_ISLOCKED(inm->inm_ifp));

	if (!(m=m_gethdr(M_DONTWAIT, MT_HEADER)))
		return;
	MGET(mopts, M_DONTWAIT, MT_IPMOPTS);
	if (mopts == NULL) {
		m_free(m);
		DEBUGMSG(("igmp_sendreport can't get mbuf\n"));
		return;
	}
 	m->m_len = IGMP_MINLEN;
	MH_ALIGN(m, m->m_len);

	igmp = mtod(m, struct igmp *);
	igmp->igmp_type   = IGMP_HOST_MEMBERSHIP_REPORT;
	igmp->igmp_code   = 0;
	igmp->igmp_group  = inm->inm_addr;
	igmp->igmp_cksum  = 0;
	igmp->igmp_cksum  = in_cksum(m, IGMP_MINLEN);
	if (m->m_data - sizeof(struct ip) < m->m_pktdat)
	  panic("igmp len");
	m->m_data -= sizeof(struct ip);
	m->m_len += sizeof(struct ip);
	m->m_pkthdr.len = m->m_len;
 	ip = mtod(m, struct ip *);

	ip->ip_tos        = 0;
	ip->ip_len        = m->m_len;
	ip->ip_vhl 	  = (IPVERSION << 4) | (sizeof(struct ip) >> 2);
	ip->ip_off        = 0;
	ip->ip_p          = IPPROTO_IGMP;
	ip->ip_src.s_addr = INADDR_ANY;
	ip->ip_dst        = igmp->igmp_group;

	imo = mtod(mopts, struct ip_moptions *);
	imo->imo_multicast_ifp  = inm->inm_ifp;
	imo->imo_multicast_ttl  = 1;
#ifdef MROUTE
	/*
	 * Request loopback of the report if we are acting as a multicast
	 * router, so that the process-level routing demon can hear it.
	 */
	imo->imo_multicast_loop = (ip_mrouter != NULL);
#else
	imo->imo_multicast_loop = 0;
#endif
	ip_output(m, (struct mbuf *)0, (struct route *)0,
			IP_MULTICASTOPTS|IP_IFMULTI_NOLOCK, mopts);

	m_free(mopts);
	NETSTAT_LOCK(&igmpstat.igps_lock);
	++igmpstat.igps_snd_reports;
	NETSTAT_UNLOCK(&igmpstat.igps_lock);
}

#endif /* IP_MULTICAST */
