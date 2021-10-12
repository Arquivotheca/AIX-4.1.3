static char sccsid[] = "@(#)46	1.58  src/bos/kernext/aixif/if_sl.c, sysxaixif, bos411, 9435D411a 9/2/94 14:44:47";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: BCMP
 *		BCOPY
 *		DECODEL
 *		DECODES
 *		DECODEU
 *		ENCODE
 *		ENCODEZ
 *		INCR
 *		INTERACTIVE
 *		config_sl
 *		config_sl_init
 *		config_sl_term
 *		sl_btom
 *		sl_compress_init
 *		sl_compress_tcp
 *		sl_detach
 *		sl_ioctl
 *		sl_output
 *		sl_uncompress_tcp
 *		slattach
 *		sldinit
 *		slinit
 *		which_slp
 *
 *   ORIGINS: 26,27,85
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
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
/* if_sl.c	2.1 16:12:45 4/20/90 SecureWare */
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
 * Copyright (c) 1987, 1989 Regents of the University of California.
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
 *	Base:	if_sl.c	7.20 (Berkeley) 4/5/90
 *	Merged:	if_sl.c	7.21 (Berkeley) 6/28/90
 *
 *	Van Jacobson (van@helios.ee.lbl.gov), Dec 31, 1989:
 *	- Initial distribution.
 *
 *	Base:	slcompress.c	7.5	90/01/20
 *	Base:	slcompress.c,v 1.19 89/12/31 08:52:59 van Exp
 *	Merged:	slcompress.c	7.6	90/06/28
 */

#include <net/net_globals.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <pse/str_lock.h>
#include <sys/nettrace.h>

#include <sys/time.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/netisr.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <aixif/net_if.h>
#include <net/slcompress.h>
#include <net/if_slvar.h>
#include <sys/fp_io.h>		/* fp_open flags 		*/

/*
 * The following disgusting hack gets around the problem that IP TOS
 * can't be set yet.  We want to put "interactive" traffic on a high
 * priority queue.  To decide if traffic is interactive, we check that
 * a) it is TCP and b) one of its ports is telnet, rlogin or ftp control.
 */
CONST static u_short interactive_ports[8] = {
	0,	513,	0,	0,
	0,	21,	0,	23
};

#define INTERACTIVE(p) (interactive_ports[(p) & 7] == (p))

#define MAXQ	150		/* max queue size for send. */

extern void sl_compress_init();

struct sl_softc *sl_softc = (struct sl_softc *)NULL;

/*************************************************************************
 *
 *	which_slp() - find a free slip struct or get a new one 
 *
 ************************************************************************/
struct sl_softc *
which_slp(unit) 
dev_t unit;
{
	struct sl_softc *slp, *p;

	p = NULL;
	for (slp = sl_softc; slp && (slp->sc_if.if_unit != unit); 
             slp = slp->slip_next)
		if (!slp->slip_attached)
			p = slp;

	if (!slp && p)
		slp = p;
		
	if (!slp) {
		slp = xmalloc(sizeof(*slp), 5, pinned_heap);
		if (slp == NULL)
			return(0);
		bzero(slp, sizeof(*slp));
		if (sl_softc) {
			p = sl_softc;
			while (p->slip_next)
				p = p->slip_next;
			p->slip_next = slp;
		}
		else
			sl_softc = slp;
	}
	return(slp);
}

/*************************************************************************
 *
 *      slattach() - logically attach the SLIP network interface
 *
 ************************************************************************/
slattach(unit)
int     unit;
{
	struct sl_softc *sc;
	register struct ifnet   *ifp;
	extern int              sl_output();
	extern int              sl_ioctl();

	NETTRC1(HKWD_IFSL|hkwd_attach_in, unit);

	if (!(sc = which_slp(unit)))
		return ENOMEM;

	if (sc->slip_attached)		/* double attach */
		return EBUSY;

	sc->slip_attached = 1;

	ifp = &sc->sc_if;

	ifp->if_name = "sl";
	ifp->if_unit = unit;
	ifp->if_mtu = SLMTU;
	ifp->if_flags = IFF_POINTOPOINT | IFF_NOTRAILERS;
	ifp->if_type = IFT_SLIP;
	ifp->if_ioctl = sl_ioctl;
	ifp->if_output = sl_output;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
	ifp->if_snd.ifq_maxlen = MAXQ;
	ifp->if_snd.ifq_len = 0;
	ifp->if_snd.ifq_drops = 0;
	ifp->if_snd.ifq_head = 0;
	ifp->if_snd.ifq_tail = 0;
	ifp->if_baudrate = 0;

	if_attach(ifp);
	if_nostat(ifp);
	SLIP_LOCK_INIT(sc);

	NETTRC(HKWD_IFSL|hkwd_attach_out);
	return 0;
}

