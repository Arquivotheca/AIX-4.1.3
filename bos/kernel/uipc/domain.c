static char sccsid[] = "@(#)93	1.32.1.8  src/bos/kernel/uipc/domain.c, sysuipc, bos41J, 9516A_all 4/17/95 14:02:21";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: HDRALIGN
 *		add_domain_af
 *		del_domain_af
 *		domain_add
 *		domain_del
 *		domaininit
 *		getdomainname
 *		gethostid
 *		gethostname
 *		kgetdomainname
 *		kgethostname
 *		pfctlinput
 *		pffastsched
 *		pffasttimo
 *		pffindproto
 *		pffindtype
 *		pfreclaim
 *		pfslowsched
 *		pfslowtimo
 *		protosw_enable
 *		protosw_disable
 *		setdomainname
 *		sethostid
 *		sethostname
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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
 *	Base:	src/bos/kernel/uipc/domain.c, sysuipc, bos410 (Berkeley) 1/8/94
 *	Merged: uipc_domain.c	7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#ifdef	_AIX_FULLOSF
#include "sys/kernel.h"
#else	/* _AIX_FULLOSF */
#include "sys/timer.h"
#include "sys/intr.h"
#include "sys/syspest.h"
#include "sys/nettrace.h"
#include "net/spl.h"
#endif	/* _AIX_FULLOSF */

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/errno.h"

#include "net/netisr.h"

#define HDRALIGN(x)	(((x)+sizeof(u_long)-1) & ~(sizeof(u_long)-1))

LOCK_ASSERTL_DECL

struct domain	*domains;
struct protox	ip_protox[MAXPROTOX];
#if	NETSYNC_LOCK
simple_lock_data_t	domain_lock;
lock_data_t		protox_lock;
#endif

#ifndef	_AIX_FULLOSF
struct trb *fast_trb, *slow_trb;
#endif	/* _AIX_FULLOSF */

void
domaininit()
{
#ifndef	_AIX_FULLOSF
    	extern  int pffastsched(), pfslowsched();
	int ticks;

	ticks = HZ/2;
	slow_trb = talloc();
	assert(slow_trb != NULL);
	slow_trb->timeout.it_value.tv_sec = ticks/HZ;
	slow_trb->timeout.it_value.tv_nsec = (ticks % HZ) * (1000000000 / HZ);
	slow_trb->flags = 0;
	slow_trb->func = (void *)pfslowsched;
	slow_trb->func_data = (ulong) 0;
	slow_trb->ipri = INTCLASS2;

	ticks = HZ/5;
	fast_trb = talloc();
	assert(fast_trb != NULL);
	fast_trb->timeout.it_value.tv_sec = ticks/HZ;
	fast_trb->timeout.it_value.tv_nsec = (ticks % HZ) * (1000000000 / HZ);
	fast_trb->flags = 0;
	fast_trb->func = (void *)pffastsched;
	fast_trb->func_data = (ulong) 0;
	fast_trb->ipri = INTCLASS2;
#endif	/* _AIX_FULLOSF */

	domains = 0;
	DOMAIN_LOCKINIT();
	PROTOX_LOCKINIT();
#ifdef	_AIX
	if (max_linkhdr < 24)
		max_linkhdr = 24;
	for (ticks = 0; ticks < MAXPROTOX; ticks++) {
		ip_protox[ticks].protosw = (struct protosw *) 0;
		ip_protox[ticks].protox_flags = PROTOX_NOINIT;
	}
#else	/* _AIX */
	max_linkhdr = 16;		/* XXX */
#endif	/* _AIX */
	max_protohdr = 0;
	max_hdr = max_linkhdr + max_protohdr;
	max_datalen = MHLEN - max_hdr;
	if (netisr_add(NETISR_PFSLOW, pfslowtimo,
				(struct ifqueue *)0, (struct domain *)0) ||
	    netisr_add(NETISR_PFFAST, pffasttimo,
				(struct ifqueue *)0, (struct domain *)0))
		panic("domaininit");
#ifdef	_AIX_FULLOSF
	pffasttimo();
	pfslowtimo();
#else	/* _AIX_FULLOSF */
    	pffastsched();
   	pfslowsched();
#endif	/* _AIX_FULLOSF */
}

