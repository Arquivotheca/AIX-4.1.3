static char sccsid[] = "@(#)31	1.9  src/bos/kernext/xns/ns_input.c, sysxxns, bos411, 9428A410j 3/21/94 15:46:45";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: idp_ctlinput
 *		idp_do_route
 *		idp_forward
 *		idp_undo_route
 *		imin
 *		ns_init
 *		ns_watch_output
 *		nsintr
 *		setnsroutemask
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
 *      Base:   ns_input.c      7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <net/netisr.h>
#include <netinet/in.h>

#include <netns/ns.h>
#include <netns/ns_if.h>
#include <netns/ns_pcb.h>
#include <netns/idp.h>
#include <netns/idp_var.h>
#include <netns/ns_error.h>
#include <netinet/if_ether.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>

struct idpstat idpstat;

#ifndef imin
#	define	imin(a, b)	(int) (((int) (a)) < ((int) (b)) ? (a) : (b))
#endif

/*
 * NS initialization.
 */
union ns_host	ns_thishost;
union ns_host	ns_zerohost;
union ns_host	ns_broadhost;
union ns_net	ns_zeronet;
union ns_net	ns_broadnet;
struct sockaddr_ns ns_netmask, ns_hostmask;

static u_short allones[] = {-1, -1, -1};

struct nspcb nspcb;
struct nspcb nsrawpcb;

struct ifqueue	nsintrq;
int	nsqmaxlen = IFQ_MAXLEN;

int	idpcksum = 1;
long	ns_pexseq;

/* ARGSUSED */
static void
setnsroutemask(dst, mask)
        struct sockaddr_ns *dst, *mask;
{
        *mask = ns_netmask;
}

#ifdef _AIXFULLOSF
/* ARGSUSED */
static int
ns_arpresolve(ac, m, dst, desten, flagsp)
        struct arpcom *ac;
        struct mbuf *m;
        struct sockaddr *dst;
        u_char *desten;
        int *flagsp;
{
        bcopy((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host),
            (caddr_t)desten, sizeof ns_thishost);
        if (bcmp((caddr_t)desten, (caddr_t)&ns_thishost, sizeof ns_thishost))
                return 1;
        (void) looutput((struct ifnet *)ac, m, dst, (struct rtentry *)0);
        return 0;
}

static struct arpent ns_arpent = {
        NULL, NULL, 0, 0, AF_NS, NULL,
        NULL, NULL, NULL,
        ns_arpresolve, NULL,
        NULL, NULL
};
#else /* _AIX_FULLOSF */
int
ns_arpresolve(ac, m, dst, eh)
        struct arpcom 		*ac;
        struct mbuf 		*m;
        struct sockaddr 	*dst;
	struct ether_header	*eh;
{
	struct ie3_mac_hdr	*ie3p;
	struct ie2_llc_hdr	*llcp;

        if (!bcmp((caddr_t)eh, (caddr_t)ac->ac_enaddr, 6)) {
		(void) looutput((struct ifnet *)ac, m, dst, 
			(struct rtentry *)0);
		return(0);
	}
	if (ac->ac_if.if_type != IFT_ETHER) {
		
		/*
		 * Adjust the header to be 802.3.
		 */
		m_adj(m, (sizeof(struct ie2_llc_snaphdr) - 
			sizeof(struct ie2_llc_hdr)));
		ie3p = mtod(m, struct ie3_mac_hdr *);

		*(struct char6 {char x[6];}*)ie3p->ie3_mac_dst = 
			*(struct char6 *)
		     ((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host));

		llcp = (struct ie2_llc_hdr *)(ie3p+1);
		llcp->dsap       = DSAP_XNS;
		llcp->ssap       = SSAP_XNS;
		llcp->ctrl       = CTRL_UI;
		ie3p->ie3_mac_len = m->m_pkthdr.len-sizeof(struct ie3_mac_hdr);
	} else {
		eh->ether_type = ETHERTYPE_NS;
		*(struct char6 {char x[6];}*)eh->ether_dhost = *(struct char6 *)
		    ((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host)); 
	}
	return (1);
}
#endif /* _AIXFULLOSF */

