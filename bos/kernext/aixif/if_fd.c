static char sccsid[] = "@(#)37        1.6.2.26  src/bos/kernext/aixif/if_fd.c, sysxaixif, bos41J, 9520A_all 5/11/95 17:20:23";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: fddi_arpinput
 *		fddi_arpresolve
 *		fddi_arpwhohas
 *		in_fddi_arpinput
 *		reversebit_copy
 *		config_fd
 *		config_fd_init
 *		config_fd_term
 *		fd_attach
 *		fd_detach
 *		fd_init
 *		fd_ioctl
 *		fd_output
 *
 *   ORIGINS: 26,27,85,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
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
/* Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/**************************************************************************
 *
 *      if_fd.c - FDDI network interface to aix device handler.
 * 		- Address Resolution Protocol for FDDI networks.
 * 
 *************************************************************************/


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/file.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/nettrace.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include <net/nh.h>	/*pdw*/
#include <net/if.h>
#include <net/if_types.h>

#include <netinet/in.h>
#include <netinet/in_netarp.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <netinet/if_fddi.h>

#include <aixif/net_if.h>
#include <net/if_arp.h>
#ifdef IP_MULTICAST
#include <netinet/if_mcast_aux.h>
#endif


/* This array allows efficient bit reversal for FDDI arp responses */

u_char bit_reverse[] = {
   0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0,
   0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
   0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4,
   0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
   0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc,
   0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
   0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca,
   0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
   0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6,
   0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
   0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1,
   0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
   0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9,
   0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
   0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd,
   0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
   0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3,
   0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
   0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7,
   0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
   0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf,
   0x3f, 0xbf, 0x7f, 0xff};


/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
fddi_arpwhohas(ac, addr)
	register struct arpcom *ac;
	struct in_addr *addr;
{
	register struct mbuf *m;
	register struct fddi_arp *fa;
	register struct fddi_hdr *hdrp;
	struct sockaddr_fddi sa;

