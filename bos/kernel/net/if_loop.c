static char sccsid[] = "@(#)28	1.41  src/bos/kernel/net/if_loop.c, sysnet, bos411, 9435A411a 8/29/94 11:05:20";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: loattach
 *		loifp
 *		loinit
 *		loioctl
 *		looutput
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
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	Base:	if_loop.c	7.9 (Berkeley) 9/20/89
 *	Merged:	if_loop.c	7.10 (Berkeley) 6/28/90
 */

/*
 * Loopback interface driver for protocol testing and timing.
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/ioctl.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/if_types.h"
#include "net/netisr.h"
#include "net/route.h"

#ifdef	_AIX
#include "sys/cdli.h"
#include "net/nd_lan.h"
#include "net/spl.h"
#endif	/* _AIX */

#include <sys/time.h>
#include <net/bpf.h>
static caddr_t lo_bpf;

#define	LOMTU	(16384+512)

LOCK_ASSERTL_DECL

struct	ifnet loif;
struct intr lo_offl;
int offlinit = 0;

extern int net_xmit_trace(struct ifnet *, struct mbuf *);

void
loinit()
{
	IFQ_LOCKINIT(&(loif.if_snd));
	NETSTAT_LOCKINIT(&(loif.if_slock));
}

void
loattach()
{
	register struct ifnet *ifp = &loif;

	ifp->if_name = "lo";
	ifp->if_mtu = LOMTU;
 	ifp->if_flags = IFF_LOOPBACK;
	ifp->if_flags |= IFF_BPF;	/* Enable bpf support*/
	ifp->if_tap = NULL;
#ifdef IP_MULTICAST
	ifp->if_flags |= IFF_MULTICAST;
	IFMULTI_INITLOCK(ifp);
#endif
	ifp->if_ioctl = loioctl;
	ifp->if_output = looutput;
	ifp->if_type = IFT_LOOP;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
	if_nostat (ifp);
	if_attach(ifp);
}

/* ARGSUSED */
looutput(ifp, m, dst, rt)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	struct rtentry *rt;
{
	NETSTAT_LOCK_DECL()
	IFQ_LOCK_DECL()

	if ((m->m_flags & M_PKTHDR) == 0)
		panic("looutput no HDR");

	if (!(ifp->if_flags & (IFF_UP)) ) {
		loif.if_snd.ifq_drops++;
		m_freem(m);
		return (ENETDOWN);
	}

	if (ifp->if_tap) {

		if ((caddr_t)ifp->if_tap == (caddr_t)net_xmit_trace) 
				(* ifp->if_tap)((ifp->if_tapctl), m);
		else {

			/*
			 * We need to prepend the address family as
			 * a four byte field.  Cons up a dummy header
			 * to pacify bpf.  This is safe because bpf
			 * will only read from the mbuf (i.e., it won't
			 * try to free it or keep a pointer to it).
			 */
			struct mbuf m0;
			u_int af = dst->sa_family;

			m0.m_flags |= M_PKTHDR;
			m0.m_next = m;
			m0.m_len = 4;
			m0.m_pkthdr.len = m->m_pkthdr.len+4;
			m0.m_data = (char *)&af;
			(* ifp->if_tap)((ifp->if_tapctl), &m0);
		}
	}

	m->m_pkthdr.rcvif = ifp;

#if	defined(RTF_REJECT)
        if (rt &&
	    rt->rt_flags & (RTF_REJECT|RTF_BLACKHOLE)) {
                m_freem(m);
                return (rt->rt_flags & RTF_BLACKHOLE ? 0 : (rt->rt_flags & RTF_HOST ? EHOSTUNREACH : ENETUNREACH));
        }
#endif
	NETSTAT_LOCK(&ifp->if_slock);
	loif.if_opackets++;
	loif.if_obytes += m->m_pkthdr.len;
	NETSTAT_UNLOCK(&ifp->if_slock);

#ifndef	_AIX
	return netisr_input(netisr_af((int)dst->sa_family), m, (caddr_t)0, 0);
#else	/* _AIX */
	if ((dst->sa_family >= AF_MAX) || 
	    (af_table[dst->sa_family].config.loop == NULL)) {
		loif.if_noproto++;
		m_freem(m);
		return (EAFNOSUPPORT);
	}

	IFQ_LOCK(af_table[dst->sa_family].config.loopq);
	/* if not full, enqueue and schednetisr	*/
	if (IF_QFULL(af_table[dst->sa_family].config.loopq)) {
		IF_DROP(af_table[dst->sa_family].config.loopq);
		loif.if_iqdrops++;
		m_freem(m);
	} else {
		IF_ENQUEUE_NOLOCK(af_table[dst->sa_family].config.loopq, m);
		NETSTAT_LOCK(&ifp->if_slock);
		loif.if_ipackets++;
		loif.if_ibytes += m->m_pkthdr.len;
		NETSTAT_UNLOCK(&ifp->if_slock);
	}

	IFQ_UNLOCK(af_table[dst->sa_family].config.loopq);
	if (dst->sa_family == AF_INET) {
		if (offlinit == 0) {
			offlinit++;
			INIT_OFFL3(&lo_offl, netisr_table[NETISR_IP].isr, 0);
		}
		i_sched(&lo_offl);
        } else
                schednetisr(af_table[dst->sa_family].config.netisr);

	return(0);
#endif	/* _AIX */
}

/*
 * Process an ioctl request.
 */
/* ARGSUSED */
loioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	int error = 0;
#ifdef IP_MULTICAST
	register struct ifreq *ifr = (struct ifreq *)data;
#endif IP_MULTICAST

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
#ifdef	 _AIX
		{
		struct timestruc_t ct;

		curtime (&ct);
		ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
		ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
		}
#endif	/* _AIX */
		/*
		 * Everything else is done at a higher level.
		 */
		break;

#ifdef IP_MULTICAST
	case SIOCADDMULTI:
	case SIOCDELMULTI:
		switch (ifr->ifr_addr.sa_family) {
#ifdef INET
		case AF_INET:
			break;
#endif INET
		default:
			error = EAFNOSUPPORT;
			break;
		}
		break;
#endif IP_MULTICAST

	default:
		error = EINVAL;
	}
	return (error);
}

#ifndef	_AIX_FULLOSF
/************************************************************************
 *
 *	loifp() - return address of ifnet structure for loopback device
 *
 ***********************************************************************/
struct ifnet *
loifp()
{
	return(&loif);
}
#endif	/* _AIX_FULLOSF */