#ifdef NSIP
struct protosw xns_sw = { 
	SOCK_RAW,	0,	IPPROTO_IDP,	PR_ATOMIC|PR_ADDR,
	idpip_input,	0,	nsip_ctlinput,	0,
	0, 		0,	0,		0,		
	0,
};
#endif

void
ns_init()
{
        struct timestruc_t ct;
        extern void curtime();
	IFQ_LOCK_DECL()

	ns_broadhost = * (union ns_host *) allones;
	ns_broadnet = * (union ns_net *) allones;
	nspcb.nsp_next = nspcb.nsp_prev = &nspcb;
	nsrawpcb.nsp_next = nsrawpcb.nsp_prev = &nsrawpcb;
	nsintrq.ifq_maxlen = nsqmaxlen;
	/* ns_pexseq = time.tv_usec; */
        curtime(&ct);			/* get the current system time */
	ns_pexseq = ct.tv_nsec/1000;	/* use microsecond as seq no. */
	ns_netmask.sns_len = 6;
	ns_netmask.sns_addr.x_net = ns_broadnet;
	ns_hostmask.sns_len = 12;
	ns_hostmask.sns_addr.x_net = ns_broadnet;
	ns_hostmask.sns_addr.x_host = ns_broadhost;
#ifdef	NSIP
	/* set up protocol switch table in internet protocol */
	{
		extern int protosw_enable();
 		struct protosw *pr;
		pr = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
		if ( pr != 0 ) {
			/* enable the protocol switch so that the IP will handle
			 * the incoming xns encapsulating packet and pass along.
			 * The route pointers are passed because they are not
			 * resolve at load time.
			 */
			protosw_enable(&xns_sw); /* XXX Should check the rc and do ? */
		}
	}
#endif NSIP
        IFQ_LOCKINIT(&nsintrq);
        rtinithead(AF_NS, 16, setnsroutemask);
        (void) netisr_add(NETISR_NS, nsintr, &nsintrq, &nsdomain);
}
/*
 * Idp input routine.  Pass to next level.
 */
