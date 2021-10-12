static char sccsid[] = "@(#)99        1.94  src/bos/kernext/aixif/if_en.c, sysxaixif, bos41J, 9520A_all 5/10/95 18:57:40";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: arpinput
 *		arpresolve
 *		arpwhohas
 *		ether_sprintf
 *		in_arpinput
 *		revarpinput
 *		config_en
 *		en_attach
 *		en_ioctl
 *		en_output
 *		ether_map_ip_multicast
 *
 *   ORIGINS: 26,27,85,89
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
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/* Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution is only permitted until one year after the first shipment
 * of 4.4BSD by the Regents.  Otherwise, redistribution and use in source and
 * binary forms are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      (#)if_ether.c   7.12 (Berkeley) 6/28/90
 */


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>		/* u_char, u_long and more	*/
#include <sys/mbuf.h>		/* mbuf structure and macros	*/
#include <sys/socket.h>		/* sockaddrs and such		*/
#include <sys/ioctl.h>		/* SIOC's			*/
#include <sys/errno.h>		/* error values			*/
#include <sys/malloc.h>		/* xmalloc			*/
#include <sys/device.h>		/* for CFG_INIT, CFG_TERM	*/
#include <sys/uio.h>		/* uio and iovec structures	*/
#include <sys/lockl.h>		/* for lockl and unlockl	*/
#include <sys/nettrace.h>	/* trace defs			*/
#include <sys/time.h>		/* for curtime()...	        */
#include <sys/syslog.h>		/* for bsdlog()			*/

#include <net/if.h>		/* ifnet			*/
#include <sys/ndd.h>		/* ndd				*/
#include <sys/cdli.h>		/* network services		*/
#include <net/nd_lan.h>		/* lan specific net services	*/
#include <netinet/if_ether.h>	/* ETHERMIN, ether header...	*/
#include <net/nh.h>
#include <net/if_types.h>	/* all the interface types	*/

#include <netinet/in.h>		/* for sockaddr_in ...		*/
#include <netinet/in_systm.h>
#include <netinet/in_var.h>	/* IA_SIN macro			*/

#include <aixif/net_if.h>
#include <netinet/ip.h>

#include <net/if_arp.h>

#ifdef IP_MULTICAST
#include <netinet/if_mcast_aux.h>
#endif

/*****************************************************************************
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * en_if, which the routing code uses to locate the interface.
 * This structure contains the hardware address, device interface structure...
 ****************************************************************************/
struct	en_softc {
	struct	arpcom en_ac;	/* Ethernet common part		*/
	struct ndd	*nddp;
};
struct en_softc *en_softc = (struct en_softc *)NULL;
struct en_softc *et_softc = (struct en_softc *)NULL;

#define	en_if		en_ac.ac_if	/* network-visible interface */
#define	en_addr		en_ac.ac_enaddr	/* hardware Ethernet address */

int			if_en_lock = LOCK_AVAIL; /* attach locking	*/
int 			init = 0;

int arpinput();
int revarpinput();

/*************************************************************************
 *
 *	config_en() - ethernet kernel extension entry point
 *
 ************************************************************************/
config_en(cmd, uio)
int		cmd;
struct uio	*uio;
{
	struct	device_req		device;
	int				error = 0;
	int				unit;
	struct 	en_softc		*enp;
	char				*cp;
	int				type;

	if ( (cmd != CFG_INIT) || (uio == NULL) )
		return(EINVAL);

	if (uiomove((caddr_t) &device, (int)sizeof(device), UIO_WRITE, uio))
		return(EFAULT);

	lockl(&if_en_lock, LOCK_SHORT);
	if (init == 0) {
		if (ifsize <= 0)
			ifsize = IF_SIZE;
		if (error = pincode(config_en))	
			goto out;
		en_softc = (struct en_softc *)
			xmalloc(sizeof(struct en_softc)*ifsize, 2, pinned_heap);
		et_softc = (struct en_softc *)
			xmalloc(sizeof(struct en_softc)*ifsize, 2, pinned_heap);
		if ( (en_softc == (struct en_softc *)NULL) ||
		     (et_softc == (struct en_softc *)NULL) ) {
			unpincode(config_en);
			unlockl(&if_en_lock);
			return(ENOMEM);
		}
		bzero(en_softc, sizeof(struct en_softc) * ifsize);
		bzero(et_softc, sizeof(struct en_softc) * ifsize);
		init++;
	}