	if ((m = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL) {
		ac->ac_if.if_snd.ifq_drops++;
		return;
	}

	m->m_len = sizeof(*fa);
	m->m_pkthdr.len = sizeof(*fa);
	m->m_flags |= M_BCAST;
	MH_ALIGN(m, sizeof(*fa));

	hdrp = (struct fddi_hdr *)&sa.sa_data;
	hdrp->fddi_mac.mac_fcf_f = FCF_FDDI;

	/*
	 * destination is BROADCAST
	 */
	bcopy((caddr_t) fddi_broadcastaddr, (caddr_t) hdrp->fddi_mac.mac_dst_f,
		sizeof (hdrp->fddi_mac.mac_dst_f));

	/*
	 * source is me
	 */
	bcopy((caddr_t)ac->ac_fdaddr, (caddr_t)hdrp->fddi_mac.mac_src_f, 
		sizeof(hdrp->fddi_mac.mac_src_f));

	hdrp->fddi_mac.mac_rcf_f = 0;
	
        if (ac->ac_if.if_flags & IFF_ALLCAST)
                hdrp->fddi_mac.mac_rcf_f |= RCF_ALL_BROADCAST;

	fa = mtod(m, struct fddi_arp *);
	bzero((caddr_t)fa, sizeof (*fa));

	fa->arp_hrd = htons(ARPHRD_FDDI);
	fa->arp_pro = htons(_FDDI_TYPE_IP);
	fa->arp_hln = sizeof(fa->arp_sha);	/* hardware address length */
	fa->arp_pln = sizeof(fa->arp_spa);	/* protocol address length */
	fa->arp_op = htons(ARPOP_REQUEST);

	reversebit_copy((caddr_t)ac->ac_fdaddr, (caddr_t)fa->arp_sha,
	   sizeof(fa->arp_sha));
	bcopy((caddr_t)&ac->ac_ipaddr, (caddr_t)fa->arp_spa,
	   sizeof(fa->arp_spa));
	bcopy((caddr_t)addr, (caddr_t)fa->arp_tpa, sizeof(fa->arp_tpa));
	ie2_llc(mac_to_llc_f(&hdrp->fddi_mac), _FDDI_TYPE_ARP);

	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}

/*
 * fddi_arpresolve -	FDDI Address Resolution Protocol
 *
 * Input:
 *	ac		-	FDDI common structure fomr interface
 *	m		-	mbuf chain of intended sendee
 *	destip		-	destination IP address
 *	daddr		-	^ to destination address
 *
 * Returns:
 *	!0		-	 OK to send packet
 *	 0		-	!OK to send packet
 *
 * Resolve an IP address into a FDDI address.  If success, 
 * daddr will contain the MAC and LLC headers required to send the pkt.
 * If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.
 *
 * We do some (conservative) locking here at splimp, since
 * arptab is also altered from input interrupt service.
 * calls fddi_arpinput when _FDDI_TYPE_ARP packets come in).
 */
fddi_arpresolve(ac, m, dst, macp)
	struct arpcom *ac;
	struct mbuf *m;
	struct sockaddr_in *dst;
	caddr_t macp;
{
	register struct arptab *at = 0;
	struct in_addr *destip;
	struct sockaddr_in sin;
	register struct in_ifaddr *ia;
#ifdef IP_MULTICAST	
	u_char	fddi_multicastaddr[6];
#endif
	NETSPL_DECL(s)

	destip = &dst->sin_addr;

	/*
	 * This hook needed by NETPMON...
	 */
	NETTRC4(HKWD_IFFD|hkwd_output_in, &(ac->ac_if), m, dst->sin_family,
		destip->s_addr);

#ifdef IP_MULTICAST
	/*
	 * check for internet multicast.
	 */
	if (IN_MULTICAST(ntohl(destip->s_addr))) {
		FDDI_MAP_IP_MULTICAST(destip, fddi_multicastaddr);

		FDDI_FILLIN_HDR((struct fddi_mac_hdr *)macp, ac->ac_fdaddr, 
			(caddr_t) fddi_multicastaddr, 0, (caddr_t)NULL);
		return 1;
	} else
#endif IP_MULTICAST
	/*
	 * check for internet broadcast.
	 */
	if (m->m_flags & M_BCAST) {
		FDDI_FILLIN_HDR((struct fddi_mac_hdr *)macp, ac->ac_fdaddr, 
			(caddr_t) fddi_broadcastaddr, 0, (caddr_t)NULL);
		return 1;
	}

	/*
	 * if for us, use software loopback driver if up
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
		if ((ia->ia_ifp == &ac->ac_if) &&
		    (destip->s_addr == ia->ia_addr.sin_addr.s_addr)) {
			if (ia->ia_ifp->if_flags & IFF_DO_HW_LOOPBACK ||
				!((&loif)->if_flags & IFF_UP)) {
				FDDI_FILLIN_HDR((struct fddi_mac_hdr *)macp, 
					ac->ac_fdaddr, ac->ac_fdaddr, 0, 
					(caddr_t)NULL);
				return (1);
			} else {
				sin.sin_family = AF_INET;
				sin.sin_addr = *destip;
				(void) looutput(&loif, m,
					(struct sockaddr *)&sin, 
					(struct rtentry *)NULL);
				/*
				 * The packet has already been sent and freed.
				 */
				return (0);
			}
		}
	}

	ARPTAB_LOCK(&arptab_lock);
	ARPTAB_LOOK(at, destip->s_addr);
	if (at == 0) {			/* not found */
		/*
		 * ifp->if_flags & IFF_NOARP is ignored for FDDI 
		 */
		at = arptnew(destip, &ac->ac_if);
		at->at_hold = m;
		ARPTAB_UNLOCK(&arptab_lock);
		fddi_arpwhohas(ac, destip);
		return (0);
	}
	if (at->at_flags & ATF_COM) {	/* entry IS complete */

		FDDI_FILLIN_HDR((struct fddi_mac_hdr *)macp, ac->ac_fdaddr, 
			(caddr_t)at->at_fdaddr, at->at_rcf, 
			(caddr_t)at->at_fddi_seg);

		ARPTAB_UNLOCK(&arptab_lock);

		/*  Addr resolved, squirt out pkt.  */
		return 1;
	}
	/*
	 * There is an arptab entry, but no fddi address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (at->at_hold) {
		m_freem(at->at_hold);
		ac->ac_if.if_arpdrops++;
	}

	at->at_hold = m;
        if (!(at->at_flags & ATF_ASK)) {        /* Free to ask again */
                at->at_flags |= ATF_ASK;
		fddi_arpwhohas(ac, destip); /* ask again */ 
        }
	ARPTAB_UNLOCK(&arptab_lock);
	return (0);
}