int nsintr_getpck = 0;
int nsintr_swtch = 0;
void
nsintr()
{
	register struct idp *idp;
	register struct mbuf *m;
	register struct nspcb *nsp;
	register int i;
	int len, s, error;
	char oddpacketp;
	IFQ_LOCK_DECL()

next:
	/*
	 * Get next datagram off input queue and get IDP header
	 * in first mbuf.
	 */
	IF_DEQUEUE(&nsintrq, m);
	nsintr_getpck++;
	if (m == 0)
		return;
	if ((m->m_flags & M_EXT || m->m_len < sizeof (struct idp)) &&
	    (m = m_pullup(m, sizeof (struct idp))) == 0) {
		idpstat.idps_toosmall++;
		goto next;
	}

	/*
	 * Give any raw listeners a crack at the packet
	 */
	for (nsp = nsrawpcb.nsp_next; nsp != &nsrawpcb; nsp = nsp->nsp_next) {
		struct mbuf *m1 = m_copy(m, 0, (int)M_COPYALL);
		if (m1) idp_input(m1, nsp);
	}

	idp = mtod(m, struct idp *);
	len = ntohs(idp->idp_len);
	if (oddpacketp = len & 1) {
		len++;		/* If this packet is of odd length,
				   preserve garbage byte for checksum */
	}

	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IDP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	if (m->m_pkthdr.len < len) {
		idpstat.idps_tooshort++;
		goto bad;
	}
	if (m->m_pkthdr.len > len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = len;
			m->m_pkthdr.len = len;
		} else
			m_adj(m, len - m->m_pkthdr.len);
	}
	if (idpcksum && ((i = idp->idp_sum)!=0xffff)) {
		idp->idp_sum = 0;
		if (i != (idp->idp_sum = ns_cksum(m, len))) {
			idpstat.idps_badsum++;
			idp->idp_sum = i;
			if (ns_hosteqnh(ns_thishost, idp->idp_dna.x_host))
				error = NS_ERR_BADSUM;
			else
				error = NS_ERR_BADSUM_T;
			ns_error(m, error, 0);
			goto next;
		}
	}
	/*
	 * Is this a directed broadcast?
	 */
	if (ns_hosteqnh(ns_broadhost,idp->idp_dna.x_host)) {
		if ((!ns_neteq(idp->idp_dna, idp->idp_sna)) &&
		    (!ns_neteqnn(idp->idp_dna.x_net, ns_broadnet)) &&
		    (!ns_neteqnn(idp->idp_sna.x_net, ns_zeronet)) &&
		    (!ns_neteqnn(idp->idp_dna.x_net, ns_zeronet)) ) {
			/*
			 * Look to see if I need to eat this packet.
			 * Algorithm is to forward all young packets
			 * and prematurely age any packets which will
			 * by physically broadcasted.
			 * Any very old packets eaten without forwarding
			 * would die anyway.
			 *
			 * Suggestion of Bill Nesheim, Cornell U.
			 */
			if (idp->idp_tc < NS_MAXHOPS) {
				idp_forward(m);
				goto next;
			}
		}
	/*
	 * Is this our packet? If not, forward.
	 */
	} else {
		struct ns_ifaddr *ia;
		for (ia = ns_ifaddr; ia; ia = ia->ia_next) {
			if (ns_hosteq(idp->idp_dna, ia->ia_addr.sns_addr))
				break;
		}
		if (!ia) {
			idp_forward(m);
			goto next;
		}
	}
	/*
	 * Locate pcb for datagram.
	 */
	nsp = ns_pcblookup(&idp->idp_sna, idp->idp_dna.x_port, NS_WILDCARD);
	/*
	 * Switch out to protocol's input routine.
	 */
	nsintr_swtch++;
	if (nsp) {
		if (oddpacketp) {
			m_adj(m, -1);
		}
		if ((nsp->nsp_flags & NSP_ALL_PACKETS)==0)
			switch (idp->idp_pt) {

			    case NSPROTO_SPP:
				    spp_input(m, nsp);
				    goto next;

			    case NSPROTO_ERROR:
				    ns_err_input(m);
				    goto next;
			}
		idp_input(m, nsp);
	} else {
		ns_error(m, NS_ERR_NOSOCK, 0);
	}
	goto next;

bad:
	m_freem(m);
	goto next;
}

u_char nsctlerrmap[PRC_NCMDS] = {
	ECONNABORTED,	ECONNABORTED,	0,		0,
	0,		0,		EHOSTDOWN,	EHOSTUNREACH,
	ENETUNREACH,	EHOSTUNREACH,	ECONNREFUSED,	ECONNREFUSED,
	EMSGSIZE,	0,		0,		0,
	0,		0,		0,		0
};

int idp_donosocks = 1;