	cp = device.dev_name;
	while(*cp < '0' || *cp > '9') cp++;
	unit = atoi(cp);

	if (unit >= ifsize) {
		error = ENXIO;
		goto out;
	}

	if (!strncmp(device.dev_name, "en", 2)) {
		type = IFT_ETHER;
		enp = &en_softc[unit];
	} else if (!strncmp(device.dev_name, "et", 2)) {
		type = IFT_ISO88023;
		device.dev_name[1] = 'n'; /* hee hee */
		enp = &et_softc[unit];
	} else {
		error = EINVAL;
		goto out;
	}

	error = ns_alloc(device.dev_name, &enp->nddp);
	if (error == 0)
		en_attach(unit, type);
	else
		bsdlog(LOG_ERR, 
			"if_en: ns_alloc(%s) failed with errno = %d\n", 
			device.dev_name, error);

out:
	unlockl(&if_en_lock);
	return(error);
}

 /****************************************************************************
 * en_attach -	logically attach the ethernet network interface
 ****************************************************************************/
en_attach(unit, type)
unsigned 		unit;
unsigned 		type;
{
	register struct ifnet 		*ifp;
	register struct en_softc        *enp;
	extern int			en_output();
	extern int			en_ioctl();

	if (type == IFT_ETHER) {
		enp             = &en_softc[unit];
		ifp             = &enp->en_if;
		ifp->if_name    = "en";
		ifp->if_mtu	= ETHERMTU;
	} else {
		enp             = &et_softc[unit];
		ifp             = &enp->en_if;
		ifp->if_name    = "et";
		ifp->if_mtu	= ETHERMTU - 8;
	}
        ifp->if_type    = type;
	ifp->if_unit	= unit;

	bcopy(enp->nddp->ndd_physaddr, enp->en_addr, 6);

	ifp->if_flags 	= IFF_BROADCAST | IFF_NOTRAILERS;
	/* Check if the adapter supports local echo */
	if (enp->nddp->ndd_flags & NDD_SIMPLEX)
		ifp->if_flags 	|= IFF_SIMPLEX;

#ifdef IP_MULTICAST
	ifp->if_flags 	|= IFF_MULTICAST;
	ACMULTI_LOCKINIT(&(enp->en_ac));
#endif
	ifp->if_output	= en_output;
	ifp->if_ioctl	= en_ioctl;
        ifp->if_addrlen = 6;
        ifp->if_hdrlen = 14;

	/* packet filter support */
	ifp->if_flags	|= IFF_BPF;   /* Enable bpf support */
	ifp->if_tap	= NULL;       /* Inactive tap filter */

	ifp->if_arpres = arpresolve;
	ifp->if_arprev = revarpinput;
	ifp->if_arpinput = arpinput;

	if_attach(ifp);
	if_nostat(ifp);
}

/***************************************************************************** 
 * Process an ioctl request.
 ****************************************************************************/ 