/*
 * fddi_arpinput -	FDDI ARP input
 *
 * Input:
 *	ac	-	^ to interface ARP common structure
 *	pkt	-	^ to MAC/LLC/ARP data
 *
 * Called from FDDI interrupt handlers
 * when FDDI packet type _FDDI_TYPE_ARP
 * is received.  Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
fddi_arpinput(ac, m, mac)
	struct arpcom *ac;
	register struct mbuf *m;
	register struct fddi_mac_hdr *mac;
{
	register struct arphdr *ar;

	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;


        /*
         * Well, this test says that we should ignore arp broadcasts from
         * foreign token rings when 'localbroadcast=true' (in /etc/net).
         * 'localbroadcast' means that when you send on the token ring,
         * you only send to the local ring, and not through any bridge.
         * Route_bytes > 2 means that the packet has passed through at
         * least one bridge.
         */
        if ((ac->ac_if.if_flags & IFF_ALLCAST) == 0)
                if(has_route_f(mac) && (route_bytes_f(mac) > 2))
                        goto out;

	ar  = mtod(m, struct arphdr *);

	if ((ntohs(ar->ar_hrd) != (ARPHRD_FDDI)) &&
	    (ntohs(ar->ar_hrd) != (ARPHRD_802_3))) {
		net_error(&ac->ac_if, ARP_WRONG_HDR, 0);
		goto out;
	}

	if (has_route_f(mac)) {
		mac->mac_rcf_f ^=  RCF_DIRECTION;
                mac->mac_rcf_f &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}

	switch (ntohs(ar->ar_pro)) {
	    case _FDDI_TYPE_IP:
		in_fddi_arpinput(ac, m, (struct fddi_hdr *)mac);
		return;

	    default:
		net_error(&ac->ac_if, ARP_UNK_PROTO, 0);
		break;
	}
out:
	m_freem(m);
	return;
}

/*
 * ARP for Internet protocols on FDDI.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 */

in_fddi_arpinput(ac, m, hp)
	register struct arpcom *ac;
	struct mbuf *m;
	struct fddi_hdr *hp;
{
	struct fddi_hdr *hdrp;
	register struct fddi_arp *fa;
	register struct arptab *at;  /* same as "merge" flag */
	struct sockaddr_in sin;
	struct sockaddr_fddi sa;
	struct in_addr isaddr, itaddr, myaddr;
	int proto, op;
	register struct mbuf *m0;
	
	NETSPL_DECL(s)


	myaddr = ac->ac_ipaddr;
	fa     = mtod(m, struct fddi_arp *);
	proto  = ntohs(fa->arp_pro);
	op     = ntohs(fa->arp_op);

	bcopy(&((struct in_addr *)fa->arp_spa)->s_addr
		, &isaddr.s_addr 
		, sizeof (isaddr.s_addr));
	bcopy(&((struct in_addr *)fa->arp_tpa)->s_addr
		, &itaddr.s_addr 
		, sizeof (itaddr.s_addr));

