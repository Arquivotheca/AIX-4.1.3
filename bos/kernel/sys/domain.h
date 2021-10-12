/* @(#)10	1.13  src/bos/kernel/sys/domain.h, sockinc, bos411, 9428A410j 3/21/94 16:20:27 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: DOMAINRC_LOCK
 *		DOMAINRC_LOCKINIT
 *		DOMAINRC_REF
 *		DOMAINRC_UNLOCK
 *		DOMAINRC_UNREF
 *		DOMAIN_FUNNEL
 *		DOMAIN_FUNNEL_DECL
 *		DOMAIN_LOCKINIT
 *		DOMAIN_LOCK_DECL
 *		DOMAIN_READ_LOCK
 *		DOMAIN_READ_UNLOCK
 *		DOMAIN_UNFUNNEL
 *		DOMAIN_UNFUNNEL_FORCE
 *		DOMAIN_WRITE_LOCK
 *		DOMAIN_WRITE_UNLOCK
 *		PROTOX_LOCKINIT
 *		PROTOX_LOCK
 *		PROTOX_UNLOCK
 *		
 *
 *   ORIGINS: 26,27,71
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
 *	Base:	domain.h	7.3 (Berkeley) 6/27/88
 *	Merged: domain.h	7.4 (Berkeley) 6/28/90
 */

#ifndef	_SYS_DOMAIN_H_
#define _SYS_DOMAIN_H_

/*
 * To get lock_data_t defined...
 */
#ifndef _NET_GLOBALS_H_
#include <net/net_globals.h>
#endif

/*
 * Uniprocessor compatibility: allows per-domain funnel operation.
 * Replaces all socket-level spl's, for instance.
 */
struct domain_funnel {
	void (*unfunnel)();		/* unfunnel operation or NULL */
	union {				/* local storage for operation */
		int	spl;			/* previous spl */
		caddr_t	other;			/* recursive lock_t, e.g. */
	} object;
};

#ifndef	CONST
#define CONST
#endif

/*
 * Structure per communications domain.
 */
struct	domain {
	int	dom_family;		/* AF_xxx */
	char	*dom_name;
#ifdef	_KERNEL
	void	(*dom_init)(void);
	int	(*dom_externalize)(struct mbuf *);
	void	(*dom_dispose)(struct mbuf *);
#else
	void	(*dom_init)();		/* initialize domain data structures */
	int	(*dom_externalize)();	/* externalize access rights */
	void	(*dom_dispose)();	/* dispose of internalized rights */
#endif
	struct	protosw CONST *dom_protosw, *dom_protoswNPROTOSW;
	struct	domain *dom_next;
	int	dom_refcnt;		/* # sockets in this domain */
#ifdef	_KERNEL
	void	(*dom_funnel)(struct domain_funnel *);
	void	(*dom_funfrc)(struct domain_funnel *);
#if	NETSYNC_LOCK
	simple_lock_data_t	dom_rc_lock;
#endif
#else
	void	(*dom_funnel)();	/* uniprocessor compat */
	void	(*dom_funfrc)();	/* uniprocessor compat */
#endif
};

#ifdef	_KERNEL
extern	struct domain *domains;
extern	simple_lock_data_t domain_lock;
extern	lock_data_t protox_lock;
/* Domain add can be used for adding an entire address family with 
 * it's own protocol switch. Problem is when one wants to add entries 
 * to an existing protocol switch. This will be the case for IP 
 * encapsulation or a new protocol within IP. In this
 * case, we store the protosw pointers in a protox array not in the 
 * domain structure. This allows us to add protocol entries up to the 
 * size of the array. Users call protosw_enable for this kernel service.
 * Support is currently limited to only the AF_INET, others
 * may be added over time.
 */
struct protox {
	struct protosw *protosw;
	short protox_flags; /* See below. */
};
#define PROTOX_USED 0x01
#define PROTOX_NOINIT 0x02
#define MAXPROTOX 256 /* should be same as IPPROTO_ MAX */
extern struct protox ip_protox[MAXPROTOX]; /* ip table */
#define DOMAIN_LOCKINIT()
#define PROTOX_LOCKINIT()	{					\
	lock_alloc(&protox_lock, LOCK_ALLOC_PIN, DOMAIN_LOCK_FAMILY, 2);\
	simple_lock_init(&protox_lock);			\
}
#define DOMAIN_LOCK_DECL()
#define DOMAIN_READ_LOCK()
#define DOMAIN_WRITE_LOCK()
#define DOMAIN_READ_UNLOCK()
#define DOMAIN_WRITE_UNLOCK()
#define	DOMAINRC_LOCKINIT(dp)
#define DOMAINRC_LOCK_DECL()
#define	DOMAINRC_LOCK(dp)
#define	DOMAINRC_UNLOCK(dp)
#define	PROTOX_LOCK()	simple_lock(&protox_lock)
#define	PROTOX_UNLOCK()	simple_unlock(&protox_lock)

#define	DOMAINRC_REF(dp) \
	{ DOMAINRC_LOCK(dp); (dp)->dom_refcnt++; DOMAINRC_UNLOCK(dp); }

#define	DOMAINRC_UNREF(dp) \
	{ DOMAINRC_LOCK(dp); (dp)->dom_refcnt--; DOMAINRC_UNLOCK(dp); }

#define DOMAIN_FUNNEL_DECL(f) \
	struct domain_funnel f;

#define DOMAIN_FUNNEL(dp, f) \
	{ (f).unfunnel = 0; if ((dp)->dom_funnel) (*(dp)->dom_funnel)(&(f)); }

#define DOMAIN_UNFUNNEL(f) \
	{ if ((f).unfunnel) (*(f).unfunnel)(&(f)); }

/* Forced unfunnel is used before sleeping in sosleep() */
#define DOMAIN_UNFUNNEL_FORCE(dp, f) \
	{ (f).unfunnel = 0; \
	  if ((dp)->dom_funfrc) (*(dp)->dom_funfrc)(&(f)); }

#endif
#endif