/*************************************************************************
 *
 *	sl_detach() - logically detach SLIP network interface 
 *
 ************************************************************************/
sl_detach(unit)
int	unit;
{
	struct sl_softc *sc;
	int (*detach)();

	NETTRC1(HKWD_IFSL|hkwd_detach_in, &sc->sc_if);

	/* search the list */
	for (sc = sl_softc; sc && sc->sc_if.if_unit != unit;
		sc = sc->slip_next)
		    ;

	if (!sc) {
		return ENODEV;
	}

	if (detach = sc->sc_detach)
		(*detach)(sc->sc_qptr); /* kill the farm animals */

	sc->slip_attached = 0;

	NETTRC(HKWD_IFSL|hkwd_detach_out);
	return 0;
}

/*************************************************************************
 *
 *	config_sl_init() - SLIP load initialization 
 *
 ************************************************************************/
config_sl_init(unit) 
int unit;
{
	struct sl_softc 	*sc;
	int			rc = 0;

	for (sc = sl_softc; sc && sc->sc_if.if_unit != unit; 
             sc = sc->slip_next)
		;

	if (sc && sc->slip_attached) 
		return(EALREADY); 

	return(slattach(unit));
}

int if_sl_lock = LOCK_AVAIL;

/*************************************************************************
 *
 *	config_sl() - SLIP kernel extension entry point
 *
 ************************************************************************/
config_sl(cmd, uio) 
int		cmd;
struct uio	*uio;
{
	struct device_req 	device;
	int 			error;
	int 			unit;
	int 			lockt;

	lockt = lockl(&if_sl_lock, LOCK_SHORT);

	if ((sl_softc == NULL) && (error = pincode(config_sl)))
		goto out;

	if (cmd != CFG_INIT) {
		error = EINVAL;
		goto out;
	}

	error = uiomove((caddr_t)&device,sizeof(device),UIO_WRITE,uio);
	if (error)
		goto out;
	unit = atoi(&device.dev_name[2]); 
	error = config_sl_init(unit);
out:
	if (lockt != LOCK_NEST)
		unlockl(&if_sl_lock);
	return(error);
}

int
slinit(sc)
	struct sl_softc *sc;
{
	register caddr_t p;
	SLIP_LOCK_DECL()

	MCLALLOC(p, M_WAIT);
	SLIP_LOCK(sc);
	if (p) {
		sc->sc_ep = (u_char *)p + SLBUFSIZE;
		sc->sc_cluster = p;
	} else {
		sc->sc_if.if_flags &= ~(IFF_UP | IFF_RUNNING);
		SLIP_UNLOCK(sc);
		return (ENOMEM);
	}
	sc->sc_buf = sc->sc_ep - SLMAX;
	sc->sc_mp = sc->sc_buf;
	sl_compress_init(&sc->sc_comp);
	SLIP_UNLOCK(sc);
	return (0);
}

void
sldinit(sc)
        struct sl_softc *sc;
{
	SLIP_LOCK_DECL()

	if_down(&sc->sc_if);
	SLIP_LOCK(sc);
	MCLFREE(sc->sc_cluster);
	sc->sc_cluster = 0;
	sc->sc_ep = 0;
	sc->sc_mp = 0;
	sc->sc_buf = 0;
	sc->sc_qptr = NULL;
	sc->sc_if.if_flags &= ~IFF_RUNNING;
	sc->sc_flags = 0;
	sc->sc_output = NULL;
	sc->sc_detach = NULL;
	SLIP_UNLOCK(sc);
}


/*
 * Copy data buffer to mbuf chain; add ifnet pointer.
 */