	reversebit_copy((caddr_t)fa->arp_sha, (caddr_t)fa->arp_sha,
		sizeof (fa->arp_sha));
	if (!bcmp((caddr_t)fa->arp_sha
		      , (caddr_t)ac->ac_fdaddr
		      , sizeof (fa->arp_sha))) {
		goto out;	/* it's from me, ignore it. */
	}
	if (!bcmp((caddr_t)fa->arp_sha
		      , (caddr_t)fddi_broadcastaddr
		      , sizeof (fa->arp_sha))) {
		net_error(&ac->ac_if, ARP_IPBRDCAST_ADDR, 0);
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		net_error(&ac->ac_if, ARP_DUP_ADDR, 0);
		itaddr = myaddr;
		goto out;
	}
	ARPTAB_LOCK(&arptab_lock);
	ARPTAB_LOOK(at, isaddr.s_addr);
	if (at) {
                /* DONT update if this packet was not received
                 * via the interface the originated this arp table
                 * entry.  (fix for multi-protocol bridges)
                 */
                if ((at->at_flags & ATF_COM) &&
                    (m->m_pkthdr.rcvif != at->at_ifp))
                        goto reply;

                if (!has_route_f(&hp->fddi_mac) || !at->at_rcf ||
                        (has_route_f(&hp->fddi_mac) &&
                        (route_bytes_f(&hp->fddi_mac) <
                        ((at->at_rcf >> 8) & 0x1f)))) {
	    add_data:
			bcopy((caddr_t)fa->arp_sha, (caddr_t)at->at_traddr,
			    sizeof(fa->arp_sha));
			at->at_flags |= ATF_COM;

			/*
			 * snarf any routing info
			 */
			if (has_route_f(&hp->fddi_mac)
			    && route_bytes_f(&hp->fddi_mac) > 
				sizeof (hp->fddi_mac.mac_rcf_f)) {
				at->at_fddi_rcf = hp->fddi_mac.mac_rcf_f;
				bcopy(hp->fddi_mac.mac_seg_f, 
					at->at_fddi_seg, 
					sizeof (at->at_fddi_seg));
			} else
				at->at_fddi_rcf = 0;


			if (at->at_hold) {
				struct mbuf *om;

				sin.sin_family = AF_INET;
				sin.sin_addr = isaddr;
				om = at->at_hold;
				at->at_hold = 0;
				ARPTAB_UNLOCK(&arptab_lock);
				(*ac->ac_if.if_output)(&ac->ac_if, 
					om, (struct sockaddr_fddi *)&sin, 
					(struct rtentry *)0);
				goto out;
			}
		}
	}
	if (at == 0 && itaddr.s_addr == myaddr.s_addr) {
		/*
		 * ensure we have a table entry
		 */
		at = arptnew(&isaddr, &ac->ac_if);
		at->at_hold = 0;	/* sanity */

		/*
		 * go to common code to add ARP data.  Note that we will
		 * make the check `if (at == 0 ...' again, but will fail
		 * the second time.   Yes, this goto looks harmful.
		 */
		goto add_data;
	}
reply:
	switch (proto) {
	    case _FDDI_TYPE_IP:
		/*
		 * Reply if this is an IP request.
		 */
		if (op != ARPOP_REQUEST) {
			goto freeout;
		}
		break;

	    default:
		goto freeout;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		reversebit_copy((caddr_t)fa->arp_sha, (caddr_t)fa->arp_tha,
		    sizeof(fa->arp_sha));
		reversebit_copy((caddr_t)ac->ac_traddr, (caddr_t)fa->arp_sha,
		    sizeof(fa->arp_sha));
	} else {
		ARPTAB_LOOK(at, itaddr.s_addr);
		if (at == NULL || (at->at_flags & ATF_PUBL) == 0) {
			goto freeout;
		}
		reversebit_copy((caddr_t)fa->arp_sha, (caddr_t)fa->arp_tha,
		    sizeof(fa->arp_sha));
		reversebit_copy((caddr_t)at->at_fdaddr, (caddr_t)fa->arp_sha,
		    sizeof(fa->arp_sha));
	}

	bcopy((caddr_t)fa->arp_spa, (caddr_t)fa->arp_tpa,
	    sizeof(fa->arp_spa));
	bcopy((caddr_t)&itaddr, (caddr_t)fa->arp_spa,
	    sizeof(fa->arp_spa));
	fa->arp_op = htons(ARPOP_REPLY); 

        if ((m0 = m_get(M_DONTWAIT, MT_DATA)) == NULL) {
		ac->ac_if.if_snd.ifq_drops++;
                goto freeout;
	}

	m0->m_len = sizeof (*fa);
        m0->m_pkthdr.len = sizeof(*fa);
	MH_ALIGN(m0, sizeof(*fa));
	*mtod(m0, struct fddi_arp *) = *fa;
	fa = mtod(m0, struct fddi_arp *);

	fa->arp_pro = htons(_FDDI_TYPE_IP);

