static char sccsid[] = "@(#)00	1.65.1.27  src/bos/kernext/aixif/if_ie5.c, sysxaixif, bos41J, 9520A_all 5/11/95 17:20:52";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: ie5_arpinput
 *		ie5_arpresolve
 *		ie5_arpwhohas
 *		in_ie5_arpinput
 *		config_ie5
 *		config_ie5_init
 *		config_ie5_term
 *		ie5_attach
 *		ie5_init
 *		ie5_ioctl
 *		ie5_output
 *		ie5_reset
 *
 *   ORIGINS: 26,27,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/******************************************************************************
 *
 *      if_ie5.c - token ring network interface to aix device handler.
 *		 - Address Resolution Protocol for IEEE 802.5 networks
 * 
 ******************************************************************************/


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/nettrace.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <aixif/net_if.h>
#include <net/nd_lan.h>
#include <netinet/if_802_5.h>

#include <netinet/in.h>
#include <netinet/in_netarp.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <net/if_dl.h>
#include <netinet/ip.h>

#include <net/if_arp.h>

#ifdef IP_MULTICAST
#if INETPRINTFS
#define DEBUGMSG(x) { if(inetprintfs > 1) printf x ; }
#else
#define DEBUGMSG(x)
#endif
#include <netinet/if_mcast_aux.h>
#endif /* IP_MULTICAST */

/******************************************************************************
 * Token Ring software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ie5_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 ******************************************************************************/
struct	ie5_softc {
	struct	arpcom ie5_ac;		/* Token ring common part	*/
	struct	ndd *nddp;
};
struct ie5_softc *ie5_softc = (struct ie5_softc *)NULL;

#define	ie5_if		ie5_ac.ac_if	/* network-visible interface */
#define	ie5_addr	ie5_ac.ac_traddr /* hardware token ring addr  */

int			if_tr_lock=LOCK_AVAIL;	/* attach locking        */
int  init = 0;

#ifdef IP_MULTICAST	
int ie5_old_multicast_mapping; /* patch to one, if broadcast should be used */
int ie5_old_multicast_mapping_dflt=0; 
u_char	ie5_multicastaddr[6] = { 0xc0, 0x00, 0x00, 0x20, 0x00, 0x00 };
#endif

/*************************************************************************
 *
 *	config_ie5() - token ring kernel extension entry point
 *
 ************************************************************************/
config_ie5(cmd, uio)
int		cmd;
struct uio	*uio;
{
	struct	device_req		device;
	int				error;
	int				unit;
	char				*cp;

	lockl(&if_tr_lock, LOCK_SHORT);

	if (init == 0) {
		pincode(config_ie5);
		if (ifsize <= 0)
			ifsize = IF_SIZE;
		ie5_softc = (struct ie5_softc *)
		    xmalloc(sizeof(struct ie5_softc)*ifsize, 2, pinned_heap);
		if (ie5_softc == (struct ie5_softc *)NULL) {
		    unpincode(config_ie5);
		    unlockl(&if_tr_lock);
		    return(ENOMEM);
		}
		bzero(ie5_softc, sizeof(struct ie5_softc) * ifsize);
		init++;
	}

	if ( cmd != CFG_INIT ) {
		error = EINVAL;
		goto out;
	}

	if (uio) {	
		error = uiomove((caddr_t) &device, (int) sizeof(device), 
			UIO_WRITE, uio);
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

	error = config_ie5_init(&device, unit);
out:
	unlockl(&if_tr_lock);

	return(error);
}

/*************************************************************************
 *
 *	config_ie5_init() - token ring load initialisation
 *
 ************************************************************************/
config_ie5_init(device, unit)
register struct	device_req	*device;
int				unit;
{
	register struct ie5_softc	*ie5p;
	int				error;

	ie5p = &ie5_softc[unit];
	error = ns_alloc(device->dev_name, &ie5p->nddp);
	if (error) {
		bsdlog(LOG_ERR, 
			"if_tr: ns_alloc(%s) failed with errno = %d\n", 
			device->dev_name, error);
		return(error);
	}

	return(ie5_attach(unit));
}