en_ioctl(ifp, cmd, data)
	register struct ifnet *		ifp;
	int				cmd;
	caddr_t				data;
{
	register struct ifaddr *	ifa = (struct ifaddr *)data;
	register struct en_softc *	enp = (struct en_softc *)ifp;
	int				error = 0;
	struct timestruc_t		ct;

	if (!enp->nddp)
		return(ENODEV);

	switch (cmd) {

	    case SIOCIFDETACH:
		ns_free(enp->nddp);
		enp->nddp = 0;
		break;

	    case IFIOCTL_ADD_FILTER:
		{
		ns_8022_t	*filter;
		ns_user_t	*user;

		filter = &((struct if_filter *)data)->filter;
		user = &((struct if_filter *)data)->user;
		error = ns_add_filter(enp->nddp, filter, sizeof(*filter), user);
		if (error && error != EALREADY)
			bsdlog(LOG_ERR, "if_en: ns_add_filter() failed with %d return code.\n", error);
		break;
		}

	    case IFIOCTL_DEL_FILTER:
		ns_del_filter(enp->nddp, (ns_8022_t *)data, sizeof(ns_8022_t));
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
		ifp->if_flags |= IFF_UP|IFF_RUNNING;
		curtime (&ct);
		ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
		ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
		break;

	    case SIOCSIFFLAGS:
		ifp->if_flags |= IFF_RUNNING;
		break;

#ifdef IP_MULTICAST
#define DEBUGMSG(x)
	    case SIOCADDMULTI:
		{
		register struct ifreq *ifr = (struct ifreq *)data;
		void ether_map_ip_multicast();
		char 	addr[6];

		/*
		 * Update our multicast list.
		 */
		switch(driver_addmulti(ifr, &(enp->en_ac), 
			ether_map_ip_multicast, &error, addr)) {
		    case ADD_ADDRESS: 
			DEBUGMSG(("if_en: ioctl SIOCADDMULTI ADD_ADDRESS\n"));
			error = (*(enp->nddp->ndd_ctl))
					(enp->nddp, NDD_ENABLE_ADDRESS, addr,
						ETHER_ADDRLEN); 
			if(error) {
				int rc;
				DEBUGMSG(("en: ENABLE_ADDRESS err %d", error));
				driver_delmulti(ifr, &(enp->en_ac), 
					ether_map_ip_multicast, &rc, addr);
			}
			break;
	      
		    case ENABLE_ALL_MULTICASTS: 
			DEBUGMSG(("en: SIOCADDMULTI ENABLE_ALL_MULTICASTS"));
			error = (*(enp->nddp->ndd_ctl))
					(enp->nddp, NDD_ENABLE_MULTICAST, addr,
						ETHER_ADDRLEN);
			if(error) {
				int rc;
				DEBUGMSG(("en:ENABLE_MULTICAST err %d", error));
				driver_delmulti(ifr, &(enp->en_ac), 
					ether_map_ip_multicast, &rc, addr);
			}
			break;

		    case 0:
			DEBUGMSG(("en: SIOCADDMULTI address already enabled"));
			break;
		    case -1:
			DEBUGMSG(("if_en: ioctl SIOCADDMULTI error %d", error));
			break;
		}
		break;
		}

	    case SIOCDELMULTI:
		{
		register struct ifreq *ifr = (struct ifreq *)data;
		void ether_map_ip_multicast();
		char addr[6];
		/*
		 * Update our multicast list.
		 */
		switch(driver_delmulti(ifr, &(enp->en_ac), 
			ether_map_ip_multicast, &error, addr))
		{
		    case DEL_ADDRESS:
			DEBUGMSG(("if_en: ioctl SIOCDELMULTI DEL_ADDRESS\n"));
		        error = (*(enp->nddp->ndd_ctl))
				(enp->nddp, NDD_DISABLE_ADDRESS, addr,
					ETHER_ADDRLEN); 
		        break;
	      
	            case DISABLE_ALL_MULTICASTS: 
		      	DEBUGMSG(("en: SIOCDELMULTI DISABLE_ALL_MULTICASTS\n"));
		      	error = (*(enp->nddp->ndd_ctl))
				(enp->nddp, NDD_DISABLE_MULTICAST, addr,
					ETHER_ADDRLEN); 
		      	break;
		    case 0:
		      	DEBUGMSG(("en: SIOCDELMULTI address still in use"));
		      	break;
		    case -1:
		      	DEBUGMSG(("en: ioctl SIOCDELMULTI error %d\n", error));
		      	break;
		 }
		 break;
	      }
#endif	

	    default:
		error = EINVAL;
	}
	return (error);
}