	/*
	 * create MAC/LLC stuff
	 */
	hdrp = (struct fddi_hdr *)&sa.sa_data;
	
	/*
	 * set destination
	 */
	reversebit_copy((caddr_t)fa->arp_tha, 
		(caddr_t)hdrp->fddi_mac.mac_dst_f,
		sizeof(hdrp->fddi_mac.mac_dst_f));

	/*
	 * Source is me.
	 */
	bcopy((caddr_t) ac->ac_fdaddr
		, (caddr_t) hdrp->fddi_mac.mac_src_f
		, sizeof (hdrp->fddi_mac.mac_src_f));


	hdrp->fddi_mac.mac_fcf_f     = FCF_FDDI;

	/*
	 * routing control info present?
	 */
	if (at->at_rcf) {
		hdrp->fddi_mac.mac_src_f[0] |= RI_PRESENT;
		hdrp->fddi_mac.mac_rcf_f     = at->at_rcf;

		bcopy(at->at_seg, hdrp->fddi_mac.mac_seg_f, 
			sizeof(hdrp->fddi_mac.mac_seg_f));
	} else
		hdrp->fddi_mac.mac_src_f[0] &= ~RI_PRESENT;

	ie2_llc((caddr_t)(hdrp)+mac_size_f(&(hdrp->fddi_mac)), _FDDI_TYPE_ARP);

	ARPTAB_UNLOCK(&arptab_lock);
	sa.sa_family      = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m0, &sa, (struct rtentry *)0);
	goto out;

freeout:
        ARPTAB_UNLOCK(&arptab_lock);
out:
	m_freem(m);
	return;
}

reversebit_copy(src, dst, size)
caddr_t src, dst;
int size;
{
	for(;size>0;size--)
		*dst++ = bit_reverse[*src++];
}


/******************************************************************************
 * Software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * fd_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address ...
 *****************************************************************************/
struct	fd_softc {
	struct	arpcom 	fd_ac;		/* network common part	   */
	ndd_t		*nddp;		/* NDD pointer */
};
struct fd_softc *fd_softc = (struct fd_softc *)NULL;

#define	fd_if		fd_ac.ac_if		/* network-visible interface */
#define	fd_addr		fd_ac.ac_enaddr		/* hardware address	     */
#define FDDI_MAX_GATHERS	(3)

typedef struct fddi_hdr Fddi_hdr_t;

int			if_fd_lock=LOCK_AVAIL; /* attach locking            */
int init = 0;

void			fd_netintr();		/* recv interrupt handler    */
void			fd_statintr();		/* status interrupt handler  */
void			fd_txintr();		/* xmit complete handler     */

/*************************************************************************
 *
 *	config_fd() - FDDI kernel extension entry point
 *
 ************************************************************************/
config_fd(cmd, uio)
int		cmd;
struct uio	*uio;
{
	struct	device_req		device;
	int				error;
	int				unit;
	int				lockt;
	char				*cp;

	lockt = lockl(&if_fd_lock, LOCK_SHORT);

	if (init == 0) {
		if (ifsize <= 0)
			ifsize = IF_SIZE;
		if (error = pincode(config_fd))
			goto out;
		fd_softc = (struct fd_softc *)
		    xmalloc(sizeof(struct fd_softc)*ifsize, 2, pinned_heap);
		if (fd_softc == (struct fd_softc *)NULL) {
		    unpincode(config_fd);
		    unlockl(&if_fd_lock);
		    return(ENOMEM);
		}
		bzero(fd_softc, sizeof(struct fd_softc) * ifsize); 
		init++;
	}

	if (cmd != CFG_INIT) {
		error = EINVAL;
		goto out;	
	}
	
	if (uio) {	
		error = uiomove((caddr_t) &device, (int) sizeof(device), UIO_WRITE, uio);
		if (error) 
			goto out;	
	}

	cp = device.dev_name;
	while(*cp < '0' || *cp > '9') cp++;
	unit = atoi(cp);

	if (unit >= ifsize) {
		error = ENXIO;
		goto out;
	}

	error = config_fd_init(&device, unit);
out:
	unlockl(&if_fd_lock);

	return(error);
}


/*************************************************************************
 *
 *	config_fd_init() - FDDI load initialization
 *
 ************************************************************************/