 /****************************************************************************
 * ie5_attach -	logically attach the token ring network interface
 ****************************************************************************/
ie5_attach(unit)
unsigned unit;
{
	register struct ifnet		*ifp;
	register struct ie5_softc	*ie5p;
	int				ie5_init();
	int				ie5_output();
	int				ie5_ioctl();
	int				rc;

	NETTRC1(HKWD_IFTR|hkwd_attach_in, unit);

	if (unit >= ifsize) {
		NETTRC(HKWD_IFTR|hkwd_attach_out);
		return ENXIO;
	}

	ie5p		= &ie5_softc[unit];
	ifp             = &ie5_softc[unit].ie5_if;

	/* Check if the adapter supports local echo */
	if (ie5p->nddp->ndd_flags & NDD_SIMPLEX)
		ifp->if_flags 	= IFF_BROADCAST | IFF_ALLCAST | IFF_SIMPLEX;
	else
		ifp->if_flags	= IFF_BROADCAST | IFF_ALLCAST;

#ifdef IP_MULTICAST
	ifp->if_flags 	|= IFF_MULTICAST;
	ACMULTI_LOCKINIT(&(ie5p->ie5_ac));
#endif
	bcopy(ie5p->nddp->ndd_physaddr, ie5p->ie5_addr, 6);
	
	ifp->if_mtu	= _802_5_MTU;
	ifp->if_unit	= unit;
	ifp->if_name	= "tr";
	ifp->if_init	= ie5_init;
	ifp->if_output	= ie5_output;
	ifp->if_ioctl	= ie5_ioctl;
	ifp->if_type 	= IFT_ISO88025;
        ifp->if_addrlen = 6;
        ifp->if_hdrlen  = sizeof(struct ie5_mac_hdr)
				+ sizeof(struct ie2_llc_hdr);
	ifp->if_flags	|= IFF_BPF;	/* Enable bpf support */

	ifp->if_arpres = ie5_arpresolve;
	ifp->if_arpinput = ie5_arpinput;

	if_attach(ifp);
	if_nostat(ifp);
#ifdef IP_MULTICAST
	/* 
	 * enable the device driver to listen to the MULTICAST  
	 * functional address. (may become necessary after a token ring 
	 * driver unload and reload)
	 */
	if(ie5p->ie5_ac.ac_multiaddrs) {
	  (*(ie5p->nddp->ndd_ctl))
	    (ie5p->nddp, NDD_ENABLE_ADDRESS, ie5_multicastaddr, IE8025_ADDRLEN); 
	}
#endif /* IP_MULTICAST */

	NETTRC(HKWD_IFTR|hkwd_attach_out);
	return(0);
}

/***************************************************************************** 
 * ie5_init -	logically initialize token ring network interface
 ****************************************************************************/ 
ie5_init()
{
	register struct ifnet		*ifp;
	int				unit;

	NETTRC(HKWD_IFTR|hkwd_init_in);

	for ( unit = 0; unit < ifsize; unit ++ ) {
		ifp  = &ie5_softc[unit].ie5_if;

		/* not yet, if address still unknown */
		if (ifp->if_addrlist == (struct ifaddr *)0)
			continue;
		ifp->if_flags |= IFF_RUNNING;
	}

	NETTRC(HKWD_IFTR|hkwd_init_out);
}

/***************************************************************************** 
 * Process an ioctl request.
 *
 * NOTE:
 *	the error code returned from this routine is ignored by the caller.
 ****************************************************************************/ 
ie5_ioctl(ifp, cmd, data)
	register struct ifnet		*ifp;
	int				cmd;
	caddr_t				data;
{
	register struct ifaddr		*ifa = (struct ifaddr *)data;
	register struct ie5_softc	*ie5p = &ie5_softc[ifp->if_unit];
	int				error = 0;
	struct timestruc_t		ct;

	NETTRC3(HKWD_IFTR|hkwd_ioctl_in, ifp, cmd, data);

	if (!ie5p->nddp)
		return(ENODEV);

	switch (cmd) {

	    case SIOCIFDETACH:
		ns_free(ie5p->nddp);
		ie5p->nddp = 0;
		break;

	    case IFIOCTL_ADD_FILTER:
		{
		ns_8022_t	*filter;
		ns_user_t	*user;

		filter = &((struct if_filter *)data)->filter;
		user = &((struct if_filter *)data)->user;
		error = ns_add_filter(ie5p->nddp, filter, sizeof(*filter), 
			user);
		if (error && error != EALREADY)
			bsdlog(LOG_ERR, "if_tr ns_add_filter() failed with %d return code.\n", error);
		break;
		}

	    case IFIOCTL_DEL_FILTER:
		ns_del_filter(ie5p->nddp, (ns_8022_t *)data, sizeof(ns_8022_t));
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
		ie5_init();
		ifp->if_flags |= IFF_UP;
		curtime (&ct);
		ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
		ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
		break;

	    case SIOCSIFFLAGS:
		ie5_init();
		break;

#ifdef IP_MULTICAST
	    case SIOCADDMULTI:
	    case SIOCDELMULTI:
		{
		  register struct ifreq *ifr = (struct ifreq *)data;
		  void ie5_map_ip_multicast();
		  int rc;
		  char mac_addr[6];
		  /*
		   * Update our multicast list.
		   */
		  DEBUGMSG(("if_ie5: ioctl SIOC%sMULTI\n", (cmd == SIOCADDMULTI) ? "ADD" : "DEL"));
		  rc = (cmd == SIOCADDMULTI) ?
		    driver_addmulti(ifr, &(ie5p->ie5_ac), ie5_map_ip_multicast, &error, mac_addr):
		    driver_delmulti(ifr, &(ie5p->ie5_ac), ie5_map_ip_multicast, &error, mac_addr);

		  switch(rc) {
		  case ENABLE_ALL_MULTICASTS:
		  case ADD_ADDRESS:
		    /*
		     * Multicast list has changed; set the hardware filter
		     */
		    error = (*(ie5p->nddp->ndd_ctl))
		      (ie5p->nddp, NDD_ENABLE_ADDRESS, ie5_multicastaddr,
				IE8025_ADDRLEN); 

		    if(error) {
		      int rc;
		      driver_delmulti(ifr, &(ie5p->ie5_ac), ie5_map_ip_multicast, &rc, mac_addr);
		    }
		    DEBUGMSG(("if_ie5: ioctl SIOCADDMULTI finished setting hardware filter error=%d\n", error));
		    break;

		  case DISABLE_ALL_MULTICASTS:
		  case DEL_ADDRESS:
		    if(!ie5p->ie5_ac.ac_multiaddrs) { /* OK, we can really get out of this mode */
		      /*
		       * reset the functional address
		       */
		      error = (*(ie5p->nddp->ndd_ctl))
			(ie5p->nddp, NDD_DISABLE_ADDRESS, ie5_multicastaddr,
				IE8025_ADDRLEN); 
		      DEBUGMSG(("if_ie5: ioctl SIOCDELMULTI finished setting hardware filter error=%d\n", error));	   
		    }
		    break;
		  } 
	        }
		break;
#endif /* IP_MULTICAST */

	    default:
		error = EINVAL;
	}

	NETTRC1(HKWD_IFTR|hkwd_ioctl_out, error);
	return (error);
}

/***************************************************************************** 
 * Token ring output routine.
 * Encapsulate a packet of type family for the local net.
 ****************************************************************************/ 
ie5_output(ifp, m, dst, rt)
register struct ifnet *ifp;
register struct mbuf *m;
struct sockaddr_802_5 *dst;
struct rtentry *rt; {
	register struct ie5_softc	*ie5p;
	struct ie5_hdr			ie5_hdr, *ie5_hdrp;
	int				mac_len, hdr_len;
	int				error = 0;
	struct mbuf			*mcopy = 0;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		ifp->if_snd.ifq_drops++;
		goto out;
	}

