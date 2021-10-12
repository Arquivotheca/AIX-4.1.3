static char sccsid[] = "@(#)93	1.15.1.27  src/bos/kernext/inet/udp_usrreq.c, sysxinet, bos41J, 9511A_all 3/8/95 15:07:38";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: 
 *		udp_cksum_and_move
 *		udp_ctlinput
 *		udp_init
 *		udp_input
 *		udp_notify
 *		udp_output
 *		udp_receive
 *		udp_saveopt
 *		udp_send
 *		udp_usrreq
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
/* udp_usrreq.c     2.1 16:12:29 4/20/90 SecureWare */
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
 * Copyright (c) 1982, 1986, 1988, 1990 Regents of the University of California.
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
 *      Merged: udp_usrreq.c    7.17 (Berkeley) 7/1/90
 *      Merged: udp_usrreq.c    7.20 (Berkeley) 4/20/91
 */

#include "net/net_globals.h"

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/nettrace.h>
#include <sys/syspest.h>

#include <net/if.h>
#include <net/route.h>
#include <net/spl.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

struct inpcb udb;
struct inpcb_hash_table udp_pcb_hash_table[INPCB_HASHSZ];
struct udpstat udpstat;

/*
 * UDP protocol implementation.
 * Per RFC 768, August, 1980.
 */
void
udp_init()
{
	int i;

	udb.inp_next = udb.inp_prev = &udb;
	INPCBRC_LOCKINIT(&udb);         /* in_pcbnotify frobs */
        udb.inp_refcnt = 2;
	lock_alloc(&(udb.inp_lock), LOCK_ALLOC_PIN, UDBHEAD_LOCK_FAMILY, -1);
        INHEAD_LOCKINIT(&udb);
        NETSTAT_LOCKINIT(&udpstat.udps_lock);

	/* Initialize the pcb hash table */
	for (i = 0; i < INPCB_HASHSZ; i++) {
		udp_pcb_hash_table[i].head.next =
			udp_pcb_hash_table[i].head.prev =
			&(udp_pcb_hash_table[i].head);
		udp_pcb_hash_table[i].cache = 0;
#ifdef	DEBUG
		udp_pcb_hash_table[i].qlen = 0;
		udp_pcb_hash_table[i].maxq = 0;
		udp_pcb_hash_table[i].cachehit = 0;
		udp_pcb_hash_table[i].cachemiss = 0;
#endif	/* DEBUG */
	}

}

int	udpcksum = UDPCKSUM;
int	udpcksum_dflt = UDPCKSUM;

int	udp_ttl = UDP_TTL;
int	udp_ttl_dflt = UDP_TTL;

#if     !NETISR_THREAD
struct  sockaddr_in udp_in = { sizeof(udp_in), AF_INET };
#endif