/***************************************************************************** 
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 ****************************************************************************/ 
en_output(ifp, m, dst, rt)
register struct ifnet *ifp;
register struct mbuf *m;
struct sockaddr *dst;
struct rtentry *rt; {
	register struct en_softc *	enp = (struct en_softc *)ifp;
	register struct ether_header	*eh;
	int				error = 0;
	struct mbuf			*mcopy = 0;
	int				hlen;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		ifp->if_snd.ifq_drops++;
		goto out;
	}

	if ((ifp->if_flags & IFF_SIMPLEX) && (m->m_flags & M_BCAST))
		mcopy = m_copymext(m, 0, (int)M_COPYALL, M_DONTWAIT);

	if (ifp->if_type == IFT_ETHER)
		hlen = sizeof(struct ether_header);
	else
		hlen = sizeof(struct ie3_hdr);

	M_PREPEND(m, hlen, M_DONTWAIT);
	if (m == 0) {
		error = ENOBUFS;
		ifp->if_snd.ifq_drops++;
		goto out;
	}

	eh = mtod(m, struct ether_header *);

	switch (dst->sa_family) {
	    case AF_INET:
		if (!arpresolve(&enp->en_ac, m, dst, eh)) {
			/* not resolved */
			m = 0;
			goto out;
		}
		break;

	    case AF_UNSPEC:
		bcopy(dst->sa_data, eh, hlen);
		break;

	    default:
		if (af_table[dst->sa_family].config.resolve == NULL) {
			error = EAFNOSUPPORT;
			goto out;
		}

		if (!(*af_table[dst->sa_family].config.resolve)
			(&enp->en_ac, m, dst, eh)){
			/* not resolved */
			m = 0;
			goto out;
		}
		break;
	}
	eh = mtod(m, struct ether_header *);
	*(struct char_6 {char x[6];}*)eh->ether_shost =
		*(struct char_6 *)enp->en_addr; /* PERF */

	if (m->m_flags & (M_BCAST|M_MCAST))
		ifp->if_omcasts++;
	ifp->if_obytes += m->m_pkthdr.len;

	if (!enp->nddp) {
		m_freem(m);
		ifp->if_oerrors++;
	} else if ((*(enp->nddp->ndd_output))(enp->nddp, m)) {
		m_freem(m);
		ifp->if_oerrors++;
	}
	if (mcopy)
		(void) looutput(ifp, mcopy, dst, rt);

	ifp->if_opackets++;
	return(error);

out:
	if (mcopy)
		m_freem(mcopy);
	if (m)
		m_freem(m);

	return (error);
}

#ifdef IP_MULTICAST
u_char ether_ipmulticast_min[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00 };
u_char ether_ipmulticast_max[6] = { 0x01, 0x00, 0x5e, 0x7f, 0xff, 0xff };

void ether_map_ip_multicast(struct sockaddr_in *sin, 
			    u_char *addrlo, u_char *addrhi)
{
	if (sin->sin_addr.s_addr == INADDR_ANY) {
		/*
		* An IP address of INADDR_ANY means listen to all
		* of the Ethernet multicast addresses used for IP.
		* (This is for the sake of IP multicast routers.)
		*/
		bcopy(ether_ipmulticast_min, addrlo, 6);
		bcopy(ether_ipmulticast_max, addrhi, 6);
	} else {
		ETHER_MAP_IP_MULTICAST(&sin->sin_addr, addrlo);
		bcopy(addrlo, addrhi, 6);
	}
}
#endif IP_MULTICAST

/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
arpwhohas(ac, addr)
	register struct arpcom *ac;
	struct in_addr *addr;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	register struct ether_arp *ea;
	struct sockaddr_802_3 sa;

	if ((m = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL)
		return;
	m->m_len = sizeof(*ea);
	m->m_pkthdr.len = sizeof(*ea);
	m->m_flags |= M_BCAST;
	MH_ALIGN(m, sizeof(*ea));
	ea = mtod(m, struct ether_arp *);
	eh = (struct ether_header *)&(sa.sa_mac);
	bzero((caddr_t)ea, sizeof (*ea));
	bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	if (ac->ac_if.if_type ==  IFT_ETHER) {
		eh->ether_type = ETHERTYPE_ARP;
		ea->arp_hrd = htons(ARPHRD_ETHER);
	} else {
		eh->ether_type = m->m_pkthdr.len + 
			sizeof(struct ie2_llc_snaphdr);
		ea->arp_hrd = htons(ARPHRD_802_3);
		ie2_llc((eh+1), ETHERTYPE_ARP);
	}
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
	   sizeof(ea->arp_sha));
	bcopy((caddr_t)&ac->ac_ipaddr, (caddr_t)ea->arp_spa,
	   sizeof(ea->arp_spa));
	bcopy((caddr_t)addr, (caddr_t)ea->arp_tpa, sizeof(ea->arp_tpa));
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}

/*
 * Resolve an IP address into an ethernet address.  If success, 
 * desten is filled in.  If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.  A return value of 1 indicates
 * that desten has been filled in and the packet should be sent
 * normally; a 0 return indicates that the packet has been
 * taken over here, either now or for later transmission.
 *
 * We do some (conservative) locking here at splimp, since
 * arptab is also altered from input interrupt service (ecintr/ilintr
 * calls arpinput when ETHERTYPE_ARP packets come in).
 */
