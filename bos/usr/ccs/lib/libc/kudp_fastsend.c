static char sccsid[] = "@(#)13  1.10  src/bos/usr/ccs/lib/libc/kudp_fastsend.c, libcrpc, bos41J, 9511A_all 3/3/95 09:51:44";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: ku_sendto_mbuf
 *		pr_mbuf
 *		
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)kudp_fastsend.c	1.9 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.16 88/02/08
 */

#ifdef	_KERNEL

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/atomic_op.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_systm.h>
#include <netinet/ip_var.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

/* Kernel RPC's own flag for checksumming */
int krpc_udpcksum = NFS_CHKSUM_DEFAULT;		


/*
 * Sendtries and Sendok counters which can be looked at via the
 * nfs_cntl() interface.  There is some room for error in the counter
 * because the more expensive fetch_and_add() call is not used to 
 * increment the counters
 */
int Sendtries = 0;
int Sendok = 0;
#define SENDTRIES()	(void)fetch_and_add((atomic_p)&Sendtries, 1)
#define SENDOK()	(void)fetch_and_add((atomic_p)&Sendok, 1)

#ifdef DEBUG

int route_hit = 0;
int route_missed = 0;
#define ROUTE_HIT()	(void)fetch_and_add((atomic_p)&route_hit, 1)
#define ROUTE_MISSED()	(void)fetch_and_add((atomic_p)&route_missed, 1)

#else /* DEBUG */

#define ROUTE_HIT()	
#define ROUTE_MISSED()	

#endif /* DEBUG */

ku_sendto_mbuf(so, am, to, soreply)
	struct socket *so;		/* socket data is sent from */
	register struct mbuf *am;	/* data to be sent */
	struct sockaddr_in *to;		/* destination data is sent to */
	struct socket *soreply;		/* socket used for response */
{
	register int maxlen;		/* max length of fragment */
	register int curlen;		/* data fragment length */
	register int fragoff;		/* data fragment offset */
	register int grablen;		/* number of mbuf bytes to grab */
	register struct ip *ip;		/* ip header */
	register struct mbuf *m;	/* ip header mbuf */
	int error;			/* error number */
	struct ifnet *ifp;		/* interface */
	struct in_ifaddr *ia;
	struct sockaddr_in *dst, tmp_dst;/* packet destination */
	struct inpcb *inp;		/* inpcb for binding */
	struct ip *nextip;		/* ip header for next fragment */
	volatile struct in_addr tmp_in_addr;	/*addr holder so we can unlock*/
	register struct route *ro;
	struct mbuf *m_split_pageboundary();
	register int sum;
	struct	socket	*routeso;


	ROUTE_LOCK_DECL()

	SENDTRIES();
	/*
	 * Determine length of data.
	 * This should be passed in as a parameter.
	 */
	curlen = 0;
	for (m = am; m; m = m->m_next) {
		curlen += m->m_len;
	}

	/* determine which of the sockets passed in should be used for routing
	   */
	if (soreply)
		routeso = soreply;
	else
		routeso = so;

	/*
	 * Bind local port if necessary.  This will usually put a route in the
	 * pcb, so we just use that one if we have not changed destinations.
	 */
	SOCKET_LOCK(routeso);
	if ( (inp = sotoinpcb(routeso)) == 0 )  {
		SOCKET_UNLOCK(routeso);
		return(ENOTCONN);
	}
		
	if ((inp->inp_lport == 0) &&
	    (inp->inp_laddr.s_addr == INADDR_ANY))
		(void) in_pcbbind(inp, (struct mbuf *) 0);


	ROUTE_WRITE_LOCK();
	ro = &inp->inp_route;
	dst = (struct sockaddr_in *) &ro->ro_dst;

	if (ro->ro_rt == 0 || (ro->ro_rt->rt_flags & RTF_UP) == 0 ||
	    dst->sin_addr.s_addr != to->sin_addr.s_addr) {
		if (ro->ro_rt) {
			rtfree_nolock(ro->ro_rt);
		}
		ro->ro_rt = (struct rtentry *)0;   /* zero contents */
		dst->sin_family = AF_INET;
		dst->sin_addr = to->sin_addr;
		dst->sin_len = sizeof(*dst);
		rtalloc_nolock(ro);
		ROUTE_MISSED();
		if (ro->ro_rt == 0 || ro->ro_rt->rt_ifp == 0) {
			(void) m_freem(am);
			ROUTE_WRITE_UNLOCK();
			SOCKET_UNLOCK(routeso);
			return (ENETUNREACH);
		}
	} else ROUTE_HIT();
	ifp = ro->ro_rt->rt_ifp;
	ro->ro_rt->rt_use++;
	if (ro->ro_rt->rt_flags & RTF_GATEWAY) {
		dst = (struct sockaddr_in *)ro->ro_rt->rt_gateway;
	}