void
udp_input(m, iphlen)
	register struct mbuf *m;
	int iphlen;
{
	register struct ip *ip;
#ifdef IP_MULTICAST
        register struct udpiphdr *ui;
#endif
	register struct udphdr *uh;
	register struct inpcb *inp;
	register struct socket *so;
	struct mbuf *opts = 0;
	int len, index;
	struct ip save_ip;
#if     NETISR_THREAD
        struct sockaddr_in udp_in;
#endif
	NETSTAT_LOCK_DECL()
	INHEAD_LOCK_DECL()

        NETSTAT_LOCK(&udpstat.udps_lock);
        udpstat.udps_ipackets++;
        NETSTAT_UNLOCK(&udpstat.udps_lock);
	/*
	 * Strip IP options, if any; should skip this,
	 * make available to user, and use on returned packets,
	 * but we don't yet have a way to check the checksum
	 * with options still present.
	 */
	if (iphlen > sizeof (struct ip)) {
		ip_stripoptions(m, (struct mbuf *)0);
		iphlen = sizeof(struct ip);
	}

	/*
	 * Get IP and UDP header together in first mbuf.
	 */
	ip = mtod(m, struct ip *);
	if (m->m_len < iphlen + sizeof(struct udphdr)) {
		if ((m = m_pullup(m, iphlen + sizeof(struct udphdr))) == 0) {
                        NETSTAT_LOCK(&udpstat.udps_lock);
                        udpstat.udps_hdrops++;
                        NETSTAT_UNLOCK(&udpstat.udps_lock);
			return;
		}
		ip = mtod(m, struct ip *);
	}
#ifdef IP_MULTICAST
        ui = (struct udpiphdr *) ip;
#endif
	uh = (struct udphdr *)((caddr_t)ip + iphlen);

	/*
	 * Make mbuf data length reflect UDP length.
	 * If not enough data to reflect UDP length, drop.
	 */
	len = ntohs((u_short)uh->uh_ulen);
	if ((int)ip->ip_len != len) {
		if (len > (int)ip->ip_len) {
                        NETSTAT_LOCK(&udpstat.udps_lock);
                        udpstat.udps_badlen++;
                        NETSTAT_UNLOCK(&udpstat.udps_lock);
			goto bad;
		}
		m_adj(m, len - (int)ip->ip_len);
		/* ip->ip_len = len; */
	}
	/*
	 * Save a copy of the IP header in case we want restore it
	 * for sending an ICMP error message in response.
	 */
	save_ip = *ip;

#ifdef IP_MULTICAST
	if (IN_MULTICAST(ntohl(ui->ui_dst.s_addr)) ||
	    in_broadcast(ui->ui_dst)) {
		struct mbuf *hitlist = NULL;

		/*
		 * Checksum extended UDP header and data.
		 */
		if (udpcksum && uh->uh_sum && m->m_pkthdr.rcvif != &loif) {
			((struct ipovly *)ip)->ih_next = 0;
			((struct ipovly *)ip)->ih_prev = 0;
			((struct ipovly *)ip)->ih_x1 = 0;
			((struct ipovly *)ip)->ih_len = uh->uh_ulen;
			if (uh->uh_sum = in_cksum(m, len + sizeof(struct ip))) {
				NETSTAT_LOCK(&udpstat.udps_lock);
				udpstat.udps_badsum++;
				NETSTAT_UNLOCK(&udpstat.udps_lock);
				goto bad;
			}
		} else
			uh->uh_sum = 0;

		/*
		 * Deliver a multicast or broadcast datagram to *all* sockets
		 * for which the local and remote addresses and ports match
		 * those of the incoming datagram.  This allows more than
		 * one process to receive multi/broadcasts on the same port.
		 * (This really ought to be done for unicast datagrams as
		 * well, but that would cause problems with existing
		 * applications that open both address-specific sockets and
		 * a wildcard socket listening to the same port -- they would
		 * end up receiving duplicates of every unicast datagram.
		 * Those applications open the multiple sockets to overcome an
		 * inadequacy of the UDP socket interface, but for backwards
		 * compatibility we avoid the problem here rather than
		 * fixing the interface.  Maybe 4.4BSD will remedy this?)
		 */

		/*
		 * Construct sockaddr format source address.
		 */
#if     NETISR_THREAD
		udp_in = in_zeroaddr;
#endif
		udp_in.sin_port = ui->ui_sport;
		udp_in.sin_addr = ui->ui_src;
		
		/*
		 * Locate pcb(s) for datagram.
		 * Basically, lock the pcb head, then create a chain
		 * of all pcb's that match (reference them too).  
		 * Then unlock the head and sbappend the packet to each
		 * socket.  Note that we create this chain by using the
		 * nextpkt field of the mbuf that is created by the mcopy... 
		 */
		INHEAD_READ_LOCK(&udb);
		for (inp = udb.inp_next; inp != &udb; inp = inp->inp_next) {
			struct mbuf *n;

			if (inp->inp_lport != ui->ui_dport) {
				continue;
			}
			if (inp->inp_laddr.s_addr != INADDR_ANY) {
				if (inp->inp_laddr.s_addr !=
					ui->ui_dst.s_addr) {
					continue;
				}
			}
			if (inp->inp_faddr.s_addr != INADDR_ANY) {
				if (inp->inp_faddr.s_addr !=
					ui->ui_src.s_addr ||
				    inp->inp_fport != ui->ui_sport) {
					continue;
				}
			}

			if ((n = m_copy(m, 0, M_COPYALL)) != NULL) {
				n->m_nextpkt = hitlist;
				hitlist = n;
				n->m_pkthdr.rcvif = (struct ifnet *)inp;
				n->m_len -= sizeof (struct udpiphdr);
				n->m_pkthdr.len -= sizeof (struct udpiphdr);
				n->m_data += sizeof (struct udpiphdr);
				INPCBRC_REF(inp);
			} else {
				break;
			}

			/*
			 * Don't look for additional 
			 * matches if this one
			 * does not have the SO_REUSEADDR 
			 * socket option set.
			 * This heuristic avoids searching 
			 * through all pcbs
			 * in the common case of a non-shared port.  It
			 * assumes that an application will never clear
			 * the SO_REUSEADDR option after setting it.
			 * It is assumed that reading the socket is ok 
			 * here since we haved referenced the inpcb.  The 
			 * socket will NOT go away while the pcb is 
			 * referenced...
			 */
			if ((inp->inp_socket->so_options & SO_REUSEADDR) == 0) {
				break;
			}
		}
		INHEAD_READ_UNLOCK(&udb);

		if (!hitlist) {
                        NETSTAT_LOCK(&udpstat.udps_lock);
                        udpstat.udps_noportbcast++;
                        NETSTAT_UNLOCK(&udpstat.udps_lock);

			/* don't send ICMP response for broadcast packet */
			goto bad;
		}

		/*
		 * Now, hitlist points to  a list of mbufs with a copy
		 * of the data to be delivered, and the rcvif ptr in the
		 * mbuf header contains the pcb of the socket we want to
		 * sbappend() on...
		 */
		while (hitlist) {
			struct mbuf *n;
			struct socket *so;

			n = hitlist;
			hitlist = hitlist->m_nextpkt;
			n->m_nextpkt = NULL;

			so = ((struct inpcb *)(n->m_pkthdr.rcvif))->inp_socket;
			SOCKET_LOCK(so);
			SOCKBUF_LOCK(&so->so_rcv);
			if (sbappendaddr(&so->so_rcv,
				(struct sockaddr *)&udp_in,
			        n, (struct mbuf *)0) == 0) {
				SOCKBUF_UNLOCK(&so->so_rcv);
				INPCBRC_UNREF(((struct inpcb *)
					(n->m_pkthdr.rcvif)));
				m_freem(n);
			} else {
				SOCKBUF_UNLOCK(&so->so_rcv);
				sorwakeup(so);
				INPCBRC_UNREF(((struct inpcb *)
					(n->m_pkthdr.rcvif)));
			}
			SOCKET_UNLOCK(so);
		}

		/*
		 * We've delivered to all sockets via m_copym() so
		 * free the mbuf and opts...
		 */
		m_freem(m);
		if (opts)
			m_freem(opts);
		return;
	}
#endif /* IP_MULTICAST */

	/*
	 * Locate pcb for datagram.
	 */
	index = INPCB_UDPHASH(uh->uh_dport);
	INHEAD_READ_LOCK(&udb);
	inp = in_pcbhashlookup(&(udp_pcb_hash_table[index]), ip->ip_src, 
		uh->uh_sport, ip->ip_dst, uh->uh_dport, 
		INPLOOKUP_USECACHE|INPLOOKUP_WILDCARD);
	INHEAD_READ_UNLOCK(&udb);

	if (inp == 0) {
                NETSTAT_LOCK(&udpstat.udps_lock);
                udpstat.udps_noport++;
                NETSTAT_UNLOCK(&udpstat.udps_lock);
		*ip = save_ip;
		ip->ip_len += iphlen;
		icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PORT, zeroin_addr);
		return;
	}

        so = inp->inp_socket;
        SOCKET_LOCK(so);
        INPCBRC_UNREF(inp);     /* Not necessary to INPCB_LOCK(inp); */

	/*
	 * Only checksum if the user has not turned on SO_CKSUMRECV.
	 */
	if ((so->so_options & SO_CKSUMRECV) == 0) {
		if (udpcksum && uh->uh_sum && m->m_pkthdr.rcvif != &loif) {
			((struct ipovly *)ip)->ih_next = 0;
			((struct ipovly *)ip)->ih_prev = 0;
			((struct ipovly *)ip)->ih_x1 = 0;
			((struct ipovly *)ip)->ih_len = uh->uh_ulen;
			if (uh->uh_sum = in_cksum(m, len + sizeof(struct ip))) {
				NETSTAT_LOCK(&udpstat.udps_lock);
				udpstat.udps_badsum++;
				NETSTAT_UNLOCK(&udpstat.udps_lock);
				SOCKET_UNLOCK(so);
				goto bad;
			}
		} else
			uh->uh_sum = 0;
	}

	/*
	 * Construct sockaddr format source address.
	 * Stuff source address and datagram in user buffer.
	 */