	if ((ifp->if_flags & IFF_SIMPLEX) && (m->m_flags & M_BCAST))
		mcopy = m_copymext(m, 0, (int)M_COPYALL, M_DONTWAIT);

	ie5p = &ie5_softc[ifp->if_unit];

	switch (dst->sa_family) {
	    case AF_INET:
		if (!ie5_arpresolve(&ie5p->ie5_ac, m, dst, &ie5_hdr)) {
			m = 0;
			goto out;
		}
		ie5_hdrp = &ie5_hdr;
		break;

	    case AF_UNSPEC:
		ie5_hdrp = (struct ie5_hdr *) &dst->sa_data;
		break;

	    default:
		if (af_table[dst->sa_family].config.resolve == NULL) {
			error = EAFNOSUPPORT;
			goto out;
		}
		if (!(*af_table[dst->sa_family].config.resolve)
		      (&ie5p->ie5_ac, m, dst, &ie5_hdr)) {
			m = 0;
			goto out;
		}
		ie5_hdrp = &ie5_hdr;
		break;
	}
	mac_len = mac_size(&ie5_hdrp->ie5_mac);
	hdr_len = mac_len + sizeof(struct ie2_llc_snaphdr);

        /*
         * Add local net header.  If no space in first mbuf,
         * allocate another.
         */
	M_PREPEND(m, hdr_len, M_DONTWAIT);
	if (m == 0) {
		error = ENOBUFS;
		++ifp->if_snd.ifq_drops;
		goto out;
        }
	