domain_add(dp)
	register struct domain *dp;
{
	register struct domain *tdp;
	register struct protosw CONST *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_WRITE_LOCK();
	for (tdp = domains; tdp; tdp = tdp->dom_next)
		if (tdp->dom_family == dp->dom_family) {
			DOMAIN_WRITE_UNLOCK();
			return EEXIST;
		}
	DOMAINRC_LOCKINIT(dp);
	dp->dom_refcnt = 0;
	dp->dom_next = domains;
	domains = dp;
	DOMAIN_WRITE_UNLOCK();

	DOMAIN_FUNNEL(dp, f);
	if (dp->dom_init)
		(*dp->dom_init)();
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
		if (pr->pr_init)
			(*pr->pr_init)();
	DOMAIN_UNFUNNEL(f);

	DOMAIN_WRITE_LOCK();
	max_linkhdr = HDRALIGN(max_linkhdr);
	max_protohdr = HDRALIGN(max_protohdr);
	max_hdr = max_linkhdr + max_protohdr;
	max_datalen = MHLEN - max_hdr;
	if (max_datalen < 0)
		panic("domain_add max_hdr");
	DOMAIN_WRITE_UNLOCK();
	return 0;
}

domain_del(dp)
	struct domain *dp;
{
	int error = 0;
	DOMAIN_LOCK_DECL()
	DOMAINRC_LOCK_DECL()

	return(EADDRINUSE);
#ifdef del_domains
	DOMAIN_WRITE_LOCK();
	DOMAINRC_LOCK(dp);
	if (dp->dom_refcnt == 0) {
		if (dp == domains)
			domains = dp->dom_next;
		else {
			register struct domain *tdp;
			for (tdp = domains; tdp; tdp = tdp->dom_next)
				if (tdp->dom_next == dp) {
					tdp->dom_next = dp->dom_next;
					break;
				}
			if (tdp == NULL)
				error = ENOENT;
		}
	} else
		error = EADDRINUSE;
	DOMAINRC_UNLOCK(dp);
	LOCK_FREE(&(dp->dom_rc_lock));
	DOMAIN_WRITE_UNLOCK();
	return error;
#endif 
}

struct protosw *
pffindtype(family, type)
	int family, type;
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	int i;
	DOMAIN_LOCK_DECL()
	DOMAINRC_LOCK_DECL()

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	DOMAIN_READ_UNLOCK();
	return (0);
found:
	if (dp->dom_family != PF_INET 
		|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_type && pr->pr_type == type) {
				DOMAINRC_REF(dp);
				DOMAIN_READ_UNLOCK();
				return (struct protosw *)pr;
			}
	} else {
		for (i=0; i < MAXPROTOX; i++) {
			if (!(ip_protox[i].protox_flags & PROTOX_USED)) 
				continue;
			if ((ip_protox[i].protosw)->pr_type && 
				(ip_protox[i].protosw)->pr_type == type) {
				DOMAINRC_REF(dp);
				DOMAIN_READ_UNLOCK();
				return(ip_protox[i].protosw);
			}
		}
	}
	DOMAIN_READ_UNLOCK();
	return (0);
}

struct protosw *
pffindproto(family, protocol, type)
	int family, protocol, type;
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	struct protosw CONST *maybe = 0;
	DOMAIN_LOCK_DECL()
	DOMAINRC_LOCK_DECL()

	if (family == 0)
		return (0);
	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	DOMAIN_READ_UNLOCK();
	return (0);
found:
	if (dp->dom_family != PF_INET 
		|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++) {
			if ((pr->pr_protocol == protocol) && (pr->pr_type == type)) {
				maybe = pr;
				break;
			}
			if (type == SOCK_RAW && pr->pr_type == SOCK_RAW &&
			    pr->pr_protocol == 0 && maybe == (struct protosw *)0)
				maybe = pr;
		}
	} else {
		if (protocol < MAXPROTOX && protocol > 0) {
			if (ip_protox[protocol].protox_flags & PROTOX_USED) {
				if ((ip_protox[protocol].protosw)->pr_type == type) /* XXX. Probably should assert this. */
					maybe = ip_protox[protocol].protosw;
			}
/* Take care of raw wildcard. */
			if (!maybe && type == SOCK_RAW)
				maybe = ip_protox[0].protosw;
		}
	}
	if (maybe)
		DOMAINRC_REF(dp);
	DOMAIN_READ_UNLOCK();
	return (struct protosw *)maybe;
}