#if     NETISR_THREAD
        udp_in = in_zeroaddr;
#endif
	udp_in.sin_port = uh->uh_sport;
	udp_in.sin_addr = ip->ip_src;
	if (inp->inp_flags & INP_CONTROLOPTS) {
		struct mbuf **mp = &opts;

		if (inp->inp_flags & INP_RECVDSTADDR) {
			*mp = udp_saveopt((caddr_t) &ip->ip_dst,
			    sizeof(struct in_addr), IP_RECVDSTADDR);
			if (*mp)
				mp = &(*mp)->m_next;
		}
#ifdef notyet
		/* options were tossed above */
		if (inp->inp_flags & INP_RECVOPTS) {
			*mp = udp_saveopt((caddr_t) opts_deleted_above,
			    sizeof(struct in_addr), IP_RECVOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
		/* ip_srcroute doesn't do what we want here, need to fix */
		if (inp->inp_flags & INP_RECVRETOPTS) {
			*mp = udp_saveopt((caddr_t) ip_srcroute(),
			    sizeof(struct in_addr), IP_RECVRETOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
#endif
	}

	iphlen += sizeof(struct udphdr);
	m->m_len -= iphlen;
	m->m_pkthdr.len -= iphlen;
	m->m_data += iphlen;
        if (so->so_state & SS_CANTRCVMORE) {
                SOCKET_UNLOCK(so);
                goto bad;
        }
        SOCKBUF_LOCK(&so->so_rcv);
	if (sbappendaddr(&so->so_rcv, (struct sockaddr *)&udp_in, m, opts)) {
                if (so->so_rcv.sb_flags & SB_NOTIFY) {
                        SOCKBUF_UNLOCK(&so->so_rcv);
                        sorwakeup(so);
                } else
                        SOCKBUF_UNLOCK(&so->so_rcv);
                SOCKET_UNLOCK(so);
                return;
	}
        SOCKBUF_UNLOCK(&so->so_rcv);
        SOCKET_UNLOCK(so);
        NETSTAT_LOCK(&udpstat.udps_lock);
        udpstat.udps_fullsock++;
        NETSTAT_UNLOCK(&udpstat.udps_lock);
bad:
	m_freem(m);
	if (opts)
		m_freem(opts);
}

/*
 * Create a "control" mbuf containing the specified data
 * with the specified type for presentation with a datagram.
 */
struct mbuf *
udp_saveopt(p, size, type)
	caddr_t p;
	register int size;
	int type;
{
	register struct cmsghdr *cp;
	struct mbuf *m;

	if ((m = m_get(M_DONTWAIT, MT_CONTROL)) == NULL)
		return ((struct mbuf *) NULL);
	cp = (struct cmsghdr *) mtod(m, struct cmsghdr *);
	bcopy(p, (caddr_t)(cp + 1), size);
	size += sizeof(*cp);
	m->m_len = size;
	cp->cmsg_len = size;
	cp->cmsg_level = IPPROTO_IP;
	cp->cmsg_type = type;
	return (m);
}

/*
 * Notify a udp user of an asynchronous error;
 * just wake up so that it can collect error status.
 */
void
udp_notify(inp, errno)
	register struct inpcb *inp;
{
        register struct socket *so = inp->inp_socket;

        LOCK_ASSERT("udp_notify so", SOCKET_ISLOCKED(so));
	so->so_error = errno;
	sorwakeup(so);
	sowwakeup(so);
}

void
udp_ctlinput(cmd, sa, ip)
	int cmd;
	struct sockaddr *sa;
	register caddr_t ip;
{
	register struct udphdr *uh;
	extern CONST struct in_addr zeroin_addr;
	extern CONST u_char inetctlerrmap[];

	switch (cmd) {
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		break;

	/*
	 * If the an interface is attached, then we must add all filters
	 * for the inet domain...
 	 */
	case PRC_IFATTACH:
		(void)in_ifattach((struct ifnet *)sa);
		return;
	
	default:
		if ((unsigned)cmd > PRC_NCMDS || inetctlerrmap[cmd] == 0) 
			return;
	}
        if (ip) {
                uh = (struct udphdr *)(ip +
                                ((((struct ip *)ip)->ip_vhl&0x0f) << 2));
                in_pcbnotify(&udb, sa, uh->uh_dport,
                    ((struct ip *)ip)->ip_src, uh->uh_sport, cmd, udp_notify);
        } else
                in_pcbnotify(&udb, sa, 0, zeroin_addr, 0, cmd, udp_notify);
}


udp_output(inp, m, addr, control)
	register struct inpcb *inp;
	register struct mbuf *m;
	struct mbuf *addr, *control;
{
	register struct udpiphdr *ui;
	register int len = m->m_pkthdr.len;
	struct in_addr laddr;
	int error = 0;
	NETSTAT_LOCK_DECL()
	INHEAD_LOCK_DECL()

	LOCK_ASSERT("udp_output", INPCB_ISLOCKED(inp));
	if (control)
		m_freem(control);		/* XXX */

	/* check len since sb_max can be set to something greater than 64k */
	if (len >= 65536 - sizeof(struct udpiphdr)) {
		error = EMSGSIZE;
		goto release;
	}

	if (addr) {
		struct in_ifaddr *ia;
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			goto release;
		}
		laddr = inp->inp_laddr;
                ia = (m->m_pkthdr.rcvif ? ifptoia(m->m_pkthdr.rcvif) : 0);
		/*
		 * Must block input while temporarily connected.
		 */
                INHEAD_WRITE_LOCK(&udb);
                if (ia)
                        inp->inp_laddr = IA_SIN(ia)->sin_addr;
		error = in_pcbconnect_nolock(inp, addr);
		if (error) {
                        inp->inp_laddr = laddr;
                        INHEAD_WRITE_UNLOCK(&udb);
			goto release;
		}
	} else {
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			goto release;
		}
	}
	if (rfc1122addrchk && !localaddr_notcast(inp->inp_laddr.s_addr)) {
		error = EINVAL;
		if (addr) {
			in_pcbdisconnect_nolock(inp);
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&udb);
		}
		goto release;
	}
	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */
	M_PREPEND(m, sizeof(struct udpiphdr), M_DONTWAIT);
	if (m == NULL) {
		error=ENOBUFS;
		if (addr) {
			in_pcbdisconnect_nolock(inp);
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&udb);
		}
		goto release;
	}

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	ui = mtod(m, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)len + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;

	/*
	 * Stuff checksum and output datagram.
	 */
	ui->ui_sum = 0;
	if (udpcksum && (ui->ui_dst.s_addr != INADDR_LOOPBACK) ) {
	    if ((ui->ui_sum = in_cksum(m, sizeof (struct udpiphdr) + len)) == 0)
		ui->ui_sum = 0xffff;
	}
	((struct ip *)ui)->ip_len = sizeof (struct udpiphdr) + len;
	((struct ip *)ui)->ip_ttl = inp->inp_ip.ip_ttl;	/* XXX */
	((struct ip *)ui)->ip_tos = inp->inp_ip.ip_tos;	/* XXX */
	NETSTAT_LOCK(&udpstat.udps_lock);
	udpstat.udps_opackets++;
	NETSTAT_UNLOCK(&udpstat.udps_lock);