void
idp_ctlinput(cmd, sa, arg)
	int cmd;
	struct sockaddr *sa;
	caddr_t arg;
{
	struct ns_addr *ns;
	struct nspcb *nsp;
	struct ns_errp *errp;
	int type;

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;

	switch(cmd) {

	/*
	 * If an interface is attached, we must add the cdli filters...
	 */
	case PRC_IFATTACH:
		ns_ifattach((struct ifnet *)sa);
		return;

	/*
	 * If the an interface is detached, then we must delete all filters
	 * added for the inet domain...
 	 */
	case PRC_IFDETACH:
		ns_ifdetach((struct ifnet *)sa);
		return;

	default:
		/* FALL THROUGH */
		break;
	}

	if (nsctlerrmap[cmd] == 0)
		return;		/* XXX */
	type = NS_ERR_UNREACH_HOST;
	switch (cmd) {
		struct sockaddr_ns *sns;

	case PRC_IFDOWN:
	case PRC_HOSTDEAD:
	case PRC_HOSTUNREACH:
                sns = (struct sockaddr_ns *)sa;
                if (sns == 0 || sns->sns_family != AF_NS)
			return;
		ns = &sns->sns_addr;
		break;


	default:
		errp = (struct ns_errp *)arg;
                if (errp == 0)
                        return;
		ns = &errp->ns_err_idp.idp_dna;
		type = errp->ns_err_num;
		type = ntohs((u_short)type);
	}
	switch (type) {

	case NS_ERR_UNREACH_HOST:
		ns_pcbnotify(ns, (int)nsctlerrmap[cmd], idp_abort, (long)0);
		break;

	case NS_ERR_NOSOCK:
		nsp = ns_pcblookup(ns, errp->ns_err_idp.idp_sna.x_port,
			NS_WILDCARD);
		if(nsp && idp_donosocks && ! ns_nullhost(nsp->nsp_faddr))
			(void) idp_drop(nsp, (int)nsctlerrmap[cmd]);
	}
}

int	idpprintfs = 0;
int	idpforwarding = 1;
/*
 * Forward a packet.  If some error occurs return the sender
 * an error packet.  Note we can't always generate a meaningful
 * error message because the NS errors don't have a large enough repetoire
 * of codes and types.
 */
struct route idp_droute;
struct route idp_sroute;

idp_forward(m)
struct mbuf *m;
{
	register struct idp *idp = mtod(m, struct idp *);
	register int error, type, code;
	struct mbuf *mcopy = NULL;
	int agedelta = 1;
	int flags = NS_FORWARDING;
	int ok_there = 0;
	int ok_back = 0;

	if (idpprintfs) {
		printf("forward: src ");
		ns_printhost(&idp->idp_sna);
		printf(", dst ");
		ns_printhost(&idp->idp_dna);
		printf("hop count %d\n", idp->idp_tc);
	}
	if (idpforwarding == 0) {
		/* can't tell difference between net and host */
		type = NS_ERR_UNREACH_HOST, code = 0;
		goto senderror;
	}
	idp->idp_tc++;
	if (idp->idp_tc > NS_MAXHOPS) {
		type = NS_ERR_TOO_OLD, code = 0;
		goto senderror;
	}
	/*
	 * Save at most 42 bytes of the packet in case
	 * we need to generate an NS error message to the src.
	 */
	mcopy = m_copy(m, 0, imin((int)ntohs(idp->idp_len), 42));