void
pfctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	int i;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		if (dp->dom_family != PF_INET 
			|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
			for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
				if (pr->pr_ctlinput)
					(*pr->pr_ctlinput)(cmd, sa, (caddr_t) 0);
		} else {
			for (i=0; i < MAXPROTOX; i++) {
				if (!(ip_protox[i].protox_flags & PROTOX_USED)) 
					continue;
				if ((ip_protox[i].protosw)->pr_ctlinput)
					(*((ip_protox[i].protosw)->pr_ctlinput)) (cmd,sa,(caddr_t) 0);
			}
		}
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
}

void
pfreclaim()
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	int i;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		if (dp->dom_family != PF_INET 
			|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
			for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
				if (pr->pr_drain)
					(*pr->pr_drain)();
		} else {
			for (i=0; i < MAXPROTOX; i++) {
				if (!(ip_protox[i].protox_flags & PROTOX_USED)) 
					continue;
				if ((ip_protox[i].protosw)->pr_drain)
					(*((ip_protox[i].protosw)->pr_drain)) ();
			}
		}
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
}

void
pfslowtimo()
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	int i;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		if (dp->dom_family != PF_INET 
			|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
			for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
				if (pr->pr_slowtimo)
					(*pr->pr_slowtimo)();
		} else {
			for (i=0; i < MAXPROTOX; i++) {
				if (!(ip_protox[i].protox_flags & PROTOX_USED)) 
					continue;
				if ((ip_protox[i].protosw)->pr_slowtimo)
					(*((ip_protox[i].protosw)->pr_slowtimo)) ();
			}
		}
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
#ifdef	_AIX_FULLOSF
	timeout(netisr_timeout, (caddr_t)NETISR_PFSLOW, hz/PR_SLOWHZ);
#endif	/* _AIX_FULLOSF */
}

void
pffasttimo()
{
	register struct domain *dp;
	register struct protosw CONST *pr;
	int i;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		if (dp->dom_family != PF_INET 
			|| (ip_protox[MAXPROTOX-1].protox_flags & PROTOX_NOINIT)) {
			for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
				if (pr->pr_fasttimo)
					(*pr->pr_fasttimo)();
		} else {
			for (i=0; i < MAXPROTOX; i++) {
				if (!(ip_protox[i].protox_flags & PROTOX_USED)) 
					continue;
				if ((ip_protox[i].protosw)->pr_fasttimo)
					(*((ip_protox[i].protosw)->pr_fasttimo)) ();
			}
		}
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
#ifdef	_AIX_FULLOSF
	timeout(netisr_timeout, (caddr_t)NETISR_PFFAST, hz/PR_FASTHZ);
#endif	/* _AIX_FULLOSF */
}

#ifndef	_AIX_FULLOSF

int pfslowsched()
{
	int ticks = HZ/2;
	
	schednetisr(NETISR_PFSLOW);	
	slow_trb->timeout.it_value.tv_sec = ticks/HZ;
	slow_trb->timeout.it_value.tv_nsec = (ticks % HZ) * (1000000000 / HZ);
	slow_trb->ipri = INTCLASS2;
	tstart(slow_trb);
}

int pffastsched()
{
	int ticks = HZ/5;

	schednetisr(NETISR_PFFAST);	
	fast_trb->timeout.it_value.tv_sec = ticks/HZ;
	fast_trb->timeout.it_value.tv_nsec = (ticks % HZ) * (1000000000 / HZ);
	fast_trb->ipri = INTCLASS2;
	tstart(fast_trb);
}

/* old sys/lfs/domain.c includes */
#include "sys/user.h"
#include "sys/priv.h"
/* end */

#define MAXDNAME 256

/* old sys/lfs/domain.c program */
int	hostid;
char	hostname[MAXDNAME];
int	hostnamelen;
char	domainname[MAXDNAME];
int	domainnamelen;


gethostid()
{
	TRCHKL0T(HKWD_SYSC_TCPIP | hkwd_gethostid_in);
	return(hostid);
}

sethostid(inhostid)
	int inhostid;
{
	TRCHKL1T(HKWD_SYSC_TCPIP | hkwd_sethostid_in,inhostid);
	if (privcheck(SYS_CONFIG)) {
		u.u_error = EPERM;
		return -1;
	}
	hostid = inhostid;
	return(0);
}

gethostname(outhostname, len)
	char *outhostname;
	int len;
{
	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_gethostname_in,outhostname,len);
	if (len > hostnamelen + 1)
		len = hostnamelen + 1;
	if ( copyout((caddr_t)hostname, (caddr_t)outhostname, len) ) {
		u.u_error = EFAULT;
		return -1;
	}
	return 0;
}