config_fd_init(device, unit)
register struct	device_req	*device;
int				unit;
{
	register struct fd_softc	*fdp;
	int				rc = 0;

	fdp = &fd_softc[unit];
	rc = ns_alloc(device->dev_name, &fdp->nddp);
	if (rc) {
		bsdlog(LOG_ERR, 
			"if_fd: ns_alloc(%s) failed with errno = %d\n", 
			device->dev_name, rc);
		return (rc);
	}

	return(fd_attach(unit));
}

/*************************************************************************
 *
 * 	fd_attach() - logically attach the fd network interface
 *
 ************************************************************************/
fd_attach(unit)
	unsigned unit;
{
	register struct ifnet		*ifp;
	struct fd_softc			*fdp;
	extern int			fd_init();
	extern int			fd_output();
	extern int			fd_ioctl();
	int				rc;

	if (unit >= ifsize) {
		return ENXIO;
	}

	ifp             = &fd_softc[unit].fd_if;
	fdp		= &fd_softc[unit];

	/* Check if the adapter supports local echo */
	if (fdp->nddp->ndd_flags & NDD_SIMPLEX)
		ifp->if_flags = IFF_BPF | IFF_BROADCAST | IFF_ALLCAST | 
			IFF_SIMPLEX;
	else
		ifp->if_flags = IFF_BPF | IFF_BROADCAST | IFF_ALLCAST;
#ifdef IP_MULTICAST
	ifp->if_flags 	|= IFF_MULTICAST;
	ACMULTI_LOCKINIT(&(fdp->fd_ac));
#endif	
	bcopy(fdp->nddp->ndd_physaddr, fdp->fd_addr, 6);
	ifp->if_mtu	= _FDDI_MTU;
	ifp->if_unit	= unit;
	ifp->if_name	= "fi";
	ifp->if_init	= fd_init;
	ifp->if_output	= fd_output;
	ifp->if_ioctl	= fd_ioctl;
	ifp->if_type 	= IFT_FDDI;
	ifp->if_addrlen = FDDI_ADDRLEN;
	ifp->if_hdrlen	= sizeof(Fddi_hdr_t);

	ifp->if_arpres = fddi_arpresolve;
	ifp->if_arpinput = fddi_arpinput;

	if_attach(ifp);
	if_nostat(ifp);

	return (0);
}

/*************************************************************************
 *
 * 	fd_init() - logically initialize the fd network interface
 *
 ************************************************************************/
fd_init()
{
	register struct ifnet		*ifp;
	int				unit;

	for (unit = 0; unit < ifsize; unit++) {
		ifp = &fd_softc[unit].fd_if;

		/* not yet, if address still unknown */
		if (ifp->if_addrlist == (struct ifaddr *)0)
			continue;
		ifp->if_flags |= IFF_RUNNING;
	}

}

/*************************************************************************
 *
 * 	fd_ioctl() - process ioctl request for the fd network interface
 *    
 *	NOTE: error code returned is ignored by the caller.
 *
 ************************************************************************/