struct mbuf *
sl_btom(sc, len)
	register struct sl_softc *sc;
	register int len;
{
        register struct mbuf *m;


        MGETHDR(m, M_DONTWAIT, MT_DATA);
        if (m == NULL)
                return (NULL);

        /*
         * If we have more than MHLEN bytes, it's cheaper to
         * queue the cluster we just filled & allocate a new one
         * for the input buffer.  Otherwise, fill the mbuf we
         * allocated above.  Note that code in the input routine
         * guarantees that packet will fit in a cluster.
         */
        if (len > MHLEN) {
                MCLGET(m, M_DONTWAIT);
                if ((m->m_flags & M_EXT) == 0) {
                        /*
                         * we couldn't get a cluster - if memory's this
                         * low, it's time to start dropping packets.
                         */
                        (void) m_free(m);
                        return (NULL);
                }
                sc->sc_ep = mtod(m, u_char *) + SLBUFSIZE;
/* Knows much too much about m_ext mbufs!! */
                m->m_data = (caddr_t)sc->sc_buf;
                { caddr_t p = m->m_ext.ext_buf;
                  m->m_ext.ext_buf = sc->sc_cluster;
                  sc->sc_cluster = p;
                }
        } else
                bcopy((caddr_t)sc->sc_buf, mtod(m, caddr_t), len);

        m->m_len = len;
        m->m_pkthdr.len = len;
        m->m_pkthdr.rcvif = &sc->sc_if;
        return (m);
}

#define ip_XHLEN ip_ff.ip_flen
#define th_XHLEN ip_XHLEN

int
sl_output(ifp, m, dst, rt)
	struct ifnet *ifp;
	struct mbuf *m;
	struct sockaddr *dst;
	struct rtentry	*rt;
{
	register int i;
	register struct sl_softc *sc;
	register int inter;
	register int unit;
	register struct ip *ip;
	int (*output)();
	SLIP_LOCK_DECL()

	if (dst->sa_family != AF_INET) {
		m_freem(m);
		return (EAFNOSUPPORT);
	}

	unit = ifp->if_unit;
	for (sc = sl_softc; sc && sc->sc_if.if_unit != unit;
		sc = sc->slip_next)
		    ;

	if (!sc) {
		m_freem(m);
		return ENETDOWN;
	}

	SLIP_LOCK(sc);

	if ((sc->sc_if.if_flags & (IFF_UP|IFF_RUNNING))!=(IFF_UP|IFF_RUNNING)) {
		m_freem(m);
		SLIP_UNLOCK(sc);
		return ENETDOWN;
	}

	if (sc->sc_qptr == NULL) {
		m_freem(m);
		SLIP_UNLOCK(sc);
		return ENETDOWN;
	}

	inter = 0;
	if ((output = sc->sc_output) == NULL) {
		ifp->if_snd.ifq_drops++;
		m_freem(m);
		SLIP_UNLOCK(sc);
		return ENETDOWN;
	}

	microtime(&sc->sc_if.if_lastchange);
	ifp->if_opackets++;

	if ((ip = mtod(m, struct ip *))->ip_p == IPPROTO_TCP) {
		register int p = ((int *)ip)[ip->ip_XHLEN & 0x0f];
	
		inter =  (INTERACTIVE(p & 0xffff) || INTERACTIVE(p >> 16));
		if (sc->sc_flags & SC_COMPRESS) {

			/*
 			 * The last parameter turns off connection id
			 * compression for background traffic:  Since
			 * fastq traffic can jump ahead of the background
			 * traffic, we don't know what order packets will
			 * go on the line.
			 */
			p = sl_compress_tcp(m, ip, &sc->sc_comp, inter);
			*mtod(m, u_char *) |= p;
		}
	} else if (sc->sc_flags & SC_NOICMP && ip->ip_p == IPPROTO_ICMP) {
		m_freem(m);
		SLIP_UNLOCK(sc);
		return (0);
	}
	
	/* trace the packet */
	net_xmit_trace(ifp, m);

/*
	++sc->slip_if.if_opackets;
	sc->slip_if.if_obytes += m->m_pkthdr.len;
*/
	SLIP_UNLOCK(sc);

	return (*output)(inter, m, sc);
}

int
sl_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	
	struct timestruc_t	ct;
	struct ifaddr *ifa = (struct ifaddr *)data;
	int error = 0;

	if (cmd ==  SIOCIFDETACH) {
		if (error = sl_detach(ifp->if_unit)) 
			bsdlog(LOG_ERR, 
				"if_sl: sl_detach() failed with errno=%d\n", 
				error);
		return(error);
	}

	if (ifa == NULL)
		return(EFAULT);

	switch (cmd) {
		
	   case SIOCSIFADDR:
		switch (ifa->ifa_addr->sa_family) {
		   case AF_INET:
			ifp->if_flags |= IFF_UP;
			curtime (&ct);
			ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
			ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
			break;
		   default:
			error = EAFNOSUPPORT;
			break;
		}
		break;
	   case SIOCSIFDSTADDR:
		switch (ifa->ifa_addr->sa_family) {
		   case AF_INET:
			break;
		   default:
			error = EAFNOSUPPORT;
			break;
		}
		break;
	    default:
		error = EINVAL;
	}
	return(error);
}

