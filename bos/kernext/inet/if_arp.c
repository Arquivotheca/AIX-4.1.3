static char sccsid[] = "@(#)19	1.14  src/bos/kernext/inet/if_arp.c, sysxinet, bos411, 9438C411a 9/23/94 10:16:37";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: arpinit
 *		arpioctl
 *		arpsched
 *		arptfree
 *		arptimer
 *		arptimer_init
 *		arptnew
 *		ie2_llc
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
/* Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <net/net_globals.h>

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/spl.h>
#include <net/netisr.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <sys/nettrace.h>
#include <net/if_arp.h>

#include <net/net_malloc.h>

LOCK_ASSERTL_DECL

struct 	trb *arptime;
int	arpt_killc = ARPT_KILLC;
int	arpt_killc_dflt = ARPT_KILLC;

#if     NETSYNC_LOCK
Simple_lock     arptab_lock;
#endif

int
arpinit()
{

 	if (arptab_bsiz <= 0 || arptab_nb <= 0) {
		arptabbsiz = arptab_bsiz_dflt;
		arptabnb = arptab_nb_dflt;
	}
	else {
		arptabbsiz = arptab_bsiz;
		arptabnb = arptab_nb;
	}

	arptabsize = arptabbsiz * arptabnb;
	NET_MALLOC(arptabp, struct arptab *, 
		(sizeof(struct arptab)) * arptabsize, M_KTABLE, M_WAITOK);
	bzero((caddr_t)arptabp, sizeof (struct arptab) * arptabsize);
	ARPTAB_LOCKINIT(&arptab_lock);

	if (!(arptime = talloc()))
		return(ENOMEM);
	arptimer_init();

	return(0);
}

void
arptimer()
{
	register  struct  arptab		*at;
	register				i;
	int s;

	ARPTAB_LOCK(&arptab_lock);
	at = arptabp;
	for (i = 0; i < arptabsize; i++ , at++) {
		if   (at->at_flags == 0 || (at->at_flags & ATF_PERM))
			continue;
		at->at_flags &= ~ATF_ASK;
		if (++at->at_timer < ((at->at_flags & ATF_COM) ?
			arpt_killc : ARPT_KILLI))
				continue;
		/* timer has expired, clear entry	*/
		arptfree(at);
	}
	ARPTAB_UNLOCK(&arptab_lock);
}

arpsched()
{
	schednetisr(NETISR_ARPTMO);
	arptime->timeout.it_value.tv_sec = 60;
	arptime->timeout.it_value.tv_nsec = 0;
        arptime->ipri = PL_IMP;
	tstart(arptime);
}

/*
 * arptimer_init - insures that only one timer per interface is started
 *
 */
arptimer_init()
{
	arptime->timeout.it_value.tv_sec = 60;
	arptime->timeout.it_value.tv_nsec = 0;
	arptime->flags = 0;
	arptime->func = (void *)arpsched;
	arptime->func_data = (ulong) 0;
	arptime->ipri = PL_IMP;
	netisr_add(NETISR_ARPTMO, arptimer, NULL, NULL);
	arpsched();
}

/*
 * Free an arptab entry.
 * MUST BE CALLED WITH arptab_lock held.
 */
void
arptfree(at)
	register struct arptab *at;
{
	/*
	 * call the IF layer ioctl() to inform it of the
	 * arp delete.  Pass in at.
	 */
	if (at->at_ifp && at->at_ifp->if_ioctl)
		(*at->at_ifp->if_ioctl)(at->at_ifp, SIOCDARP,(caddr_t)at);

	if (at->at_hold)
		m_freem(at->at_hold);
	bzero(at, sizeof(*at));
}