arpresolve(ac, m, dst, eh)
	register struct arpcom *ac;
	struct mbuf *m;
	struct sockaddr_in *dst;
	struct ether_header *eh;
{
	register struct arptab *at;
	struct sockaddr_in sin;
	register struct in_ifaddr *ia;
	u_long lna;
	struct ie2_llc_snaphdr	*llcp;
	register struct in_addr *destip;
	int s;
	int hlen;

	destip = &dst->sin_addr;
	llcp = (struct ie2_llc_snaphdr *) (eh + 1);

	/*
	 * This hook needed by NETPMON...
	 */
	NETTRC4(HKWD_IFEN|hkwd_output_in, &(ac->ac_if), m, dst->sin_family, 
		destip->s_addr);

#ifdef IP_MULTICAST
	/*
	 * check for internet multicast.
	 */
	if (IN_MULTICAST(ntohl(destip->s_addr))) {
		ETHER_MAP_IP_MULTICAST(destip, (caddr_t)eh);
		eh->ether_type = ETHERTYPE_IP;
		if (ac->ac_if.if_type != IFT_ETHER) {
			ie2_llc(llcp, ETHERTYPE_IP);
			eh->ether_type = m->m_pkthdr.len - 
				sizeof(struct ie3_mac_hdr);
		}
		return 1;
	} else
#endif /* IP_MULTICAST */
 	if (m->m_flags & M_BCAST) {	/* broadcast */
		bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh,
		    sizeof(etherbroadcastaddr));
		eh->ether_type = ETHERTYPE_IP;
		if (ac->ac_if.if_type != IFT_ETHER) {
			ie2_llc(llcp, ETHERTYPE_IP);
			eh->ether_type = m->m_pkthdr.len - 
				sizeof(struct ie3_mac_hdr);
		}
		return (1);
	}

	/* 
	 * if for us, use software loopback driver if up
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
		if ((ia->ia_ifp == &ac->ac_if) &&
		    (destip->s_addr == ia->ia_addr.sin_addr.s_addr)) {
			if (ia->ia_ifp->if_flags & IFF_DO_HW_LOOPBACK ||
			    !((&loif)->if_flags & IFF_UP)) {
				bcopy((caddr_t)ac->ac_enaddr, (caddr_t)eh,
					    sizeof(ac->ac_enaddr));
				eh->ether_type = ETHERTYPE_IP;
				if (ac->ac_if.if_type != IFT_ETHER) {
					ie2_llc(llcp, ETHERTYPE_IP);
					eh->ether_type = m->m_pkthdr.len - 
						sizeof(struct ie3_mac_hdr);
				}
				return (1);
			} else {
				sin.sin_family = AF_INET;
				sin.sin_addr = *destip;
				/* point mbuf to ip */
				if (ac->ac_if.if_type == IFT_ETHER)
					hlen = sizeof(struct ether_header);
				else
					hlen = sizeof(struct ie3_hdr);
				m->m_data += hlen;
				m->m_len -= hlen;
				m->m_pkthdr.len -= hlen;
				(void) looutput(&loif, m,
					(struct sockaddr *) &sin, 
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
		if (ac->ac_if.if_flags & IFF_NOARP) {
			u_char	*desten = (u_char *)eh;
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)desten, 3);
			lna = in_lnaof(*destip);
			desten[3] = (lna >> 16) & 0x7f;
			desten[4] = (lna >> 8) & 0xff;
			desten[5] = lna & 0xff;
			eh->ether_type = ETHERTYPE_IP;
			ARPTAB_UNLOCK(&arptab_lock);
			return (1);
		} else {
			at = arptnew(destip, &ac->ac_if);
			if (at == 0)
				panic("arpresolve: no free entry");
			at->at_hold = m;
			ARPTAB_UNLOCK(&arptab_lock);
	
			arpwhohas(ac, destip);
			return (0);
		}
	}
	if (at->at_flags & ATF_COM) {	/* entry IS complete */
		eh->ether_type = ETHERTYPE_IP;
		*(struct char6 {char x[6];}*)eh->ether_dhost =
			*(struct char6 *)at->at_enaddr; /* PERF */

		if (ac->ac_if.if_type != IFT_ETHER) {
			ie2_llc(llcp, ETHERTYPE_IP);
			eh->ether_type = m->m_pkthdr.len - sizeof(struct ie3_mac_hdr);
		}
		ARPTAB_UNLOCK(&arptab_lock);
		return (1);
	}
	/*
	 * There is an arptab entry, but no ethernet address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (at->at_hold) {
		m_freem(at->at_hold);
		ac->ac_if.if_arpdrops++;
	}
	at->at_hold = m;
	if (!(at->at_flags & ATF_ASK)) { /* Free to ask again*/
		at->at_flags |= ATF_ASK;
		ARPTAB_UNLOCK(&arptab_lock);
		arpwhohas(ac, destip);		/* ask again */
	} else 
		ARPTAB_UNLOCK(&arptab_lock);
	return (0);
}