#ifndef SL_NO_STATS
#define INCR(counter) ++comp->counter;
#else
#define INCR(counter)
#endif

#define BCMP(p1, p2, n) bcmp((char *)(p1), (char *)(p2), (int)(n))
#define BCOPY(p1, p2, n) bcopy((char *)(p1), (char *)(p2), (int)(n))

void
sl_compress_init(comp)
	struct slcompress *comp;
{
	register u_int i;
	register struct cstate *tstate = comp->tstate;

	bzero((char *)comp, sizeof(*comp));
	for (i = MAX_STATES - 1; i > 0; --i) {
		tstate[i].cs_id = i;
		tstate[i].cs_next = &tstate[i - 1];
	}
	tstate[0].cs_next = &tstate[MAX_STATES - 1];
	tstate[0].cs_id = 0;
	comp->last_cs = &tstate[0];
	comp->last_recv = 255;
	comp->last_xmit = 255;
}


/* ENCODE encodes a number that is known to be non-zero.  ENCODEZ
 * checks for zero (since zero has to be encoded in the long, 3 byte
 * form).
 */
#define ENCODE(n) { \
	if ((u_short)(n) >= 256) { \
		*cp++ = 0; \
		cp[1] = (n); \
		cp[0] = (n) >> 8; \
		cp += 2; \
	} else { \
		*cp++ = (n); \
	} \
}
#define ENCODEZ(n) { \
	if ((u_short)(n) >= 256 || (u_short)(n) == 0) { \
		*cp++ = 0; \
		cp[1] = (n); \
		cp[0] = (n) >> 8; \
		cp += 2; \
	} else { \
		*cp++ = (n); \
	} \
}

#define DECODEL(f) { \
	if (*cp == 0) {\
		(f) = htonl(ntohl(f) + ((cp[1] << 8) | cp[2])); \
		cp += 3; \
	} else { \
		(f) = htonl(ntohl(f) + (u_long)*cp++); \
	} \
}

#define DECODES(f) { \
	if (*cp == 0) {\
		(f) = htons(ntohs(f) + ((cp[1] << 8) | cp[2])); \
		cp += 3; \
	} else { \
		(f) = htons(ntohs(f) + (u_long)*cp++); \
	} \
}

#define DECODEU(f) { \
	if (*cp == 0) {\
		(f) = htons((cp[1] << 8) | cp[2]); \
		cp += 3; \
	} else { \
		(f) = htons((u_long)*cp++); \
	} \
}