#ifdef IP_MULTICAST
        error = ip_output(m, inp->inp_options, &inp->inp_route,
            inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST) | 
			IP_MULTICASTOPTS, inp->inp_moptions);
#else
        error = ip_output(m, inp->inp_options, &inp->inp_route,
            inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST));
#endif /* IP_MULTICAST */

	if (addr) {
		in_pcbdisconnect_nolock(inp);
		inp->inp_laddr = laddr;
		INHEAD_WRITE_UNLOCK(&udb);
	}
	return (error);

release:
	m_freem(m);
	return (error);
}

u_long	udp_sendspace = 9216;		/* really max datagram size */
u_long	udp_recvspace = 40 * (1024 + sizeof(struct sockaddr_in));
					/* 40 1K datagrams */
u_long	udp_sendspace_dflt = 9216;		/* really max datagram size */
u_long	udp_recvspace_dflt = 40 * (1024 + sizeof(struct sockaddr_in));

/*ARGSUSED*/
udp_usrreq(so, req, m, addr, control)
	struct socket *so;
	int req;
	struct mbuf *m, *addr, *control;
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;

	LOCK_ASSERT("udp_usrreq", SOCKET_ISLOCKED(so));

	if (req == PRU_CONTROL)
		return (in_control(so, (int)m, (caddr_t)addr,
			(struct ifnet *)control));