	/* copy in the mac and llc headers. */
	bcopy((caddr_t)ie5_hdrp, mtod(m, caddr_t), hdr_len);

	if (m->m_flags & (M_BCAST|M_MCAST))
		++ifp->if_omcasts;
	ifp->if_obytes += m->m_pkthdr.len;

	if (!ie5p->nddp) {
		m_freem(m);
		++ifp->if_oerrors;
	} else if ((*(ie5p->nddp->ndd_output))(ie5p->nddp, m)) {
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

	NETTRC1(HKWD_IFTR|hkwd_output_out, error);
	return (error);
}

/***************************************************************************** 
 * ie5_reset() - reset token ring network interface
 ****************************************************************************/ 
ie5_reset()
{
	int	unit;

	for ( unit = 0; unit < ifsize; unit ++ ) {
		ie5_softc[unit].ie5_if.if_flags &= ~IFF_RUNNING;
	}
	ie5_init();
}

#ifdef IP_MULTICAST
void ie5_map_ip_multicast(struct sockaddr_in *sin, u_char *lo, u_char *hi)
{
  if (sin->sin_addr.s_addr == INADDR_ANY) {
    /*
     * An IP address of INADDR_ANY means listen to all
     * of the tokenring multicast addresses used for IP.
     * (This is for the sake of IP multicast routers.)
     */
    bcopy(ie5_multicastaddr, lo, 6);
    bcopy(ie5_multicastaddr, hi, 6);
  } else {
    IEEE802_5_MAP_IP_MULTICAST(&sin->sin_addr, lo);
    bcopy(lo, hi, 6);
  }
}
#endif /* IP_MULTICAST */

/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
ie5_arpwhohas(ac, addr)
	register struct arpcom *ac;
	struct in_addr *addr;
{
	register struct mbuf *m;
	register struct ie5_arp *ta;
	register struct ie5_hdr *hdrp;
	struct sockaddr_802_5 sa;