	/*
	 * save the in_addr locally so we can release the route lock 
	 */
	ia = (struct in_ifaddr *)ro->ro_rt->rt_ifa;
	tmp_in_addr = ia->ia_addr.sin_addr;
	tmp_dst = *dst;

	ROUTE_WRITE_UNLOCK();
	SOCKET_UNLOCK(routeso);

	/*
	 * Get mbuf for ip, udp headers.
	 */
	MGETHDR(m, M_WAIT, MT_HEADER);
	if (m == NULL) {
		(void) m_freem(am);
		return (ENOBUFS);
	}
	MH_ALIGN(m, (sizeof (struct ip) + sizeof (struct udphdr)));
	m->m_len = sizeof (struct ip) + sizeof (struct udphdr);
	ip = mtod(m, struct ip *);

	/*
	 * Create pseudo-header and UDP header, and calculate
	 * the UDP level checksum only if desired.
	 */
	{
#define	ui ((struct udpiphdr *)ip)

		ui->ui_pr = IPPROTO_UDP;
		ui->ui_len = htons((u_short) curlen + sizeof (struct udphdr));
		ui->ui_src = tmp_in_addr;
		ui->ui_src = ia->ia_addr.sin_addr;
		ui->ui_dst = to->sin_addr;
		if (soreply)
			ui->ui_sport = sotoinpcb(so)->inp_lport;
		else
			ui->ui_sport = inp->inp_lport;
		ui->ui_dport = to->sin_port;
		ui->ui_ulen = ui->ui_len;
		ui->ui_sum = 0;
		if (krpc_udpcksum) {
			ui->ui_next = ui->ui_prev = 0;
			ui->ui_x1 = 0;
			m->m_next = am;
			ui->ui_sum = in_cksum(m,
				sizeof (struct udpiphdr) + curlen);
			if (ui->ui_sum == 0)
				ui->ui_sum = 0xFFFF;
		}
# undef ui
	}
	/*
	 * Now that the pseudo-header has been checksummed, we can
	 * fill in the rest of the IP header except for the IP header
	 * checksum, done for each fragment.
	 */
	ip->ip_hl = sizeof (struct ip) >> 2;
	ip->ip_v = IPVERSION;
	ip->ip_tos = 0;
	ip->ip_id = htons(get_ip_id());
	ip->ip_off = 0;
	ip->ip_ttl = MAXTTL;

	/*
	 * Fragnemt the data into packets big enough for the
	 * interface, prepend the header, and send them off.
	 */
	maxlen = (ifp->if_mtu - sizeof (struct ip)) & ~7;
	curlen = sizeof (struct udphdr);
	fragoff = 0;
	for (;;) {
		register struct mbuf *mm;
		register struct mbuf *lam;	/* last mbuf in chain */

		/*
		 * Assertion: m points to an mbuf containing a mostly filled
		 * in ip header, while am points to a chain which contains
		 * all the data.
		 * The problem here is that there may be too much data.
		 * If there is, we have to fragment the data (and maybe the
		 * mbuf chain).
		 */
		m->m_next = am;
		lam = m;
		while (am->m_len + curlen <= maxlen) {
			curlen += am->m_len;
			lam = am;
			am = am->m_next;
			if (am == 0) {
				ip->ip_off = htons((u_short) (fragoff >> 3));
				goto send;
			}
		}
		if (curlen == maxlen) {
			/*
			 * Incredible luck: last mbuf exactly
			 * filled out the packet.
			 */
			lam->m_next = 0;
		} else {
			/*
			 * Have to fragment the mbuf chain.  am points
			 * to the mbuf that has too much, so we take part
			 * of its data, point mm to it, and attach mm to
			 * the current chain.  lam conveniently points to
			 * the last mbuf of the current chain.
			 */
			MGET(mm, M_WAIT, MT_DATA);
			if (mm == NULL) {
				(void) m_freem(m);
				return(ENOBUFS);
			}
			grablen = maxlen - curlen;
			mm->m_next = NULL;
			mm->m_data = mtod(am, int);
			mm->m_len = grablen;
			/* copy the external info from am */
			mm->m_ext = am->m_ext;
			mm->m_flags |= M_EXT;
			mm->m_forw = mm->m_back = &mm->m_ext.ext_ref;
			MCLREFERENCE(mm, am);
			/* correct length */
			mm->m_ext.ext_size = mm->m_len;

			lam->m_next = mm;

			am->m_len -= grablen;
			/* adjust cluster length based on split */
			am->m_ext.ext_size = am->m_len;
			am->m_data += grablen;
			curlen = maxlen;
		}
		/*
		 * Assertion: m points to an mbuf chain of data which
		 * can be sent, while am points to a chain containing
		 * all the data that is to be sent in later fragments.
		 */
		ip->ip_off = htons((u_short) ((fragoff >> 3) | IP_MF));
		/*
		 * There are more frags, so we save
		 * a copy of the ip hdr for the next
		 * frag.
		 */
		MGETHDR(mm, M_WAIT, MT_HEADER);
		if (mm == 0) {
			(void) m_free(m);	/* this frag */
			(void) m_freem(am);	/* rest of data */
			return (ENOBUFS);
		}
		MH_ALIGN(mm, sizeof(struct ip));
		mm->m_len = sizeof (struct ip);
		nextip = mtod(mm, struct ip *);
		*nextip = *ip;
send:
		/* Before we send, the data portions of the mbuf must 
		 * be checked to ensure that they do not cross page
		 * boundaries.
		 */
		if (!m_split_pageboundary(m)) {
			(void) m_freem(m);
			if (am) {
				(void) m_freem(am);
				(void) m_free(mm);
			}
			return(ENOBUFS);
		}
		/*
		 * Set ip_len and calculate the ip header checksum.
		 * Note ips[5] is skipped below since it IS where the
		 * checksum will eventually go.
		 */
		ip->ip_len = htons(sizeof (struct ip) + curlen);
#define	ips ((u_short *) ip)
		sum = ips[0] + ips[1] + ips[2] + ips[3] + ips[4]
			+ ips[6] + ips[7] + ips[8] + ips[9];
		sum = (sum & 0xFFFF) + (sum >> 16);
		ip->ip_sum = ~((sum & 0xFFFF) + (sum >> 16));
#undef ips
		m->m_pkthdr.len = ip->ip_len;
		/*
		 * Send it off to the newtork.
		 */
		if (error = (*ifp->if_output)(ifp, m, &tmp_dst, ro->ro_rt)) {
			if (am) {
				(void) m_freem(am);	/* rest of data */
				(void) m_free(mm);	/* hdr for next frag */
			}
			return (error);
		}
		if (am == 0) {			/* All fragments sent OK */
			SENDOK();
			return (0);
		}
		ip = nextip;
		m = mm;
		fragoff += curlen;
		curlen = 0;
	}
	/*NOTREACHED*/
}