fd_ioctl(ifp, cmd, data)
	register struct ifnet		*ifp;
	int				cmd;
	caddr_t				data;
{
	register struct ifaddr		*ifa = (struct ifaddr *)data;
	register struct fd_softc	*fdp = &fd_softc[ifp->if_unit];
	int				error = 0;
	struct timestruc_t		ct;

	if (!fdp->nddp) {
		return(ENODEV);
	}

	switch (cmd) {
	    case SIOCIFDETACH:
		ns_free(fdp->nddp);
		fdp->nddp = 0;
		break;

	    case IFIOCTL_ADD_FILTER:
		{
		ns_8022_t	*filter;
		ns_user_t	*user;

		filter = &((struct if_filter *)data)->filter;
		user = &((struct if_filter *)data)->user;
		error = ns_add_filter(fdp->nddp, filter, sizeof(*filter), 
			user);
		if (error && error != EALREADY)
			bsdlog(LOG_ERR, 
				"if_fd: ns_add_filter() failed with %d return code.\n", error);
		break;
		}

	    case IFIOCTL_DEL_FILTER:
		ns_del_filter(fdp->nddp, (ns_8022_t *)data, sizeof(ns_8022_t));
		break;

	    case SIOCSIFADDR:
                switch (ifa->ifa_addr->sa_family) {
		    case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			break;

		    default:
			break;
		}
		fd_init();
		ifp->if_flags |= IFF_UP;
		curtime(&ct);
		ifp->if_lastchange.tv_sec = (int)ct.tv_sec;
		ifp->if_lastchange.tv_usec = (int)ct.tv_nsec / 1000;
		break;

	    case SIOCSIFFLAGS:
		fd_init();
		break;
#ifdef IP_MULTICAST
	    case NDD_PROMISCUOUS_ON:
	    case NDD_PROMISCUOUS_OFF:
		error = (*(fdp->nddp->ndd_ctl))(fdp->nddp, cmd, 0, 0); 
		break;	

#define DEBUGMSG(x)
	    case SIOCADDMULTI:
		{
		register struct ifreq *ifr = (struct ifreq *)data;
		void fddi_map_ip_multicast();
		char 	addr[6];

		/*
		 * Update our multicast list.
		 */
		switch(driver_addmulti(ifr, &(fdp->fd_ac), 
			fddi_map_ip_multicast, &error, addr)) {
		    case ADD_ADDRESS: 
			DEBUGMSG(("if_fd: ioctl SIOCADDMULTI ADD_ADDRESS\n"));
			error = (*(fdp->nddp->ndd_ctl))
					(fdp->nddp, NDD_ENABLE_ADDRESS, addr,
						FDDI_ADDRLEN); 
			if(error) {
				int rc;
				DEBUGMSG(("if_fd: ENABLE_ADDRESS err %d", error));
				driver_delmulti(ifr, &(fdp->fd_ac), 
					fddi_map_ip_multicast, &rc, addr);
			}
			break;
	      
		    case ENABLE_ALL_MULTICASTS: 
			DEBUGMSG(("if_fd: SIOCADDMULTI ENABLE_ALL_MULTICASTS"));
			error = (*(fdp->nddp->ndd_ctl))
					(fdp->nddp, NDD_ENABLE_MULTICAST, addr,
						FDDI_ADDRLEN);
			if(error) {
				int rc;
				DEBUGMSG(("if_fd:ENABLE_MULTICAST err %d", error));
				driver_delmulti(ifr, &(fdp->fd_ac), 
					fddi_map_ip_multicast, &rc, addr);
			}
			break;

		    case 0:
			DEBUGMSG(("if_fd: SIOCADDMULTI address already enabled"));
			break;
		    case -1:
			DEBUGMSG(("if_fd: ioctl SIOCADDMULTI error %d", error));
			break;
		}
		break;
		}

	    case SIOCDELMULTI:
		{
		register struct ifreq *ifr = (struct ifreq *)data;
		void fddi_map_ip_multicast();
		char addr[6];
		/*
		 * Update our multicast list.
		 */
		switch(driver_delmulti(ifr, &(fdp->fd_ac), 
			fddi_map_ip_multicast, &error, addr))
		{
		    case DEL_ADDRESS:
			DEBUGMSG(("if_fd: ioctl SIOCDELMULTI DEL_ADDRESS\n"));
		        error = (*(fdp->nddp->ndd_ctl))
				(fdp->nddp, NDD_DISABLE_ADDRESS, addr,
					FDDI_ADDRLEN); 
		        break;
	      
	            case DISABLE_ALL_MULTICASTS: 
		      	DEBUGMSG(("if_fd: SIOCDELMULTI DISABLE_ALL_MULTICASTS\n"));
		      	error = (*(fdp->nddp->ndd_ctl))
				(fdp->nddp, NDD_DISABLE_MULTICAST, addr,
					FDDI_ADDRLEN); 
		      	break;
		    case 0:
		      	DEBUGMSG(("if_fd: SIOCDELMULTI address still in use"));
		      	break;
		    case -1:
		      	DEBUGMSG(("if_fd: ioctl SIOCDELMULTI error %d\n", error));
		      	break;
		 }
		 break;
	      }
#endif /* IP_MULTICAST */

	    default:
		error = EINVAL;
	}

	return (error);
}

