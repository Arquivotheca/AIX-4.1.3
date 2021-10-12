static char sccsid[] = "@(#)74	1.23  src/bos/kernext/inet/raw_ip.c, sysxinet, bos41B, 412_41B_sync 12/1/94 14:59:26";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: DEBUGMSG
 *		rip_ctloutput
 *		rip_input
 *		rip_output
 *		rip_usrreq
 *		satosin
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
 *      Base:   raw_ip.c        7.6 (Berkeley) 9/20/89
 *      Merged: raw_ip.c        7.7 (Berkeley) 6/28/90
 *      Merged: raw_ip.c        7.8 (Berkeley) 7/25/90
 */

#include <net/net_globals.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/nettrace.h>

#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <net/spl.h>
#include <net/netopt.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_pcb.h>

#ifdef IP_MULTICAST
#define DEBUGMSG(x) printf x
#endif

#include <net/net_malloc.h>

LOCK_ASSERTL_DECL

/*
 * Raw interface to IP protocol.
 */

#if      !NETISR_THREAD
struct	sockaddr_in ripdst = { sizeof(ripdst), AF_INET };
struct	sockaddr_in ripsrc = { sizeof(ripsrc), AF_INET };
struct	sockproto ripproto = { PF_INET };
#endif


/*
 * Setup generic address and protocol structures
 * for raw_input routine, then pass them along with
 * mbuf chain.
 */
/*ARGSUSED*/
void
rip_input(m, len)
	struct mbuf *m;
{
	register struct ip *ip = mtod(m, struct ip *);
	NETSTAT_LOCK_DECL()
#if     NETISR_THREAD
        struct  sockaddr_in ripdst, ripsrc;
        struct  sockproto ripproto;

        ripdst = in_zeroaddr;
        ripsrc = in_zeroaddr;
        ripproto.sp_family = PF_INET;
#endif

	ripproto.sp_protocol = ip->ip_p;
	ripdst.sin_addr = ip->ip_dst;
	ripsrc.sin_addr = ip->ip_src;
	if (raw_input(m, &ripproto, (struct sockaddr *)&ripsrc,
	  (struct sockaddr *)&ripdst) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_noproto++;
		ipstat.ips_delivered--;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
	}
}

/*
 * Generate IP header and pass packet to ip_output.
 * Tack on options user may have setup with control call.
 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
rip_output(m, so)
	register struct mbuf *m;
	struct socket *so;
{
	register struct ip *ip;
	register struct raw_inpcb *rp = sotorawinpcb(so);
	register struct sockaddr_in *sin;
	struct mbuf *opts;
	INIFADDR_LOCK_DECL()
	int flags = (so->so_options & SO_DONTROUTE) | IP_ALLOWBROADCAST;

	LOCK_ASSERT("rip_output", SOCKET_ISLOCKED(so));

	/*
	 * If the user handed us a complete IP packet, use it.
	 * Otherwise, allocate an mbuf for a header and fill it in.
	 */
	if (rp->rinp_flags & RINPF_HDRINCL) {
		ip = mtod(m, struct ip *);
                if (ip->ip_id == 0) {
                        ip->ip_id = htons(fetch_and_add(&ip_id,1));
		}
                /* XXX prevent ip_output from overwriting header fields */
                flags |= IP_RAWOUTPUT;
                ipstat.ips_rawout++;
		opts = NULL;
		if ((ip->ip_vhl & 0x0f) << 2 == 0)
			ip->ip_vhl = (IPVERSION << 4) | (sizeof(struct ip) >> 2);
#ifdef IP_MULTICAST
		if (ip->ip_src.s_addr != 0) {
		  /*
		   * Verify that the source address is one of ours.
		   */
		  struct ifnet *ifp;
		  INADDR_TO_IFP(ip->ip_src, ifp);
		  if (ifp == NULL) {
		    m_freem(m);
		    return EADDRNOTAVAIL;
		  }
		}
		if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr)))
			ip->ip_dst = ((struct sockaddr_in *)&rp->rinp_faddr)->sin_addr;
#endif IP_MULTICAST
	} else {
		M_PREPEND(m, sizeof(struct ip), M_DONTWAIT);
		if (m == NULL) {
			return(ENOBUFS);
		}
		ip = mtod(m, struct ip *);
		ip->ip_tos = 0;
		ip->ip_off = 0;
		ip->ip_p = rp->rinp_rcb.rcb_proto.sp_protocol;
		ip->ip_len = m->m_pkthdr.len;
		if (sin = satosin(rp->rinp_rcb.rcb_laddr)) {
			ip->ip_src = sin->sin_addr;
		} else
			ip->ip_src.s_addr = 0;
                if (sin = satosin(rp->rinp_rcb.rcb_faddr))
                    ip->ip_dst = sin->sin_addr;
		ip->ip_ttl = maxttl;
		opts = rp->rinp_options;
	}
#ifdef IP_MULTICAST
        return (ip_output(m,
            opts,
	    &rp->rinp_route,
            flags | IP_MULTICASTOPTS,
            rp->rinp_moptions));
#else
        return (ip_output(m,
	    opts,
            &rp->rinp_route,
            flags);
#endif
}

/*
 * Raw IP socket option processing.
 */