#if     !SEC_ARCH
        if (control && control->m_len) {
                error = EINVAL;
                goto release;
        }
#endif
        if (inp) {
                INPCB_LOCK(inp);
        } else if (req != PRU_ATTACH) {
                error = EINVAL;
                goto release;
        }

	switch (req) {

	case PRU_ATTACH:
		if (inp != NULL) {
			error = EINVAL;
			break;
		}
		error = soreserve(so, udp_sendspace, udp_recvspace);
		if (error)
			break;
		error = in_pcballoc(so, &udb);
		if (error)
			break;
		((struct inpcb *) so->so_pcb)->inp_ip.ip_ttl = udp_ttl;
		break;

	case PRU_DETACH:
#ifdef IP_MULTICAST
		SOCKET_UNLOCK(so);
		ip_freemoptions(inp->inp_moptions);
		SOCKET_LOCK(so);
#endif /* IP_MULTICAST */
		in_pcbdetach(inp);
		inp = 0;
		break;

	case PRU_BIND:
		error = in_pcbbind(inp, addr);
		break;

	case PRU_LISTEN:
		error = EOPNOTSUPP;
		break;

	case PRU_CONNECT:
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		error = in_pcbconnect(inp, addr);
		if (error == 0)
			soisconnected(so);
		break;

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		break;

	case PRU_ACCEPT:
		error = EOPNOTSUPP;
		break;

	case PRU_DISCONNECT:
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		in_pcbdisconnect(inp);
#ifdef notdef
		/* 
		 * in_pcbdisconnect dereferences the pcb which could
	         * cause it to be freed.  DO NOT REFERENCE IT HERE!!!
		 * Aoot!
		 */
		inp->inp_laddr.s_addr = INADDR_ANY;
#endif
		so->so_state &= ~SS_ISCONNECTED;		/* XXX */
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	case PRU_SEND:
                error = udp_output(inp, m, addr, control);
                m = 0;
                control = 0;
                break;

	case PRU_ABORT:
#ifdef IP_MULTICAST
		SOCKET_UNLOCK(so);
		ip_freemoptions(inp->inp_moptions);
		SOCKET_LOCK(so);
#endif /* IP_MULTICAST */
		soisdisconnected(so);
		in_pcbdetach(inp);
                inp = 0;
		break;

	case PRU_SOCKADDR:
		in_setsockaddr(inp, addr);
		break;

	case PRU_PEERADDR:
		in_setpeeraddr(inp, addr);
		break;

	case PRU_SENSE:
		/*
		 * stat: don't bother with a blocksize.
		 */
		INPCB_UNLOCK(inp);
		return (0);

	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:
		error =  EOPNOTSUPP;
		break;

	case PRU_RCVD:
	case PRU_RCVOOB:
		INPCB_UNLOCK(inp);
		return (EOPNOTSUPP);	/* do not free mbuf's */

	default:
		panic("udp_usrreq");
	}
        if (inp)
                INPCB_UNLOCK(inp);
release:
	if (control != NULL)
		m_freem(control);
	if (m && (req != PRU_RCVOOB))
		m_freem(m);
	return (error);
}