	if ((m = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL)
		return;
	m->m_len = sizeof(*ta);
	m->m_pkthdr.len = sizeof(*ta);
	m->m_flags |= M_BCAST;
	MH_ALIGN(m, sizeof(*ta));


	hdrp = (struct ie5_hdr *)&sa.sa_data;

	hdrp->ie5_mac.mac_acf = ACF_PRIORITY3;
	hdrp->ie5_mac.mac_fcf = FCF_LLC_FRAME;

	/*
	 * destination is BROADCAST
	 */
	bcopy((caddr_t) ie5_broadcastaddr 
		, (caddr_t) hdrp->ie5_mac.mac_dst_802_5
		, sizeof (hdrp->ie5_mac.mac_dst_802_5));

	/*
	 * source is me
	 */
	bcopy((caddr_t) ac->ac_traddr
		, (caddr_t) hdrp->ie5_mac.mac_src_802_5
		, sizeof (hdrp->ie5_mac.mac_src_802_5));

	hdrp->ie5_mac.mac_src_802_5[0] |= RI_PRESENT;

	/*
	 * If the interface mtu is greater than the max frame size for
	 * 4 Mbps 802.5, we assume they're doing 16 Mbps and broadcast
	 * the max size for that (we don't always do this since older
	 * machines will reject it).
	 */
        hdrp->ie5_mac.mac_rcf = (ac->ac_if.if_mtu + 
		sizeof(struct ie2_llc_snaphdr) <=
		RCF_FRAME4_MAX ? RCF_FRAME4 : RCF_FRAME6) |
		sizeof (hdrp->ie5_mac.mac_rcf) << 8;
        if (ac->ac_if.if_flags & IFF_ALLCAST)
                hdrp->ie5_mac.mac_rcf |= RCF_ALL_BROADCAST;


	ta = mtod(m, struct ie5_arp *);

	bzero((caddr_t)ta, sizeof (*ta));

	ta->arp_hrd = htons(ARPHRD_802_5);
	ta->arp_pro = htons(_802_5_TYPE_IP);
	ta->arp_hln = sizeof(ta->arp_sha);	/* hardware address length */
	ta->arp_pln = sizeof(ta->arp_spa);	/* protocol address length */
	ta->arp_op = htons(ARPOP_REQUEST);

	bcopy((caddr_t)ac->ac_traddr, (caddr_t)ta->arp_sha,
	   sizeof(ta->arp_sha));
	bcopy((caddr_t)&ac->ac_ipaddr, (caddr_t)ta->arp_spa,
	   sizeof(ta->arp_spa));
	bcopy((caddr_t)addr, (caddr_t)ta->arp_tpa, sizeof(ta->arp_tpa));
	ie2_llc(snd_mac_to_llc(&hdrp->ie5_mac), _802_5_TYPE_ARP);

	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}

/*
 * ie5_arpresolve -	Token ring Address Resolution Protocol
 *
 * Input:
 *	ac		-	token ring common structure for interface
 *	m		-	mbuf chain of intended sendee
 *	dst		-	destination sockaddr
 *	macp		-	pointer to mac header. *macp must be able
 *				hold largest mac and llc.
 *
 * Returns:
 *	!0		-	 OK to send packet
 *	 0		-	!OK to send packet
 *
 * Resolve an IP address into an token ring address.  If success, 
 * macp will contain the MAC and LLC headers required to send the pkt.
 * (the output routine must still deal with the variable size of the MAC)
 * If there is no entry in arptab, set one up and broadcast an ARP request 
 * for the IP address. Hold onto this mbuf and resend it once the address
 * is finally resolved.
 *
 */
ie5_arpresolve(ac, m, dst, macp)
	struct arpcom *ac;
	struct mbuf *m;
	struct sockaddr_in *dst;
	caddr_t macp;
{
	register struct arptab *at;
	struct in_addr *destip;
	struct sockaddr_in sin;
	register struct in_ifaddr *ia;
	caddr_t dest;
	int s;

	destip = &dst->sin_addr;

	/*
	 * This hook needed by NETPMON...
	 */
	NETTRC4(HKWD_IFTR|hkwd_output_in, &(ac->ac_if), m, dst->sin_family,
		destip->s_addr);

#ifdef IP_MULTICAST
	/*
	 * check for internet multicast.
	 */
	if (IN_MULTICAST(ntohl(destip->s_addr))) {
		u_short rcf;

		dest = (caddr_t) (ie5_old_multicast_mapping) ? 
			ie5_broadcastaddr : ie5_multicastaddr;

		/* broadcast multicast packets also over bridges */
                rcf =  RCF_FRAME2 | sizeof(rcf) << 8;
                if (ac->ac_if.if_flags & IFF_ALLCAST)
                        rcf |= RCF_ALL_BROADCAST;

		IE5_FILLIN_HDR((struct ie5_mac_hdr *)macp, ac->ac_traddr, 
			dest, rcf, (caddr_t)NULL);
		return 1;
	} else
#endif /* IP_MULTICAST */

	/*
	 * check for internet broadcast.
	 */
 	if (m->m_flags & M_BCAST) {
		u_short rcf;

                rcf =  RCF_FRAME2 | sizeof(rcf) << 8;
                if (ac->ac_if.if_flags & IFF_ALLCAST)
                        rcf |= RCF_ALL_BROADCAST;
		IE5_FILLIN_HDR((struct ie5_mac_hdr *)macp, ac->ac_traddr, 
			(caddr_t) ie5_broadcastaddr, rcf, (caddr_t)NULL);
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

				IE5_FILLIN_HDR((struct ie5_mac_hdr *)macp, 
					ac->ac_traddr, ac->ac_traddr,
					0, (caddr_t)NULL);
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
		 * ifp->if_flags & IFF_NOARP is ignored for token ring
		 */
		at = arptnew(destip, &ac->ac_if);
		at->at_hold = m;
		ARPTAB_UNLOCK(&arptab_lock);
		ie5_arpwhohas(ac, destip);
		return (0);
	}
	if (at->at_flags & ATF_COM) {		/* entry IS complete */
		IE5_FILLIN_HDR((struct ie5_mac_hdr *)macp, ac->ac_traddr, 
			(caddr_t)at->at_traddr, at->at_rcf, at->at_seg);
		ARPTAB_UNLOCK(&arptab_lock);
		return !0;
	}
	/*
	 * There is an arptab entry, but no token ring address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (at->at_hold) {
		m_freem(at->at_hold);
		ac->ac_if.if_arpdrops++;
	}

	at->at_hold = m;
	if (!(at->at_flags & ATF_ASK)) { 	/* Free to ask again */
		at->at_flags |= ATF_ASK;
		ie5_arpwhohas(ac, destip);
	}
	ARPTAB_UNLOCK(&arptab_lock);
	return (0);
}

/*
 * ie5_arpinput -	token ring ARP input
 *
 * Input:
 *	ac	-	^ to interface ARP common structure
 *	pkt	-	^ to MAC/LLC/ARP data
 *
 * Called from Token ring interrupt handlers
 * when token ring packet type _802_5_TYPE_ARP
 * is received.  Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
ie5_arpinput(ac, m, hp)
	struct arpcom *ac;
	register struct mbuf *m;
	register caddr_t  hp;
{
	register struct arphdr *ar;
	register struct ie2_llc_snaphdr *llc;
	register struct ie5_mac_hdr *mac;

	if (ac->ac_if.if_flags & IFF_NOARP) { 
		NETTRC(HKWD_NETERR|122);
                goto out;
        }

	mac = (struct ie5_mac_hdr *)hp;

	/*
	 * Well, this test says that we should ignore arp broadcasts from 
	 * foreign token rings when ALLCAST is not set.
	 * 'localbroadcast' means that when you send on the token ring,
	 * you only send to the local ring, and not through any bridge.
	 * Route_bytes > 2 means that the packet has passed through at 
	 * least one bridge.
	 */
	if ((ac->ac_if.if_flags & IFF_ALLCAST) == 0) 
		if(has_route(mac) && (route_bytes(mac) > 2)) {
			NETTRC(HKWD_NETERR|123);
			goto out;
		}