rip_ctloutput(op, so, level, optname, m)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **m;
{
	int error = 0;
	register struct raw_inpcb *rp;
	
	if (level != IPPROTO_IP)
		error = EINVAL;
	else
		if (!(so->so_state & SS_PRIV))
			error = EACCES;
	if (error)
		return (error);

	rp = sotorawinpcb(so);
	if (rp == (struct raw_inpcb *) NULL) {
		return(ENOTCONN);
	}

	switch (op) {

	case PRCO_SETOPT:
		switch (optname) {

		case IP_OPTIONS:
			return(ip_pcbopts(&rp->rinp_options, *m));

		case IP_HDRINCL:
			if (m == 0 || *m == 0 || (*m)->m_len < sizeof (int)) {
				error = EINVAL;
				break;
			}
			if (*mtod(*m, int *))
				rp->rinp_flags |= RINPF_HDRINCL;
			else
				rp->rinp_flags &= ~RINPF_HDRINCL;
			break;

#ifdef IP_MULTICAST
                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_setmoptions(so, optname, 
				&rp->rinp_moptions, *m);
                        break;
#endif IP_MULTICAST

#ifdef IP_MULTICAST
#ifdef MROUTE
                default:
                        error = ip_mrouter_cmd(optname, so, *m);
                        break;
#else
                default:
                        error = EINVAL;
                        break;
#endif MROUTE
#else
                default:
                        error = EINVAL;
                        break;
#endif IP_MULTICAST

		}
		break;

	case PRCO_GETOPT:
		*m = m_get(M_DONTWAIT, MT_SOOPTS);
		if (*m == NULL) {
			error = ENOBUFS;
			break;
		}

		switch (optname) {

		case IP_OPTIONS:
			if (rp->rinp_options) {
				(*m)->m_len = rp->rinp_options->m_len;
				bcopy(mtod(rp->rinp_options, caddr_t),
				    mtod(*m, caddr_t), (unsigned)(*m)->m_len);
			} else
				(*m)->m_len = 0;
			break;

		case IP_HDRINCL:
			(*m)->m_len = sizeof (int);
			*mtod(*m, int *) = rp->rinp_flags & RINPF_HDRINCL;
			break;

#ifdef IP_MULTICAST
                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_getmoptions(optname, rp->rinp_moptions, m);
                        break;
#endif IP_MULTICAST

		default:
			error = EINVAL;
			m_freem(*m);
			*m = 0;
			break;
		}
		break;
	}
	if (op == PRCO_SETOPT && *m)
		(void)m_free(*m);
	return (error);
}

/*ARGSUSED*/
rip_usrreq(so, req, m, nam, control)
	register struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register int error = 0;
	register struct raw_inpcb *rp = sotorawinpcb(so);

	LOCK_ASSERT("rip_usrreq", SOCKET_ISLOCKED(so));

	switch (req) {

	case PRU_ATTACH:
		if (rp)
			panic("rip_attach");
		
		NET_MALLOC(rp, struct raw_inpcb *, sizeof *rp, M_PCB, M_NOWAIT);
		if (rp == NULL) {
			error = ENOBUFS;
			return(error);
		}
			
		bzero((caddr_t)rp, sizeof *rp);
		so->so_pcb = (caddr_t)rp;
		break;

	case PRU_DETACH:
		if (rp == 0)
			panic("rip_detach");
		if (rp->rinp_options)
			m_freem(rp->rinp_options);
		if (rp->rinp_route.ro_rt)
			RTFREE(rp->rinp_route.ro_rt);
		if (rp->rinp_rcb.rcb_laddr)
			rp->rinp_rcb.rcb_laddr = 0;
#ifdef IP_MULTICAST
#ifdef MROUTE
                {
                  extern struct socket *ip_mrouter;
                  if (so == ip_mrouter)
                    ip_mrouter_done();
                }
#endif MROUTE
                  if (rp->rinp_rcb.rcb_proto.sp_family == AF_INET) {
			SOCKET_UNLOCK(so);
			ip_freemoptions(rp->rinp_moptions);
			SOCKET_LOCK(so);
		  }
#endif IP_MULTICAST
		break;

	case PRU_BIND:
	    {
		struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);

		if (nam->m_len != sizeof(*addr))
			return (EINVAL);
		if ((ifnet == 0) ||
		    ((addr->sin_family != AF_INET) &&
		     (addr->sin_family != AF_IMPLINK)) ||
		    (addr->sin_addr.s_addr &&
		     ifa_ifwithaddr((struct sockaddr *)addr) == 0))
			return (EADDRNOTAVAIL);
		rp->rinp_rcb.rcb_laddr = (struct sockaddr *)&rp->rinp_laddr;
		rp->rinp_laddr = *addr;
		return (0);
	    }
	case PRU_CONNECT:
	    {
		struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);

		if (nam->m_len != sizeof(*addr))
			return (EINVAL);
		if (ifnet == 0)
			return (EADDRNOTAVAIL);
		if ((addr->sin_family != AF_INET) &&
		     (addr->sin_family != AF_IMPLINK))
			return (EAFNOSUPPORT);
		rp->rinp_rcb.rcb_faddr = (struct sockaddr *)&rp->rinp_faddr;
		rp->rinp_faddr = *addr;
		soisconnected(so);
		return (0);
	    }
	}
	error =  raw_usrreq(so, req, m, nam, control);

	if (error && (req == PRU_ATTACH) && so->so_pcb)
		NET_FREE(so->so_pcb, M_PCB);
	return (error);
}