udp_send(so, addr, uio, top, control, flags)
	register struct socket *so;
	struct mbuf *addr;
	struct uio *uio;
	struct mbuf *top;
	struct mbuf *control;
	int flags;
{
	register struct mbuf **mp, *m;
	register long space, resid;
	register struct udpiphdr *ui;
	int error=0, dontroute;
	struct inpcb *inp;
	u_int movesum;
	register u_int sum=0;
        struct in_addr laddr;
	int badsum=0;
	int len;
        NETSTAT_LOCK_DECL()
        INHEAD_LOCK_DECL()

	assert(top == NULL);

	resid = uio->uio_resid;

	/* Sanity check length... */
	if ((uint)resid >= 65536 - sizeof(struct udpiphdr))
		return(EMSGSIZE);
	if (resid == 0)
		return(0); /* EHOSEDUSER */

	SOCKET_LOCK(so);

	dontroute =
	    (flags & MSG_DONTROUTE) && (so->so_options & SO_DONTROUTE) == 0;

	if (control)
		m_freem(control);

#define	snderr(errno)	{ error = errno; goto release; }

	if (resid > so->so_snd.sb_hiwat)
		snderr(EMSGSIZE);
	SOCKET_UNLOCK(so);
	space = resid;
	top = 0;
	mp = &top;
	do {
		if (resid > MHLEN) {
			len = MIN(resid, MAXALLOCSAVE);
			m = m_getclustm(M_WAIT, MT_DATA, len);
			if (top == 0) {
				m->m_flags |= M_PKTHDR;
				if ((len == resid) && 
					((len+max_hdr) <= m->m_ext.ext_size))
					m->m_data += max_hdr;
			}
		} else {
			MGETHDR(m, M_WAIT, MT_DATA);
			len = MIN(MHLEN, resid);
			if (resid < MHLEN)
				MH_ALIGN(m, resid);
		}

		*mp = m;
		error = uiomove_chksum(mtod(m, caddr_t),
			len, UIO_WRITE, uio, &movesum);

		if (movesum & 0xFFFF0000)
			badsum++;
		if (!badsum) {
			sum += movesum;
			sum += sum >> 16;
			sum &= 0xFFFF;
		}

		resid = uio->uio_resid;
		m->m_len = len;
		if (error) {
			SOCKET_LOCK(so);
			goto release;
		}
		mp = &m->m_next;
	} while (resid);

	SOCKET_LOCK(so);
	top->m_pkthdr.len = space;
	top->m_pkthdr.rcvif = (struct ifnet *)0;

	if (dontroute)
	    so->so_options |= SO_DONTROUTE;

	inp = sotoinpcb(so);
	if (!inp)
		snderr(EINVAL);

	INPCB_LOCK(inp);
	if (addr) {
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			goto release;
		}
		laddr = inp->inp_laddr;

		/*
		 * Must block input while temporarily connected.
		 */
		INHEAD_WRITE_LOCK(&udb);
		error = in_pcbconnect_nolock(inp, addr);
		if (error) {
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&udb);
			goto release;
		}
	} else {
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			goto release;
		}
	}

	if (rfc1122addrchk && 
		!localaddr_notcast(inp->inp_laddr.s_addr)) {
		error = EINVAL;
		if (addr) {
			in_pcbdisconnect_nolock(inp);
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&udb);
		}
		goto release;
	}

	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */

	M_PREPEND(top, sizeof(struct udpiphdr), M_DONTWAIT);
	if (top == NULL) {
		error=ENOBUFS;
		if (addr) {
			in_pcbdisconnect_nolock(inp);
			inp->inp_laddr = laddr;
			INHEAD_WRITE_UNLOCK(&udb);
		}
		goto release;
	}

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	ui = mtod(top, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)space + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;

	ui->ui_sum = 0;
	/*
	 * Stuff checksum and output datagram.
	 */
	if (udpcksum && (ui->ui_dst.s_addr != INADDR_LOOPBACK) ) {
		register u_int hdrsum;

		/*
		 * If checksum worked on the uiomove, then use it, else
		 * rechecksum...
		 */
		if (badsum) {
			sum = (u_int)in_cksum(top, top->m_pkthdr.len);
		} else {

			hdrsum =  *(u_short *)((caddr_t)ui);
			hdrsum +=  *(u_short *)((caddr_t)ui + 2);
			hdrsum +=  *(u_short *)((caddr_t)ui + 4);
			hdrsum +=  *(u_short *)((caddr_t)ui + 6);
			hdrsum +=  *(u_short *)((caddr_t)ui + 8);
			hdrsum +=  *(u_short *)((caddr_t)ui + 10);
			hdrsum +=  *(u_short *)((caddr_t)ui + 12);
			hdrsum +=  *(u_short *)((caddr_t)ui + 14);
			hdrsum +=  *(u_short *)((caddr_t)ui + 16);
			hdrsum +=  *(u_short *)((caddr_t)ui + 18);
			hdrsum +=  *(u_short *)((caddr_t)ui + 20);
			hdrsum +=  *(u_short *)((caddr_t)ui + 22);
			hdrsum +=  *(u_short *)((caddr_t)ui + 24);
			hdrsum +=  *(u_short *)((caddr_t)ui + 26);

			/* Fold it */
			hdrsum = (hdrsum & 0xffff) + (hdrsum >> 16);

			/* Deal with carry */
			hdrsum += hdrsum >> 16;

			/* One's complement */
			hdrsum = (~hdrsum & 0xffff);

			sum = sum + hdrsum;
			sum += sum >> 16;
			sum = sum & 0xFFFF;
		}
		if (sum == 0)
			sum = 0xffff;
		ui->ui_sum = (u_short)sum;
	}

	((struct ip *)ui)->ip_len = top->m_pkthdr.len;
	((struct ip *)ui)->ip_ttl = inp->inp_ip.ip_ttl;	/* XXX */
	((struct ip *)ui)->ip_tos = inp->inp_ip.ip_tos;	/* XXX */
	NETSTAT_LOCK(&udpstat.udps_lock);
	udpstat.udps_opackets++;
	NETSTAT_UNLOCK(&udpstat.udps_lock);
	error = ip_output(top, inp->inp_options, &inp->inp_route,
		inp->inp_socket->so_options & 
		(SO_DONTROUTE|SO_BROADCAST) | IP_MULTICASTOPTS, 
		inp->inp_moptions);

	if (addr) {
		in_pcbdisconnect_nolock(inp);
		inp->inp_laddr = laddr;
		INHEAD_WRITE_UNLOCK(&udb);
	}

	INPCB_UNLOCK(inp);
	top = 0;

release:
	if (dontroute)
	    so->so_options &= ~SO_DONTROUTE;

	if (top)
		m_freem(top);
out:
	SOCKET_UNLOCK(so);
	return (error);
}

/*
 * Drop a record off the front of a sockbuf
 * and move the next record to the front.  This function adds the mbufs to
 * be freed to *freehead so the caller can free them outside the socket lock.
 */
void
delay_sbdroprecord(sb, freehead)
        register struct sockbuf *sb;
	struct mbuf **freehead;
{
        register struct mbuf *m, *mn;

        LOCK_ASSERT("sbdroprecord", SOCKBUF_ISLOCKED(sb));
        m = sb->sb_mb;
        if (m) {
                sb->sb_mb = m->m_nextpkt;
                do {
			register struct mbuf *tmp;

                        sbfree(sb, m);
			mn = m->m_next;
        		tmp = *freehead;
			*freehead = m;
			(*freehead)->m_next = tmp;
			(*freehead)->m_nextpkt = (struct mbuf *)NULL;
                } while (m = mn);
        }
}