	llc = rcv_mac_to_llc(mac);
	ar  = mtod(m, struct arphdr *);

	if (ntohs(ar->ar_hrd) != ARPHRD_802_5) {
		net_error(&ac->ac_if, ARP_WRONG_HDR, 0);
		goto out;
	}

	if (has_route(mac)) {
		int pdu_max;

		/*
		 * deal with any routing information.
		 * make sure that the route will allow us to send the frames
		 * of a size advertised by the interface.
		 *
		 * Magic numbers from IBM Token-Ring Network Arch. 
		 */
		switch (largest_frame(mac)) {
		    case RCF_FRAME_MASK:
		    case RCF_FRAME6:	/* ISO 8802/5 (16 Mbps) max	*/
			pdu_max = RCF_FRAME6_MAX;
			break;

		    case RCF_FRAME5:	
			pdu_max = RCF_FRAME5_MAX;
			break;

		    case RCF_FRAME4:	/* ISO 8802/4 max		*/
			pdu_max = RCF_FRAME4_MAX;
			break;

		    case RCF_FRAME3:	/* FDDI and ISO 8802/5 max	*/
			pdu_max = RCF_FRAME3_MAX;
			break;

		    case RCF_FRAME2:	/* ???				*/
			pdu_max = RCF_FRAME2_MAX;
			break;

		    case RCF_FRAME1:	/* ISO 8802/3 max		*/
			pdu_max = RCF_FRAME1_MAX;
			break;

		    case RCF_FRAME0:	/* ISO 8802/2 max		*/
			pdu_max = RCF_FRAME0_MAX;
			break;

		    default:
			NETTRC1(HKWD_NETERR|124, largest_frame(mac));
			pdu_max = RCF_FRAME3_MAX;
			break;
		}

		/*
		 * reverse the direction of the route, and remove
		 * any broadcast bits.
		 */
		mac->mac_rcf ^=  RCF_DIRECTION;
		mac->mac_rcf &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}