int
sl_compress_tcp(m, ip, comp, compress_cid)
	struct mbuf *m;
	register struct ip *ip;
	struct slcompress *comp;
	int compress_cid;
{
	register struct cstate *cs = comp->last_cs->cs_next;
	register u_int hlen = (ip->ip_XHLEN & 0x0f);
	register struct tcphdr *oth;
	register struct tcphdr *th;
	register u_int deltaS, deltaA;
	register u_int changes = 0;
	u_char new_seq[16];
	register u_char *cp = new_seq;

	/*
	 * Bail if this is an IP fragment or if the TCP packet isn't
	 * `compressible' (i.e., ACK isn't set or some other control bit is
	 * set).  (We assume that the caller has already made sure the
	 * packet is IP proto TCP).
	 */
	if ((ip->ip_off & htons(0x3fff)) || m->m_len < 40)
		return (TYPE_IP);

	th = (struct tcphdr *)&((int *)ip)[hlen];
	if ((th->th_flags & (TH_SYN|TH_FIN|TH_RST|TH_ACK)) != TH_ACK)
		return (TYPE_IP);
	/*
	 * Packet is compressible -- we're going to send either a
	 * COMPRESSED_TCP or UNCOMPRESSED_TCP packet.  Either way we need
	 * to locate (or create) the connection state.  Special case the
	 * most recently used connection since it's most likely to be used
	 * again & we don't have to do any reordering if it's used.
	 */
	INCR(sls_packets)
	if (ip->ip_src.s_addr != cs->cs_ip.ip_src.s_addr ||
	    ip->ip_dst.s_addr != cs->cs_ip.ip_dst.s_addr ||
	    *(int *)th != ((int *)&cs->cs_ip)[cs->cs_ip.ip_XHLEN & 0x0f]) {
		/*
		 * Wasn't the first -- search for it.
		 *
		 * States are kept in a circularly linked list with
		 * last_cs pointing to the end of the list.  The
		 * list is kept in lru order by moving a state to the
		 * head of the list whenever it is referenced.  Since
		 * the list is short and, empirically, the connection
		 * we want is almost always near the front, we locate
		 * states via linear search.  If we don't find a state
		 * for the datagram, the oldest state is (re-)used.
		 */
		register struct cstate *lcs;
		register struct cstate *lastcs = comp->last_cs;

		do {
			lcs = cs; cs = cs->cs_next;
			INCR(sls_searches)
			if (ip->ip_src.s_addr == cs->cs_ip.ip_src.s_addr
			    && ip->ip_dst.s_addr == cs->cs_ip.ip_dst.s_addr
			    && *(int *)th == ((int *)&cs->cs_ip)[cs->cs_ip.ip_XHLEN & 0x0f])
				goto found;
		} while (cs != lastcs);

		/*
		 * Didn't find it -- re-use oldest cstate.  Send an
		 * uncompressed packet that tells the other side what
		 * connection number we're using for this conversation.
		 * Note that since the state list is circular, the oldest
		 * state points to the newest and we only need to set
		 * last_cs to update the lru linkage.
		 */
		INCR(sls_misses)
		comp->last_cs = lcs;
		hlen += th->th_XHLEN >> 4;
		hlen <<= 2;
		goto uncompressed;

	found:
		/*
		 * Found it -- move to the front on the connection list.
		 */
		if (cs == lastcs)
			comp->last_cs = lcs;
		else {
			lcs->cs_next = cs->cs_next;
			cs->cs_next = lastcs->cs_next;
			lastcs->cs_next = cs;
		}
	}

	/*
	 * Make sure that only what we expect to change changed. The first
	 * line of the `if' checks the IP protocol version, header length &
	 * type of service.  The 2nd line checks the "Don't fragment" bit.
	 * The 3rd line checks the time-to-live and protocol (the protocol
	 * check is unnecessary but costless).  The 4th line checks the TCP
	 * header length.  The 5th line checks IP options, if any.  The 6th
	 * line checks TCP options, if any.  If any of these things are
	 * different between the previous & current datagram, we send the
	 * current datagram `uncompressed'.
	 */
	oth = (struct tcphdr *)&((int *)&cs->cs_ip)[hlen];
	deltaS = hlen;
	hlen += th->th_XHLEN >> 4;
	hlen <<= 2;

	if (((u_short *)ip)[0] != ((u_short *)&cs->cs_ip)[0] ||
	    ((u_short *)ip)[3] != ((u_short *)&cs->cs_ip)[3] ||
	    ((u_short *)ip)[4] != ((u_short *)&cs->cs_ip)[4] ||
	    th->th_XHLEN != oth->th_XHLEN ||
	    (deltaS > 5 &&
	     BCMP(ip + 1, &cs->cs_ip + 1, (deltaS - 5) << 2)) ||
	    ((th->th_XHLEN & 0xf0) > (5 << 4) &&
	     BCMP(th + 1, oth + 1, ((th->th_XHLEN & 0xf0) - (5 << 4)) >> 2)))
		goto uncompressed;

	/*
	 * Figure out which of the changing fields changed.  The
	 * receiver expects changes in the order: urgent, window,
	 * ack, seq (the order minimizes the number of temporaries
	 * needed in this section of code).
	 */
	if (th->th_flags & TH_URG) {
		deltaS = ntohs(th->th_urp);
		ENCODEZ(deltaS);
		changes |= NEW_U;
	} else if (th->th_urp != oth->th_urp)
		/* argh! URG not set but urp changed -- a sensible
		 * implementation should never do this but RFC793
		 * doesn't prohibit the change so we have to deal
		 * with it. */
		 goto uncompressed;

	if (deltaS = (u_short)(ntohs(th->th_win) - ntohs(oth->th_win))) {
		ENCODE(deltaS);
		changes |= NEW_W;
	}

	if (deltaA = ntohl(th->th_ack) - ntohl(oth->th_ack)) {
		if (deltaA > 0xffff)
			goto uncompressed;
		ENCODE(deltaA);
		changes |= NEW_A;
	}

	if (deltaS = ntohl(th->th_seq) - ntohl(oth->th_seq)) {
		if (deltaS > 0xffff)
			goto uncompressed;
		ENCODE(deltaS);
		changes |= NEW_S;
	}

	switch(changes) {

	case 0:
		/*
		 * Nothing changed. If this packet contains data and the
		 * last one didn't, this is probably a data packet following
		 * an ack (normal on an interactive connection) and we send
		 * it compressed.  Otherwise it's probably a retransmit,
		 * retransmitted ack or window probe.  Send it uncompressed
		 * in case the other side missed the compressed version.
		 */
		if (ip->ip_len != cs->cs_ip.ip_len &&
		    ntohs(cs->cs_ip.ip_len) == hlen)
			break;

		/* (fall through) */

	case SPECIAL_EI:
	case SPECIAL_D:
		/*
		 * actual changes match one of our special case encodings --
		 * send packet uncompressed.
		 */
		goto uncompressed;

	case NEW_S|NEW_A:
		if (deltaS == deltaA &&
		    deltaS == ntohs(cs->cs_ip.ip_len) - hlen) {
			/* special case for echoed terminal traffic */
			changes = SPECIAL_EI;
			cp = new_seq;
		}
		break;

	case NEW_S:
		if (deltaS == ntohs(cs->cs_ip.ip_len) - hlen) {
			/* special case for data xfer */
			changes = SPECIAL_D;
			cp = new_seq;
		}
		break;
	}

	deltaS = ntohs(ip->ip_id) - ntohs(cs->cs_ip.ip_id);
	if (deltaS != 1) {
		ENCODEZ(deltaS);
		changes |= NEW_I;
	}
	if (th->th_flags & TH_PUSH)
		changes |= TCP_PUSH_BIT;
	/*
	 * Grab the cksum before we overwrite it below.  Then update our
	 * state with this packet's header.
	 */
	deltaA = ntohs(th->th_sum);
	BCOPY(ip, &cs->cs_ip, hlen);

	/*
	 * We want to use the original packet as our compressed packet.
	 * (cp - new_seq) is the number of bytes we need for compressed
	 * sequence numbers.  In addition we need one byte for the change
	 * mask, one for the connection id and two for the tcp checksum.
	 * So, (cp - new_seq) + 4 bytes of header are needed.  hlen is how
	 * many bytes of the original packet to toss so subtract the two to
	 * get the new packet size.
	 */
	deltaS = cp - new_seq;
	cp = (u_char *)ip;
	if (compress_cid == 0 || comp->last_xmit != cs->cs_id) {
		comp->last_xmit = cs->cs_id;
		hlen -= deltaS + 4;
		cp += hlen;
		*cp++ = changes | NEW_C;
		*cp++ = cs->cs_id;
	} else {
		hlen -= deltaS + 3;
		cp += hlen;
		*cp++ = changes;
	}
	m->m_len -= hlen;
	m->m_data += hlen;
	*cp++ = deltaA >> 8;
	*cp++ = deltaA;
	BCOPY(new_seq, cp, deltaS);
	INCR(sls_compressed)
	return (TYPE_COMPRESSED_TCP);

	/*
	 * Update connection state cs & send uncompressed packet ('uncompressed'
	 * means a regular ip/tcp packet but with the 'conversation id' we hope
	 * to use on future compressed packets in the protocol field).
	 */
uncompressed:
	BCOPY(ip, &cs->cs_ip, hlen);
	ip->ip_p = cs->cs_id;
	comp->last_xmit = cs->cs_id;
	return (TYPE_UNCOMPRESSED_TCP);
}