udp_receive(so, paddr, uio, mp0, controlp, flagsp)
	register struct socket *so;
	struct mbuf **paddr;
	struct uio *uio;
	struct mbuf **mp0;
	struct mbuf **controlp;
	int *flagsp;
{
	register struct mbuf *m;
	register int flags, len, error;
	struct mbuf *nextrecord;
	register struct ip *ip;
	register struct udphdr *uh;
	int moff;
	register u_int movesum;
	int	org_resid = uio->uio_resid;
	struct mbuf *freehead=0;

/*
 * DELAY_MFREE - macro to delay freeing mbufs until after the socket lock
 * is released.  Reduces contentin on socket lock.
 */
#define	DELAY_MFREE(m, sb, freehead) \
{ \
	register struct mbuf *tmp; \
	sb = m->m_next; \
	tmp = freehead; \
	freehead = m; \
	freehead->m_next = tmp; \
	freehead->m_nextpkt = (struct mbuf *)NULL; \
}


	assert(mp0 == NULL);
	if (paddr)
		*paddr = 0;
	if (controlp)
		*controlp = 0;
	if (flagsp)
		flags = *flagsp &~ MSG_EOR;
	else 
		flags = 0;
	if (flags & MSG_OOB)
		return(EOPNOTSUPP);
	SOCKET_LOCK(so);

restart:
	if (error = sosblock(&so->so_rcv, so))
		goto out;
	m = so->so_rcv.sb_mb;
	/*
	 * If we have less data than requested, block awaiting more
	 * (subject to any timeout) if:
	 *   1. the current count is less than the low water mark, or
	 *   2. MSG_WAITALL is set, and it is possible to do the entire
	 *	receive operation at once if we block (resid <= hiwat).
	 * If MSG_WAITALL is set but resid is larger than the receive buffer,
	 * we have to do the receive in sections, and thus risk returning
	 * a short count if a timeout or signal occurs after we start.
	 */
	while (m == 0 || so->so_rcv.sb_cc < uio->uio_resid && 
	    (so->so_rcv.sb_cc < so->so_rcv.sb_lowat ||
	    ((flags & MSG_WAITALL) && uio->uio_resid <= so->so_rcv.sb_hiwat)) &&
	    m->m_nextpkt == 0) {
#ifdef DIAGNOSTIC
		if (m == 0 && so->so_rcv.sb_cc)
			panic("receive 1");
#endif
		if (so->so_error) {
			if (m)
				break;
			error = so->so_error;
			if ((flags & MSG_PEEK) == 0)
				so->so_error = 0;
			goto release;
		}
		if (so->so_state & SS_CANTRCVMORE) {
			if (m)
				break;
			goto release;
		}
		if (uio->uio_resid == 0)
			goto release;
		if ((so->so_state & SS_NBIO) || (flags & MSG_NONBLOCK)) {
			error = EWOULDBLOCK;
			goto release;
		}
		if (error = sosbwait(&so->so_rcv, so))
			goto out;
		goto restart;
	}
	nextrecord = m->m_nextpkt;

#ifdef DIAGNOSTIC
	if (m->m_type != MT_SONAME)
		panic("receive 1a");
#endif
	if (flags & MSG_PEEK) {
		if (paddr)
			*paddr = m_copym(m, 0, m->m_len, M_DONTWAIT);
		if (*paddr == (struct mbuf *) NULL) {
			error = ENOBUFS;
			goto release;
		}
		m = m->m_next;
	} else {
		sbfree(&so->so_rcv, m);
		if (paddr) {
			*paddr = m;
			so->so_rcv.sb_mb = m->m_next;
			m->m_next = 0;
			m = so->so_rcv.sb_mb;
		} else {
			DELAY_MFREE(m, so->so_rcv.sb_mb, freehead);
			m = so->so_rcv.sb_mb;
		}
	}

	if (m) {
		if ((flags & MSG_PEEK) == 0)
			m->m_nextpkt = nextrecord;
	}

	ip = (struct ip *)(mtod(m, caddr_t) - sizeof(struct udpiphdr));
	uh = (struct udphdr *)((caddr_t)ip + sizeof(struct ip));
	if (so->so_options & SO_CKSUMRECV) {
		if (udpcksum && uh->uh_sum && (m->m_pkthdr.rcvif != &loif)) {
			register u_int	sum;

			((struct ipovly *)ip)->ih_next = 0;
			((struct ipovly *)ip)->ih_prev = 0;
			((struct ipovly *)ip)->ih_x1 = 0;
			((struct ipovly *)ip)->ih_len = uh->uh_ulen;

			sum =  *(u_short *)((caddr_t)ip);
			sum +=  *(u_short *)((caddr_t)ip + 2);
			sum +=  *(u_short *)((caddr_t)ip + 4);
			sum +=  *(u_short *)((caddr_t)ip + 6);
			sum +=  *(u_short *)((caddr_t)ip + 8);
			sum +=  *(u_short *)((caddr_t)ip + 10);
			sum +=  *(u_short *)((caddr_t)ip + 12);
			sum +=  *(u_short *)((caddr_t)ip + 14);
			sum +=  *(u_short *)((caddr_t)ip + 16);
			sum +=  *(u_short *)((caddr_t)ip + 18);
			sum +=  *(u_short *)((caddr_t)ip + 20);
			sum +=  *(u_short *)((caddr_t)ip + 22);
			sum +=  *(u_short *)((caddr_t)ip + 24);
			sum +=  *(u_short *)((caddr_t)ip + 26);

			sum = (sum & 0xffff) + (sum >> 16);
			sum += sum >> 16;
			sum = (~sum & 0xffff);

			/*
			 * If everything is cule, then let's do a fast path...
			 * checksumming the user data whilest we move it into 
			 * user space!
			 */
			if ((uio->uio_resid >= m->m_pkthdr.len) && 
				((flags & MSG_PEEK) == 0) && 
				(uio->uio_iovcnt == 1)) {
				error = udp_cksum_and_move(m, uio, sum, so);
				delay_sbdroprecord(&so->so_rcv, &freehead);
				goto release;
			}

			sum += (u_int)in_cksum(m, ntohs((u_short)uh->uh_ulen)
				- sizeof(struct udphdr));
			sum += sum >> 16;
			sum = (~sum & 0xffff);

			if (sum) {
				NETSTAT_LOCK(&udpstat.udps_lock);
				udpstat.udps_badsum++;
				NETSTAT_UNLOCK(&udpstat.udps_lock);
				error = EAGAIN; /* XXX */
				delay_sbdroprecord(&so->so_rcv, &freehead);
				goto release;
			}
		}
	}

	moff = 0;
	while (m && uio->uio_resid > 0 && error == 0) {
		len = uio->uio_resid;
		if (len > m->m_len - moff)
			len = m->m_len - moff;
		/*
		 * Sockbuf must be consistent here (points to current mbuf,
		 * it points to next record) when we drop priority;
		 * we must note any additions to the sockbuf when we
		 * block interrupts again.
		 */
		SOCKET_UNLOCK(so);
		if (len > 0)
			error = uiomove(mtod(m, caddr_t) + moff, 
				(int)len, UIO_READ, uio);

		SOCKET_LOCK(so);
		if (len == m->m_len - moff) {
			if (flags & MSG_PEEK) {
				m = m->m_next;
				moff = 0;
			} else {
				nextrecord = m->m_nextpkt;
				sbfree(&so->so_rcv, m);
				DELAY_MFREE(m, so->so_rcv.sb_mb, freehead);
				m = so->so_rcv.sb_mb;
				if (m)
					m->m_nextpkt = nextrecord;
			}
		} else {
			if (flags & MSG_PEEK)
				moff += len;
			else {
				m->m_data += len;
				m->m_len -= len;
				so->so_rcv.sb_cc -= len;
			}
		}
	}
	if ((flags & MSG_PEEK) == 0) {
		if (m == 0)
			so->so_rcv.sb_mb = nextrecord;
		else {
			flags |= MSG_TRUNC;
			delay_sbdroprecord(&so->so_rcv, &freehead);
		}
	}
	if (flagsp)
		*flagsp |= flags;
release:
	sbunlock(&so->so_rcv);
out:
	/*
	 * If error == EAGAIN, then the checksum was bad...we must restore the
	 * uio resid count, so that the socket layer returns EAGAIN to the user.
	 */
	if (error == EAGAIN)
		uio->uio_resid = org_resid;
	SOCKET_UNLOCK(so);
	if (freehead)
		m_freem(freehead);
	return (error);
#undef	DELAY_MFREE
}