/*
 * Called from 10 Mb/s Ethernet interrupt handlers
 * when ether packet type ETHERTYPE_ARP
 * is received.  Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
arpinput(ac, m, hp)
	struct arpcom *ac;
	struct mbuf *m;
	caddr_t hp;	/* not used */
{
	register struct arphdr *ar;

	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	if (m->m_len < sizeof(struct arphdr))
		goto out;
	ar = mtod(m, struct arphdr *);
	if (m->m_len < sizeof(struct arphdr) + 2 * ar->ar_hln + 2 * ar->ar_pln)
		goto out;

	if (ntohs(ar->ar_pro) != ETHERTYPE_IP) {
		net_error(&ac->ac_if, ARP_UNK_PROTO, 0);
		goto out;
	}

	switch (ntohs(ar->ar_hrd)) {
		case ARPHRD_ETHER:
		case ARPHRD_802_3:
			in_arpinput(ac, m);
			break;

		default:
			net_error(&ac->ac_if, ARP_WRONG_HDR, 0);
			goto out;
	}

	return;

out:
	m_freem(m);
}

/*
 * ARP for Internet protocols on 10 Mb/s Ethernet.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 */
void
in_arpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct ether_arp *ea;
	struct ether_header *eh;
	register struct arptab *at;  /* same as "merge" flag */
	register struct in_ifaddr *ia;
	struct in_ifaddr *maybe_ia = 0;
	struct sockaddr_in sin;
	struct sockaddr sa;
	struct in_addr isaddr, itaddr, myaddr;
	int proto, op;
	int s;

	ea = mtod(m, struct ether_arp *);
	proto = ntohs(ea->arp_pro);
	op = ntohs(ea->arp_op);
	bcopy((caddr_t)ea->arp_spa, (caddr_t)&isaddr, sizeof (isaddr));
	bcopy((caddr_t)ea->arp_tpa, (caddr_t)&itaddr, sizeof (itaddr));
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
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)ac->ac_enaddr,
	  sizeof (ea->arp_sha)))
		goto out;	/* it's from me, ignore it. */
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)etherbroadcastaddr,
	    sizeof (ea->arp_sha))) {
		net_error(&ac->ac_if, ARP_IPBRDCAST_ADDR, 0);
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		net_error(&ac->ac_if, ARP_DUP_ADDR, 0);
		itaddr = myaddr;
		if (op == ARPOP_REQUEST)
			goto reply;
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
		    (m->m_pkthdr.rcvif != at->at_ifp)) {
			ARPTAB_UNLOCK(&arptab_lock);
                        goto reply;
		}

		bcopy((caddr_t)ea->arp_sha, (caddr_t)at->at_enaddr,
		    sizeof(ea->arp_sha));
		at->at_flags |= ATF_COM;
		if (at->at_hold) {
			struct mbuf *om;

			sin.sin_family = AF_INET;
			sin.sin_addr = isaddr;
			if (ac->ac_if.if_type != IFT_ETHER)
				m_adj(at->at_hold, sizeof(struct ie3_hdr));
			else
				m_adj(at->at_hold, sizeof(struct ether_header));
			om = at->at_hold;
			at->at_hold = 0;
			ARPTAB_UNLOCK(&arptab_lock);
			(*ac->ac_if.if_output)(&ac->ac_if, om, 
				(struct sockaddr *)&sin, (struct rtentry *)0);
			goto reply;
		}
	}
	if (at == 0 && itaddr.s_addr == myaddr.s_addr) {
		/* ensure we have a table entry */
		if (at = arptnew(&isaddr, &ac->ac_if)) {
			bcopy((caddr_t)ea->arp_sha, (caddr_t)at->at_enaddr,
			    sizeof(ea->arp_sha));
			at->at_flags |= ATF_COM;
		}
	}
	ARPTAB_UNLOCK(&arptab_lock);