int
sl_uncompress_tcp(bufp, len, type, comp)
	u_char **bufp;
	int len;
	int type;
	struct slcompress *comp;
{
	register u_char *cp;
	register u_int hlen, changes;
	register struct tcphdr *th;
	register struct cstate *cs;
	register struct ip *ip;

	switch (type) {

	case TYPE_UNCOMPRESSED_TCP:
		ip = (struct ip *) *bufp;
		if (ip->ip_p >= MAX_STATES)
			goto bad;
		cs = &comp->rstate[comp->last_recv = ip->ip_p];
		comp->flags &=~ SLF_TOSS;
		ip->ip_p = IPPROTO_TCP;
		hlen = ip->ip_XHLEN & 0x0f;
		hlen += ((struct tcphdr *)&((int *)ip)[hlen])->th_XHLEN >> 4;
		hlen <<= 2;
		BCOPY(ip, &cs->cs_ip, hlen);
		cs->cs_ip.ip_sum = 0;
		cs->cs_hlen = hlen;
		INCR(sls_uncompressedin)
		return (len);

	default:
		goto bad;

	case TYPE_COMPRESSED_TCP:
		break;
	}
	/* We've got a compressed packet. */
	INCR(sls_compressedin)
	cp = *bufp;
	changes = *cp++;
	if (changes & NEW_C) {
		/* Make sure the state index is in range, then grab the state.
		 * If we have a good state index, clear the 'discard' flag. */
		if (*cp >= MAX_STATES)
			goto bad;

		comp->flags &=~ SLF_TOSS;
		comp->last_recv = *cp++;
	} else {
		/* this packet has an implicit state index.  If we've
		 * had a line error since the last time we got an
		 * explicit state index, we have to toss the packet. */
		if (comp->flags & SLF_TOSS) {
			INCR(sls_tossed)
			return (0);
		}
	}
	cs = &comp->rstate[comp->last_recv];
	hlen = (cs->cs_ip.ip_XHLEN & 0x0f) << 2;
	th = (struct tcphdr *)&((u_char *)&cs->cs_ip)[hlen];
	th->th_sum = htons((*cp << 8) | cp[1]);
	cp += 2;
	if (changes & TCP_PUSH_BIT)
		th->th_flags |= TH_PUSH;
	else
		th->th_flags &=~ TH_PUSH;