arpioctl(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct arpreq *ar = (struct arpreq *)data;
	register struct arptab *at;
	register struct sockaddr_in *sin;
	register struct ifaddr *ifa;
	struct ifnet *ifp;
	int s;

	sin = (struct sockaddr_in *)&ar->arp_ha;
#if defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
	if (sin->sin_family == 0 && sin->sin_len < 16)
		sin->sin_family = sin->sin_len;
#endif
	sin->sin_len = sizeof(ar->arp_ha);
	sin = (struct sockaddr_in *)&ar->arp_pa;
#if defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
	if (sin->sin_family == 0 && sin->sin_len < 16)
		sin->sin_family = sin->sin_len;
#endif
	sin->sin_len = sizeof(ar->arp_pa);

	if (ar->arp_pa.sa_family != AF_INET ||
	    ar->arp_ha.sa_family != AF_UNSPEC)
		return (EAFNOSUPPORT);
        /* Don't need ifa to delete. */
        if (cmd != SIOCDARP && ((ifa = ifa_ifwithnet(&ar->arp_pa)) == NULL))
                return (ENETUNREACH);

	ARPTAB_LOCK(&arptab_lock);
	ARPTAB_LOOK(at, sin->sin_addr.s_addr);
	if (at == NULL) {		/* not found */
		if (cmd != SIOCSARP) {
			ARPTAB_UNLOCK(&arptab_lock);
			return (ENXIO);
		}
	}
	switch (cmd) {

	case SIOCSARP:		/* set entry */
		if (at == NULL) {
			at = arptnew(&sin->sin_addr, ifa->ifa_ifp);
			if (at == NULL) {
				ARPTAB_UNLOCK(&arptab_lock);
				return (EADDRNOTAVAIL);
			}
			if (ar->arp_flags & ATF_PERM) {
			/* never make all entries in a bucket permanent */
				register struct arptab *tat;
				
				/* try to re-allocate */
				tat = arptnew(&sin->sin_addr, ifa->ifa_ifp);
				if (tat == NULL) {
					arptfree(at);
					ARPTAB_UNLOCK(&arptab_lock);
					return (EADDRNOTAVAIL);
				}
				arptfree(tat);
			}
		}
                bcopy((caddr_t)ar->arp_ha.sa_data, (caddr_t)at->hwaddr,
                    sizeof(at->hwaddr));
		at->at_flags = ATF_COM | ATF_INUSE |
			(ar->arp_flags & (ATF_PERM|ATF_PUBL));

		bcopy(&ar->ifd, &at->if_dependent, sizeof(at->if_dependent));

		at->at_timer = 0;
		at->at_ifp = ifa->ifa_ifp;
		break;

	case SIOCDARP:		/* delete entry */
		arptfree(at);
		break;

	case SIOCGARP:		/* get entry */
		ifp = ifa->ifa_ifp;
		bcopy((caddr_t)at->hwaddr, (caddr_t)ar->arp_ha.sa_data, 
			ifp->if_addrlen);
		ar->arp_flags = at->at_flags;
		ar->ifType = ifp->if_type;
		ar->at_length = ifp->if_addrlen;
		bcopy(&at->if_dependent, &ar->ifd, sizeof(ar->ifd));
		break;

	}
	ARPTAB_UNLOCK(&arptab_lock);
	return (0);
}

/*
 * Enter a new address in arptab, pushing out the oldest entry 
 * from the bucket if there is no room.
 * This always succeeds since no bucket can be completely filled
 * with permanent entries (except from arpioctl when testing whether
 * another permanent entry will fit).
 * MUST BE CALLED AT SPLIMP. MUST BE CALLED WITH arptab_lock locked.
 */
struct arptab *
arptnew(addr, ifp)
	struct in_addr *addr;
	struct ifnet *ifp;
{
	register n;
	int oldest = -1;
	register struct arptab *at, *ato = NULL;

	at = arptabp + (ARPTAB_HASH(addr->s_addr) * arptabbsiz);
	for (n = 0; n < arptabbsiz; n++,at++) {
		if (at->at_flags == 0)
			goto out;	 /* found an empty entry */
		if (at->at_flags & ATF_PERM)
			continue;
		if ((int) at->at_timer > oldest) {
			oldest = at->at_timer;
			ato = at;
		}
	}
	if (ato == NULL) {
		net_error(0, ARP_TABFULL, 0);
		return (NULL);
	}
	at = ato;
	arptfree(at);
out:
	at->at_iaddr = *addr;
	at->at_flags = (ATF_INUSE|ATF_ASK);
	at->at_ifp = ifp;
	return (at);
}

#ifdef ie2_llc
#undef ie2_llc
#endif

/*
 * ie2_llc -	fill in an IEEE 802.2 LLC field.
 *
 * This is a #define in net/nd_lan.h, but is left here
 * for binary compatibility!
 *
 * Input:
 *	llc	-	^ to LLC
 *	type	-	pkt type
 */
ie2_llc(llc, type)
register struct ie2_llc_snaphdr *llc; {

	llc->dsap       = DSAP_INET;
	llc->ssap       = SSAP_INET;
	llc->ctrl       = CTRL_UI;	/* Unnumbered Information	*/
	llc->prot_id[0] = 0;		/* Reserved, MBZ		*/
	llc->prot_id[1] = 0;		/* Reserved, MBZ		*/
	llc->prot_id[2] = 0;		/* Reserved, MBZ		*/
	llc->type       = type;		/* pkt type			*/
}
