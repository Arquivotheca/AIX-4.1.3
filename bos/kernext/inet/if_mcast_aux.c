static char sccsid[] = "@(#)87	1.3  src/bos/kernext/inet/if_mcast_aux.c, sysxinet, bos411, 9428A410j 3/15/94 16:51:48";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: DEBUGMSG
 *		DRIVER_LOOKUP_MULTI
 *		driver_addmulti
 *		driver_delmulti
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


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <sys/nettrace.h>

#include <net/nh.h>		/*pdw*/
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <net/spl.h>


#ifdef IP_MULTICAST
#include <netinet/if_mcast_aux.h>
#include <net/net_globals.h>
#define DEBUGMSG(x) 

/*
 * Macro for looking up the driver_multi record for a given range of MAC
 * multicast addresses connected to a given arpcom structure.  If no matching
 * record is found, "enm" returns NULL.
 */
#define DRIVER_LOOKUP_MULTI(addrlo, addrhi, ac, enm)			\
	/* u_char              addrlo[6]; */				\
	/* u_char              addrhi[6]; */				\
	/* struct arpcom      *ac;        */				\
	/* struct driver_multi *enm;       */				\
{									\
	for ((enm) = (ac)->ac_multiaddrs;				\
	     (enm) != NULL &&						\
		(bcmp((enm)->enm_addrlo, (addrlo), 6) != 0 ||		\
		 bcmp((enm)->enm_addrhi, (addrhi), 6) != 0);		\
	     (enm) = (enm)->enm_next);					\
}


/*
 * Add a mac-level multicast address or range of addresses to the list for a
 * given interface.
 */
driver_addmulti(struct ifreq *ifr, struct arpcom *ac, void (*map)(struct sockaddr_in *, u_char *, u_char *), int *error, char *mac_address)
{
	register struct driver_multi *enm;
	struct sockaddr_in *sin;
	u_char addrlo[6];
	u_char addrhi[6];
	struct mbuf *m;
	NETSPL_DECL(s)
	ACMULTI_LOCK_DECL()

	*error = 0;

	DEBUGMSG(("driver_addmulti\n")); 
	switch (ifr->ifr_addr.sa_family) {

	case AF_UNSPEC:
		bcopy(ifr->ifr_addr.sa_data, addrlo, 6);
		bcopy(addrlo, addrhi, 6);
		break;

#ifdef INET
	case AF_INET:
		sin = (struct sockaddr_in *)&(ifr->ifr_addr);
		(*map)(sin, addrlo, addrhi);
		break;
#endif INET

	default:
		*error = EAFNOSUPPORT;
		return -1;
	}

	ACMULTI_LOCK(ac); /* lock address list */
	/*
	 * See if the address range is already in the list.
	 */
	DRIVER_LOOKUP_MULTI(addrlo, addrhi, ac, enm);
	if (enm != NULL) {
 	  DEBUGMSG(("driver_addmulti address already in list\n")); 
		/*
		 * Found it; just increment the reference count.
		 */
		++enm->enm_refcount;
	        ACMULTI_UNLOCK(ac);
		return 0;
	}
	/*
	 * New address or range; get an mbuf for a new multicast record
	 * and link it into the interface's multicast list.
	 */
	if ((m = m_getclr(M_DONTWAIT, MT_IFMADDR)) == NULL) {
	  DEBUGMSG(("driver_addmulti m_getclr no buffer\n")); 
	        ACMULTI_UNLOCK(ac);
	        *error = ENOBUFS;
		return -1;
	}
	enm = mtod(m, struct driver_multi *);
	bcopy(addrlo, enm->enm_addrlo, 6);
	bcopy(addrhi, enm->enm_addrhi, 6);
	enm->enm_ac = ac;
	enm->enm_refcount = 1;
	enm->enm_next = ac->ac_multiaddrs;
	ac->ac_multiaddrs = enm;
	ACMULTI_UNLOCK(ac);
	/*
	 * Return and inform the driver that the list has changed
	 * and its reception filter should be adjusted accordingly.
	 */
	if(bcmp(addrlo, addrhi, 6)) {
	    return ENABLE_ALL_MULTICASTS;
	} else {
	    bcopy(addrlo, mac_address, 6);
	    return ADD_ADDRESS;
	}
}

/*
 * Delete a multicast address record.
 */
driver_delmulti(struct ifreq *ifr, struct arpcom *ac, void (*map)(struct sockaddr_in *, u_char *, u_char *), int *error, char *mac_address)
{
	register struct driver_multi *enm;
	register struct driver_multi **p;
	struct sockaddr_in *sin;
	u_char addrlo[6];
	u_char addrhi[6];
	NETSPL_DECL(s)
	ACMULTI_LOCK_DECL()

	*error = 0;

	DEBUGMSG(("driver_delmulti\n")); 
	switch (ifr->ifr_addr.sa_family) {

	case AF_UNSPEC:
		bcopy(ifr->ifr_addr.sa_data, addrlo, 6);
		bcopy(addrlo, addrhi, 6);
		break;

#ifdef INET
	case AF_INET:
		sin = (struct sockaddr_in *)&(ifr->ifr_addr);
		(*map)(sin, addrlo, addrhi);
		break;
#endif INET

	default:
		*error = EAFNOSUPPORT;
		return -1;
	}

	ACMULTI_LOCK(ac); /* lock address list */

	/*
	 * Look up the address in our list.
	 */
	DRIVER_LOOKUP_MULTI(addrlo, addrhi, ac, enm);
	if (enm == NULL) {
  	  DEBUGMSG(("driver_delmulti address not in list\n")); 
	        ACMULTI_UNLOCK(ac);
	        *error = ENXIO;
	        return -1;
	}
	if (--enm->enm_refcount != 0) {
  	  DEBUGMSG(("driver_delmulti address still claimed\n")); 
		/*
		 * Still some claims to this record.
		 */
	        ACMULTI_UNLOCK(ac);
		return 0;
	}
	/*
	 * No remaining claims to this record; unlink and free it.
	 */
	DEBUGMSG(("driver_delmulti freeing address\n")); 
	for (p = &enm->enm_ac->ac_multiaddrs;
	     *p != enm;
	     p = &((*p)->enm_next));
	*p = (*p)->enm_next;
	ACMULTI_UNLOCK(ac);

	m_free(dtom(enm));
	/*
	 * Return and inform the driver that the list has changed
	 * and its reception filter should be adjusted accordingly.
	 */
	if(bcmp(addrlo, addrhi, 6)) {
	    return DISABLE_ALL_MULTICASTS;
	} else {
	    bcopy(addrlo, mac_address, 6);
	    return DEL_ADDRESS;
	}
}
#endif IP_MULTICAST