	switch (changes & SPECIALS_MASK) {
	case SPECIAL_EI:
		{
		register u_int i = ntohs(cs->cs_ip.ip_len) - cs->cs_hlen;
		th->th_ack = htonl(ntohl(th->th_ack) + i);
		th->th_seq = htonl(ntohl(th->th_seq) + i);
		}
		break;

	case SPECIAL_D:
		th->th_seq = htonl(ntohl(th->th_seq) + ntohs(cs->cs_ip.ip_len)
				   - cs->cs_hlen);
		break;

	default:
		if (changes & NEW_U) {
			th->th_flags |= TH_URG;
			DECODEU(th->th_urp)
		} else
			th->th_flags &=~ TH_URG;
		if (changes & NEW_W)
			DECODES(th->th_win)
		if (changes & NEW_A)
			DECODEL(th->th_ack)
		if (changes & NEW_S)
			DECODEL(th->th_seq)
		break;
	}
	if (changes & NEW_I) {
		DECODES(cs->cs_ip.ip_id)
	} else
		cs->cs_ip.ip_id = htons(ntohs(cs->cs_ip.ip_id) + 1);

	/*
	 * At this point, cp points to the first byte of data in the
	 * packet.  If we're not aligned on a 4-byte boundary, copy the
	 * data down so the ip & tcp headers will be aligned.  Then back up
	 * cp by the tcp/ip header length to make room for the reconstructed
	 * header (we assume the packet we were handed has enough space to
	 * prepend 128 bytes of header).  Adjust the length to account for
	 * the new header & fill in the IP total length.
	 */
	len -= (cp - *bufp);
	if (len < 0)
		/* we must have dropped some characters (crc should detect
		 * this but the old slip framing won't) */
		goto bad;

	if ((int)cp & 3) {
		if (len > 0)
			(void) ovbcopy(cp, (caddr_t)((int)cp &~ 3), len);
		cp = (u_char *)((int)cp &~ 3);
	}
	cp -= cs->cs_hlen;
	len += cs->cs_hlen;
	cs->cs_ip.ip_len = htons(len);
	BCOPY(&cs->cs_ip, cp, cs->cs_hlen);
	*bufp = cp;

	/* recompute the ip header checksum */
	{
		register u_short *bp = (u_short *)cp;
		for (changes = 0; hlen > 0; hlen -= 2)
			changes += *bp++;
		changes = (changes & 0xffff) + (changes >> 16);
		changes = (changes & 0xffff) + (changes >> 16);
		((struct ip *)cp)->ip_sum = ~ changes;
	}
	return (len);
bad:
	comp->flags |= SLF_TOSS;
	INCR(sls_errorin)
	return (0);
}