	if ((ok_there = idp_do_route(&idp->idp_dna,&idp_droute))==0) {
		type = NS_ERR_UNREACH_HOST, code = 0;
		goto senderror;
	}
	/*
	 * Here we think about  forwarding  broadcast packets,
	 * so we try to insure that it doesn't go back out
	 * on the interface it came in on.  Also, if we
	 * are going to physically broadcast this, let us
	 * age the packet so we can eat it safely the second time around.
	 */
	if (idp->idp_dna.x_host.c_host[0] & 0x1) {
		struct ns_ifaddr *ia = ns_iaonnetof(&idp->idp_dna);
		struct ifnet *ifp;
		if (ia) {
			/* I'm gonna hafta eat this packet */
			agedelta += NS_MAXHOPS - idp->idp_tc;
			idp->idp_tc = NS_MAXHOPS;
		}
		if ((ok_back = idp_do_route(&idp->idp_sna,&idp_sroute))==0) {
			/* error = ENETUNREACH; He'll never get it! */
			m_freem(m);
			goto cleanup;
		}
		if (idp_droute.ro_rt &&
		    (ifp=idp_droute.ro_rt->rt_ifp) &&
		    idp_sroute.ro_rt &&
		    (ifp!=idp_sroute.ro_rt->rt_ifp)) {
			flags |= NS_ALLOWBROADCAST;
		} else {
			type = NS_ERR_UNREACH_HOST, code = 0;
			goto senderror;
		}
	}
	/* need to adjust checksum */
	if (idp->idp_sum!=0xffff) {
		union bytes {
			u_char c[4];
			u_short s[2];
			long l;
		} x;
		register int shift;
		x.l = 0; x.c[0] = agedelta;
		shift = (((((int)ntohs(idp->idp_len))+1)>>1)-2) & 0xf;
		x.l = idp->idp_sum + (x.s[0] << shift);
		x.l = x.s[0] + x.s[1];
		x.l = x.s[0] + x.s[1];
		if (x.l==0xffff) idp->idp_sum = 0; else idp->idp_sum = x.l;
	}
	if ((error = ns_output(m, &idp_droute, flags)) && 
	    (mcopy!=NULL)) {
		idp = mtod(mcopy, struct idp *);
		type = NS_ERR_UNSPEC_T, code = 0;
		switch (error) {

		case ENETUNREACH:
		case EHOSTDOWN:
		case EHOSTUNREACH:
		case ENETDOWN:
		case EPERM:
			type = NS_ERR_UNREACH_HOST;
			break;

		case EMSGSIZE:
			type = NS_ERR_TOO_BIG;
			code = 576; /* too hard to figure out mtu here */
			break;

		case ENOBUFS:
			type = NS_ERR_UNSPEC_T;
			break;
		}
		mcopy = NULL;
	senderror:
		ns_error(m, type, code);
	}
cleanup:
	if (ok_there)
		idp_undo_route(&idp_droute);
	if (ok_back)
		idp_undo_route(&idp_sroute);
	if (mcopy != NULL)
		m_freem(mcopy);
}

idp_do_route(src, ro)
struct ns_addr *src;
struct route *ro;
{
	
	struct sockaddr_ns *dst;

	bzero((caddr_t)ro, sizeof (*ro));
	dst = (struct sockaddr_ns *)&ro->ro_dst;

	dst->sns_len = sizeof(*dst);
	dst->sns_family = AF_NS;
	dst->sns_addr = *src;
	dst->sns_addr.x_port = 0;
	rtalloc(ro);
	if (ro->ro_rt == 0 || ro->ro_rt->rt_ifp == 0) {
		return (0);
	}
	ro->ro_rt->rt_use++;
	return (1);
}

idp_undo_route(ro)
register struct route *ro;
{
	if (ro->ro_rt) {RTFREE(ro->ro_rt);}
}

ns_watch_output(m, ifp)
struct mbuf *m;
struct ifnet *ifp;
{
	register struct nspcb *nsp;
	register struct ifaddr *ifa;
	/*
	 * Give any raw listeners a crack at the packet
	 */
	for (nsp = nsrawpcb.nsp_next; nsp != &nsrawpcb; nsp = nsp->nsp_next) {
		struct mbuf *m0 = m_copy(m, 0, (int)M_COPYALL);
		if (m0) {
			register struct idp *idp;

			M_PREPEND(m0, sizeof (*idp), M_DONTWAIT);
			if (m0 == NULL)
				continue;
			idp = mtod(m0, struct idp *);
			idp->idp_sna.x_net = ns_zeronet;
			idp->idp_sna.x_host = ns_thishost;
			if (ifp && (ifp->if_flags & IFF_POINTOPOINT))
			    for(ifa = ifp->if_addrlist; ifa;
						ifa = ifa->ifa_next) {
				if (ifa->ifa_addr->sa_family==AF_NS) {
				    idp->idp_sna = IA_SNS(ifa)->sns_addr;
				    break;
				}
			    }
			idp->idp_len = ntohl(m0->m_pkthdr.len);
			idp_input(m0, nsp);
		}
	}
}