/*
 * ASSUMES THE SOCKET LOCK IS HELD!!!
 * Move and checksum user data into user's buffer.  If checksum fails, return
 * EAGAIN.  If checksum passes, then the user's got his data!
 */
int
udp_cksum_and_move(m, uio, hdrsum, so)
struct mbuf	*m;
struct uio	*uio;
u_int		hdrsum;					/* UDP header cksum */
struct socket	*so;
{
	register u_int	sum = 0;
	u_int		movesum;
	int		error=0, badsum = 0;
	int		len;
	int		odd=0;

	while (m && (error == 0)) {
		len = uio->uio_resid;
		if (len > m->m_len)
			len = m->m_len;
		
		if (len == 0) {
			m = m->m_next;
			continue;
		}
		SOCKET_UNLOCK(so);
		error = uiomove_chksum(mtod(m, caddr_t), 
			len, UIO_READ, uio, &movesum);
		SOCKET_LOCK(so);

		/* 
		 * If the uiomove checksum is bad, then we'll recalculate the
		 * entire checksum after moving the data...
	 	 */
		if (movesum & 0xffff0000)
			badsum++;
		if (!error && !badsum) {
			if (odd) {
				sum = ((sum & 0xff) <<8) | ((sum & 0xff00) >>8);
				odd = 0;
			}
			if ((u_int)len & 1)
				odd++;
			sum += movesum;
			sum += sum >> 16;
			sum &= 0xFFFF;
		}
		m = m->m_next;
	}
	if (!error) {
		if (badsum) {
			sum = hdrsum + (u_int)in_cksum(so->so_rcv.sb_mb, 
				so->so_rcv.sb_mb->m_pkthdr.len);
		} else {
			sum += hdrsum;
		}
		sum += sum >> 16;
		sum = ~sum & 0xffff;
		if (sum) {
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_badsum++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			error = EAGAIN; 
		}
	}
	return(error);
}