/*************************************************************************
 *
 * 	fd_output() - output packet to network 
 *
 ************************************************************************/
fd_output(ifp, m, dst, rt)
register struct ifnet *ifp;
register struct mbuf *m;
struct sockaddr_fddi *dst; 
struct rtentry *rt;
{
	register struct fd_softc		*fdp;
	register struct fddi_hdr		*hdrp;
	struct fddi_hdr				hdr;
	register int				mac_len, hdr_len;
	register int				error = 0;
	struct mbuf				*mcopy = 0;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		++ifp->if_snd.ifq_drops;
		goto out;
	}

	if ((ifp->if_flags & IFF_SIMPLEX) && (m->m_flags & M_BCAST))
		mcopy = m_copymext(m, 0, (int)M_COPYALL, M_DONTWAIT);

	fdp = &fd_softc[ifp->if_unit];

	switch (dst->sa_family) {
	    case AF_INET:
		if (!fddi_arpresolve(&fdp->fd_ac, m, dst, &hdr)) {
			m = 0;
			goto out;
		}
		hdrp = (struct fddi_hdr *)&hdr;
		break;

	    case AF_UNSPEC:
		hdrp = (struct fddi_hdr *) &dst->sa_data;
		break;

	    default:
		if (af_table[dst->sa_family].config.resolve == NULL) {
			error = EAFNOSUPPORT;
			goto out;
		}
		if (!(*af_table[dst->sa_family].config.resolve)
		      (&fdp->fd_ac, m, dst, (caddr_t)&hdr)) {
			m = 0;
			goto out;
		}
		hdrp = (struct fddi_hdr *)&hdr;
		break;
	}

        /*
         * Add local net header.  If no space in first mbuf,
         * allocate another.
         */
	mac_len = mac_size_f(&hdrp->fddi_mac);
	hdr_len = mac_len + sizeof(struct ie2_llc_snaphdr);
	M_PREPEND(m, hdr_len, M_DONTWAIT);
	if (!m) {
		error = ENOBUFS;
		++ifp->if_snd.ifq_drops;
		goto out;
	}

	/* 
	 * For the FDDI NDD, the mac hdr starts with 3 reserved bytes,
	 * the first of which must be 0.
	 */
	hdrp->fddi_mac._First.reserved[0] = 0;

	/* copy in the mac and llc headers */
	bcopy((caddr_t)hdrp, mtod(m, caddr_t), hdr_len);

	m = m_collapse(m, FDDI_MAX_GATHERS);
	if (!m) {
		error = ENOBUFS;
		++ifp->if_snd.ifq_drops;
		goto out;
	}
	
	if (m->m_flags & M_BCAST|M_MCAST)
		++ifp->if_omcasts;

	ifp->if_obytes += m->m_pkthdr.len;

	if (!fdp->nddp) {
		m_freem(m);
		++ifp->if_oerrors;
	} else if ((*(fdp->nddp->ndd_output))(fdp->nddp, m)) {
		m_freem(m);
		++ifp->if_oerrors;
	}

	if (mcopy)
		(void) looutput(ifp, mcopy, dst, rt);

	++ifp->if_opackets;
	m = 0;

    out:
	if (m)
		m_freem(m);

	return (error);
}

#ifdef IP_MULTICAST
u_char fddi_ipmulticast_min[6] = { 0x80, 0x00, 0x7a, 0x00, 0x00, 0x00 };
u_char fddi_ipmulticast_max[6] = { 0x80, 0x00, 0x7a, 0xfe, 0xff, 0xff };

void fddi_map_ip_multicast(struct sockaddr_in *sin, u_char *lo, u_char *hi)
{
  if (sin->sin_addr.s_addr == INADDR_ANY) {
    /*
     * An IP address of INADDR_ANY means listen to all
     * of the FDDI multicast addresses used for IP.
     * (This is for the sake of IP multicast routers.)
     */
    bcopy(fddi_ipmulticast_min, lo, 6);
    bcopy(fddi_ipmulticast_max, hi, 6);
  } else {
    FDDI_MAP_IP_MULTICAST(&sin->sin_addr, lo);
    bcopy(lo, hi, 6);
  }

}
#endif /* IP_MULTICAST */