reply:
	if (op != ARPOP_REQUEST)
		goto out;

	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
		    sizeof(ea->arp_sha));
	} else {
		ARPTAB_LOOK(at, itaddr.s_addr);
		if (at == NULL || (at->at_flags & ATF_PUBL) == 0)
			goto out;
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		bcopy((caddr_t)at->at_enaddr, (caddr_t)ea->arp_sha,
		    sizeof(ea->arp_sha));
	}

	bcopy((caddr_t)ea->arp_spa, (caddr_t)ea->arp_tpa,
	    sizeof(ea->arp_spa));
	bcopy((caddr_t)&itaddr, (caddr_t)ea->arp_spa,
	    sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY); 
	eh = (struct ether_header *)sa.sa_data;
	bcopy((caddr_t)ea->arp_tha, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));

	if (ac->ac_if.if_type != IFT_ETHER) {
		eh->ether_type = m->m_pkthdr.len + 
			sizeof(struct ie2_llc_snaphdr);
		ie2_llc((eh+1), ETHERTYPE_ARP);
	} else {
		eh->ether_type = ETHERTYPE_ARP;
	}
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
	return;
out:
	m_freem(m);
	return;
}


#ifdef _SUN
/*
 * Reverse-ARP input. If this is a request we look the ethernet address
 * of the sender up in the arp table (server side).
 * If this is a response, the incoming packet contains our internet address (client).
 */
#define RARP_REQUEST  3   /* Reverse ARP request */
#define RARP_REPLY    4   /* Reverse ARP request */

revarpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct ether_arp *ea;
	register struct arptab *at = 0;
	register struct ether_header *eh;
	struct ether_addr myether;
	struct ifnet *ifp;
	struct ifaddr *ifa;
        struct sockaddr sa;
	int s;

	ea = mtod(m, struct ether_arp *);
	if (m->m_len < sizeof *ea)
		goto out;
	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	if (ntohs(ea->arp_pro) != ETHERTYPE_IP)
		goto out;
	switch(ntohs(ea->arp_op)) {
	case RARP_REPLY:
		break;

	case RARP_REQUEST:
		ARPTAB_LOCK(&arptab_lock);
 		for (at = arptabp ; at < (arptabp + arptabsize) ; at++) {
                        if (at->at_flags & ATF_PERM &&
                            !bcmp((caddr_t)at->at_enaddr,
                            (caddr_t)ea->arp_tha, 6))
                                break;
                }
                if (at < (arptabp + arptabsize)) {
                        /* found a match, send it back */
                        eh = (struct ether_header *)sa.sa_data;
                        bcopy(ea->arp_sha, eh->ether_dhost, 
				sizeof(ea->arp_sha));
                        bcopy((caddr_t)(&at->at_iaddr), ea->arp_tpa,
				sizeof(at->at_iaddr));
			/* search for interface address to use */
			ifp = &ac->ac_if;
			for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
				if (ifa->ifa_ifp == ifp
					&& ifa->ifa_addr->sa_family == AF_INET){
				    bcopy((caddr_t)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
					ea->arp_spa, sizeof(ea->arp_spa));
				    break;
				}
			}
			if (ifa == 0) {
				break;
			}
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
			    sizeof(ea->arp_sha));
                        eh->ether_type = ETHERTYPE_REVARP;
                        ea->arp_op = htons(RARP_REPLY);
                        sa.sa_family = AF_UNSPEC;
			ARPTAB_UNLOCK(&arptab_lock);
                        (*ac->ac_if.if_output)(&ac->ac_if, m, &sa,
				(struct rtentry *)0);
                        return;
		}
		ARPTAB_UNLOCK(&arptab_lock);
		break;

	default:
		break;
	}
out:
	m_freem(m);
	return;
}
#endif /* _SUN */

#ifdef	DEBUG
/*
 * Convert Ethernet address to printable (loggable) representation.
 */
static char *
ether_sprintf(ap)
	register u_char *ap;
{
	register i;
	static char etherbuf[18];
	register char *cp = etherbuf;
	static char digits[] = "0123456789abcdef";

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}
#endif