/* Given an mbuf pointer, walk the list looking for mbuf data that spans
 * a page boundary.  If this type of data is found, the mbuf must be split 
 * into two mbufs.  A new one will be added and the data split appropriately.
 */

struct mbuf *
m_split_pageboundary(struct mbuf *m)
{
	struct mbuf *tm, *sm;
	uint	pbeg, pend, grablen;

	for (tm = m; tm; tm = tm->m_next) {
		pbeg = mtod(m, uint);
		pend = pbeg + m->m_len - 1;

		/* Does the data span a page boundary? */
		if (pbeg / PAGESIZE != pend / PAGESIZE) {
			MGET(sm, M_WAIT, MT_DATA);
			if (sm == NULL) {
				return((struct mbuf *)NULL);
			}
			/* how much should be left in the original mbuf? */
			grablen = PAGESIZE - (pbeg % PAGESIZE);
			/* set up new mbuf */
			sm->m_data = mtod(tm, int) + grablen;
			sm->m_len = tm->m_len - grablen;
			sm->m_next = tm->m_next;
			/* adjust original mbuf */
			tm->m_next = sm;
			tm->m_len = grablen;

			/* external stuff - do not forget to update 
			   external sizes to match updated mbuf data sizes.
			   */
			sm->m_ext = tm->m_ext;
			sm->m_flags != M_EXT;
			sm->m_forw = sm->m_back = &sm->m_ext.ext_ref;
			MCLREFERENCE(sm, tm);
			sm->m_ext.ext_size = sm->m_len;
			
			tm->m_ext.ext_size = tm->m_len;
		}
	}
	return(m);
}

#ifdef DEBUG
pr_mbuf(p, m)
	char *p;
	struct mbuf *m;
{
	register char *cp, *cp2;
	register struct ip *ip;
	register int len;

	len = 28;
	printf("%s: ", p);
	if (m && m->m_len >= 20) {
		ip = mtod(m, struct ip *);
		printf("hl %d v %d tos %d len %d id %d mf %d off ",
			ip->ip_hl, ip->ip_v, ip->ip_tos, ip->ip_len,
			ip->ip_id, ip->ip_off >> 13, ip->ip_off & 0x1fff);
		printf("%d ttl %d p %d sum %d src %x dst %x\n",
			ip->ip_ttl, ip->ip_p, ip->ip_sum, ip->ip_src.s_addr,
			ip->ip_dst.s_addr);
		len = 0;
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		m = m->m_next;
	} else if (m) {
		printf("pr_mbuf: m_len %d\n", m->m_len);
	} else {
		printf("pr_mbuf: zero m\n");
	}
	while (m) {
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		cp = mtod(m, caddr_t);
		cp2 = cp + m->m_len;
		while (cp < cp2) {
			if (len-- < 0) {
				break;
			}
			printf("%x ", *cp & 0xFF);
			cp++;
		}
		m = m->m_next;
		printf("\n");
	}
}
#endif /* DEBUG*/
#endif	/* _KERNEL */