kgethostname(outhostname, len)
	char *outhostname;
	int *len;
{
	if (*len > hostnamelen + 1)
		*len = hostnamelen + 1;

	bcopy((caddr_t)hostname, (caddr_t)outhostname, *len); 
	return 0;
}
sethostname(inhostname, len)
	char *inhostname;
	int len;
{
	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_sethostname_in,inhostname,len);
	if (privcheck(SYS_CONFIG)) {
		u.u_error = EPERM;
		return -1;
	}
	if (len > sizeof (hostname) - 1) {
		u.u_error = EINVAL;
		return -1;
	}
	hostnamelen = len;
	if ( copyin((caddr_t)inhostname, hostname, len) ) {
		u.u_error = EFAULT;
		return -1;
	}
	hostname[len] = 0;
	return 0;
}

#ifdef _SUN 
/* getdomainname and setdomainname */
getdomainname(outdomainname, len)
	char *outdomainname;
	int len;
{
	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_getdomainname_in,outdomainname,len);
	if (len > domainnamelen + 1)
		len = domainnamelen + 1;
	if ( copyout((caddr_t)domainname,(caddr_t)outdomainname,len) ) {
		u.u_error = EFAULT;
		return -1;
	}
	return 0; 
}

kgetdomainname(outdomainname, len)
	char *outdomainname;
	int *len;
{
	if (*len > domainnamelen + 1)
		*len = domainnamelen + 1;
	bcopy((caddr_t)domainname, (caddr_t)outdomainname, *len); 
	return 0;
}

setdomainname(indomainname, len)
	char *indomainname;
	int len;
{
	TRCHKL2T(HKWD_SYSC_TCPIP | hkwd_setdomainname_in,indomainname,len);
	if (privcheck(SYS_CONFIG)) {
		u.u_error = EPERM;
		return -1;
	}
	if (len > sizeof (domainname) - 1) {
		u.u_error = EINVAL;
		return -1;
	}
	domainnamelen = len;

	if ( copyin((caddr_t)indomainname, domainname, len) ) {
		u.u_error = EFAULT;
		return -1;
	}

	domainname[len] = 0;
	return 0;
}
/* end sys/lfs/domain.c program */
#endif

int
add_domain_af(dom)
	register struct domain *dom;
{
	return(domain_add(dom));
}

int
del_domain_af(dom)
	register struct domain *dom;
{
	return(domain_del(dom));
}

/* LWR. Not just for NSIP anymore. */
/* enable the protocol switch table.  Some protocols, e.g. XNS or OSI
 * encapsulation, can not be enabled until the kernel extension is loaded.
 * This route enables the appropriate protocols during the initialization
 * of those kernel extension.
 */
int protosw_enable(pr)
struct protosw *pr;
{
	if (pr->pr_protocol > MAXPROTOX ||
			pr->pr_type  != SOCK_RAW)
		return(EINVAL);
/* Serialization. We are really just concerned with simultaneous
 * calls for the same protocol number. Hardly likely normally just
 * blow it off but this is not the IBM way.
 */
	PROTOX_LOCK();
	if (ip_protox[pr->pr_protocol].protox_flags & PROTOX_USED){
		PROTOX_UNLOCK();
		return(ENOTEMPTY);
	}
	ip_protox[pr->pr_protocol].protosw = pr;
/* Always set the pointer before the flag. We can be called when flag
 * is set. Also init protocol before setting call flag.
 */
	if ((ip_protox[pr->pr_protocol].protosw)->pr_init)
		(*((ip_protox[pr->pr_protocol].protosw)->pr_init)) ();
	ip_protox[pr->pr_protocol].protox_flags = PROTOX_USED;
	PROTOX_UNLOCK();
	return(0);
}

int protosw_disable(pr)
struct protosw *pr;
{
	struct protosw *raw_protoswp;

	if (pr->pr_protocol > MAXPROTOX ||
			pr->pr_type  != SOCK_RAW)
		return(EINVAL);

#ifndef IPPROTO_RAW
#define IPPROTO_RAW 255		/* XXX: from netinet/in.h.  I don't want a
				 * dependency on a kernel extension!
				 */
#endif
	if ((raw_protoswp = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW)) == 0)
		panic("protosw_disable: no raw IP protosw entry");

	ip_protox[pr->pr_protocol].protox_flags = 0;
	ip_protox[pr->pr_protocol].protosw = raw_protoswp;
/* Conservative. Yeehaw no serialization, none required */
	return(0);
}
#endif	/* _AIX_FULLOSF */