	switch (ntohs(ar->ar_pro)) {
	    case _802_5_TYPE_IP:
		in_ie5_arpinput(ac, m, hp);
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
 * ARP for Internet protocols on token ring.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 */

in_ie5_arpinput(ac, m, hp)
	register struct arpcom *ac;
	struct mbuf *m;
	caddr_t hp;
{
	struct ie5_hdr *hdrp;
	register struct ie5_arp *ta;
	register struct arptab *at, *tat;  /* same as "merge" flag */
	register struct in_ifaddr *ia;
	struct in_ifaddr *maybe_ia = 0;
	struct sockaddr_in sin;
	struct sockaddr_802_5 sa;
	struct in_addr isaddr, itaddr, myaddr;
	int proto, op;
	register struct mbuf *m0;

	int s;

	hdrp = (struct ie5_hdr *)hp;

	ta     = mtod(m, struct ie5_arp *);
	proto  = ntohs(ta->arp_pro);
	op     = ntohs(ta->arp_op);
	bcopy(&((struct in_addr *)ta->arp_spa)->s_addr, &isaddr.s_addr 
		, sizeof (isaddr.s_addr));
	bcopy(&((struct in_addr *)ta->arp_tpa)->s_addr, &itaddr.s_addr 
		, sizeof (itaddr.s_addr));
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == &ac->ac_if) {
			maybe_ia = ia;
			if ((itaddr.s_addr == ia->ia_addr.sin_addr.s_addr) ||
			     (isaddr.s_addr == ia->ia_addr.sin_addr.s_addr))
				break;
		}
	if (maybe_ia == 0)
		goto out;
	myaddr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;
	if (!bcmp((caddr_t)ta->arp_sha, (caddr_t)ac->ac_traddr
		      , sizeof (ta->arp_sha)))
		goto out;	/* it's from me, ignore it. */
	if (!bcmp((caddr_t)ta->arp_sha, (caddr_t)ie5_broadcastaddr
		      , sizeof (ta->arp_sha))) {
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

		if (!has_route(&hdrp->ie5_mac) || !at->at_rcf || 
			(has_route(&hdrp->ie5_mac) && 
			(route_bytes(&hdrp->ie5_mac) <
			((at->at_rcf >> 8) & 0x1f)))) { 
	    add_data:
			bcopy((caddr_t)ta->arp_sha, (caddr_t)at->at_traddr,
		    		sizeof(ta->arp_sha));
			at->at_flags |= ATF_COM;

			/*
		 	 * snarf any routing info
		 	 */
			if (has_route(&hdrp->ie5_mac) && 
				route_bytes(&hdrp->ie5_mac) > 
				sizeof (hdrp->ie5_mac.mac_rcf)) {
				at->at_rcf = hdrp->ie5_mac.mac_rcf;
				bcopy(hdrp->ie5_mac.mac_seg, at->at_seg, 
                                      sizeof (at->at_seg));
			} else
				at->at_rcf = 0;

			if (at->at_hold) {
				struct mbuf *om;

				sin.sin_family = AF_INET;
				sin.sin_addr = isaddr;
				om = at->at_hold;
				at->at_hold = 0;
				ARPTAB_UNLOCK(&arptab_lock);
				(*ac->ac_if.if_output)(&ac->ac_if, om,
				  (struct sockaddr_802_5 *)&sin, (struct rtentry *)0);
				goto out;
			}
		}
		else
 			NETTRC3(HKWD_NETERR|125,has_route(&hdrp->ie5_mac),
				at->at_rcf, route_bytes(&hdrp->ie5_mac));

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
	    case _802_5_TYPE_IP:
		/*
		 * Reply if this is an IP request.
		 */
		if (op != ARPOP_REQUEST)
			goto freeout;
		break;

	    default:
		NETTRC1(HKWD_NETERR|126, proto);
		goto freeout;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t)ta->arp_sha, (caddr_t)ta->arp_tha,
		    sizeof(ta->arp_sha));
		bcopy((caddr_t)ac->ac_traddr, (caddr_t)ta->arp_sha,
		    sizeof(ta->arp_sha));
	} else {
		ARPTAB_LOOK(tat, itaddr.s_addr);
		if (tat == NULL || (tat->at_flags & ATF_PUBL) == 0) {
 			if (tat != NULL)
                                NETTRC2(HKWD_NETERR|127, tat, tat->at_flags);
			goto freeout; 
		}

		bcopy((caddr_t)ta->arp_sha, (caddr_t)ta->arp_tha,
		    sizeof(ta->arp_sha));
		bcopy((caddr_t)tat->at_traddr, (caddr_t)ta->arp_sha,
		    sizeof(ta->arp_sha));
	}

	bcopy((caddr_t)ta->arp_spa, (caddr_t)ta->arp_tpa,
	    sizeof(ta->arp_spa));
	bcopy((caddr_t)&itaddr, (caddr_t)ta->arp_spa,
	    sizeof(ta->arp_spa));
	ta->arp_op = htons(ARPOP_REPLY); 

        if ((m0 = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL) {
             	NETTRC(HKWD_NETERR|128);
                goto freeout;
	}

	m0->m_len = sizeof(*ta);
	m0->m_pkthdr.len = sizeof(*ta);
	MH_ALIGN(m0, sizeof(*ta));
	*mtod(m0, struct ie5_arp *) = *ta;
	ta = mtod(m0, struct ie5_arp *);

	ta->arp_pro = htons(_802_5_TYPE_IP);

	/*
	 * create MAC/LLC stuff
	 */
	hdrp = (struct ie5_hdr *)&sa.sa_data;
	hdrp->ie5_mac.mac_acf = ACF_PRIORITY3;
	hdrp->ie5_mac.mac_fcf = FCF_LLC_FRAME;

	bcopy((caddr_t) ta->arp_tha
		, (caddr_t) hdrp->ie5_mac.mac_dst_802_5
		, sizeof (hdrp->ie5_mac.mac_dst_802_5));

	bcopy((caddr_t) ac->ac_traddr
		, (caddr_t) hdrp->ie5_mac.mac_src_802_5
		, sizeof (hdrp->ie5_mac.mac_src_802_5));

	/*
	 * determine what routing control is required to respond
	 */
	if (at->at_rcf) {
		hdrp->ie5_mac.mac_src_802_5[0] |= RI_PRESENT;
		hdrp->ie5_mac.mac_rcf     = at->at_rcf;

		bcopy((caddr_t) at->at_seg
			, hdrp->ie5_mac.mac_seg
			, sizeof (hdrp->ie5_mac.mac_seg));
	} else
		hdrp->ie5_mac.mac_src_802_5[0] &= ~RI_PRESENT;
	ie2_llc(snd_mac_to_llc(&hdrp->ie5_mac), _802_5_TYPE_ARP);
	ARPTAB_UNLOCK(&arptab_lock);
	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m0, &sa, (struct rtentry *)0);
	goto out;

freeout:
	ARPTAB_UNLOCK(&arptab_lock);
out:
	m_freem(m);
	return;
}


