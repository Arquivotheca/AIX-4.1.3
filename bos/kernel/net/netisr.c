static char sccsid[] = "@(#)17	1.6  src/bos/kernel/net/netisr.c, sysnet, bos411, 9428A410j 3/15/94 17:10:15";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: Netintr
 *		netinit
 *		netisr_add
 *		netisr_af
 *		netisr_del
 *		netisr_input
 *		netisr_lookup
 *		netisr_thread
 *		netisr_timeout
 *		netisrinit
 *		
 *
 *   ORIGINS: 27,85
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
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
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
 *	netisr.c - Kernel thread(s) for network code.
 *	Also does network initialization.
 */

#include "net/net_globals.h"

#include "sys/param.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/errno.h"

#include "net/if.h"
#include "net/netisr.h"

#ifdef	_AIX_FULLOSF
#include <streams.h>
#include "sys/sysconfig.h"
#endif	/* _AIX_FULLOSF */

#ifndef	_AIX_FULLOSF
#include "sys/intr.h"
#include "sys/pri.h"
#include "sys/proc.h"
#include <ulimit.h>
#endif	/* _AIX_FULLOSF */

LOCK_ASSERTL_DECL

struct netisr netisr_active;
struct { struct netisr *next, *prev; } netisr_inactive;
struct netisr *netisr_table;

struct netisr_dynamic {
	struct netisr_dynamic *next, *prev;
	struct netisr netisr;
};
struct { struct netisr_dynamic *next, *prev; } netisr_dynamic;

#if	NETISR_THREAD
extern task_t	first_task;
/* Configurable number of netisr threads (always at least 1). */
int netisrthreads = NNETTHREADS;	/* configurable as "pseudo-device" */
#endif
int	netisr_servers = EVENT_NULL;
int	netthread_start();

#if	NETSYNC_LOCK
simple_lock_data_t	netisr_slock;
#endif

void
netinit(int priority)
{
	static char initialized;

	if (initialized)
		return;
	initialized = 1;

#ifndef	_AIX_FULLOSF
	kmeminit();
	kmeminit_thread(30);
#endif	/* _AIX_FULLOSF */

	mbinit();

	netthread_init();	

	/* Attach those devices which do not attach themselves */
	/* config() has previously attached hardware */

	loattach();

	/* Initialize interface lists, global network data, domains */
	ifinit();
	domaininit();

	aix_netinit();

	/*
	 * Configure the domains. Test for dynamic options and defer
	 * to config manager if set, else look at actual options.
	 */

	/* Raw and routing sockets. Configure unconditionally. */
	route_configure();

	/* Unix-local sockets (includes pipes, fifos!) */

	uipc_configure();
}

/* Called from mbinit prior to adding first isr */
void
netisrinit()
{
	int i;

	NETISR_LOCKINIT();
	netisr_active.next = netisr_active.prev =
		(struct netisr *)&netisr_active;
	netisr_inactive.next = netisr_inactive.prev =
		(struct netisr *)&netisr_inactive;
	netisr_dynamic.next = netisr_dynamic.prev =
		(struct netisr_dynamic *)&netisr_dynamic;
	MALLOC(netisr_table, struct netisr *,
		NNETISR * sizeof (struct netisr), M_TEMP, M_WAITOK);
	if (netisr_table == 0)
		panic("netisrinit");
	bzero((caddr_t)netisr_table, NNETISR * sizeof (struct netisr));
	for (i = 0; i < NNETISR; i++) {
		netisr_table[i].id = i;
		insque(&netisr_table[i], netisr_inactive.prev);
	}
}

struct netisr *
netisr_lookup(id)
	register int id;
{
	register struct netisr_dynamic *nd;

	/* Scan only the dynamic netisr's to speed search */
	for (nd = netisr_dynamic.next; ; nd = nd->next) {
		if (nd == (struct netisr_dynamic *)&netisr_dynamic)
			break;
		if (nd->netisr.id == (short)id)
			return &nd->netisr;
	}
	return 0;
}

/*
 * Add/delete isr's in input table. Isr's are specified by a small
 * integer, and an optional input queue and domain may be specified.
 * The input queue is used for isr's which receive packets, the
 * domain is used for its funnel and reference count.
 */
netisr_add(num, isr, ifq, dp)
	int num;
	void (*isr)(void);
	struct ifqueue *ifq;
	struct domain *dp;
{
	register struct netisr *netisr;
	struct netisr_dynamic *nd = NULL;
	int s, err = 0;
	NETISR_LOCK_DECL()
	DOMAINRC_LOCK_DECL()

	if (isr == NULL || num < 0)
		return EINVAL;
	if (num < NNETISR)
		netisr = &netisr_table[num];
	else {
		NETISR_LOCK();
		netisr = netisr_lookup(num);
		NETISR_UNLOCK();
		if (netisr)
			return EEXIST;
		NET_MALLOC(nd, struct netisr_dynamic *, sizeof *nd, M_TEMP, M_WAITOK);
		netisr = &nd->netisr;
		bzero((caddr_t)netisr, sizeof *netisr);
		netisr->id = num;
	}
	if (dp)
		DOMAINRC_REF(dp);
	NETISR_LOCK();
	if ((netisr->isr && netisr->isr != isr) ||
	    (num >= NNETISR && netisr_lookup(num)))
		err = EEXIST;
	else {
		netisr->active = 0;
		netisr->pending = 0;
		netisr->dom = dp;
		netisr->ifq = ifq;
		netisr->isr = isr;
		if (nd) {
			insque(netisr, netisr_inactive.prev);
			insque(nd, netisr_dynamic.prev);
		}
	}
	NETISR_UNLOCK();
	if (err) {
		if (dp)
			DOMAINRC_UNREF(dp);
		if (nd)
			NET_FREE(nd, M_TEMP);
	}
	return err;
}

netisr_del(num)
	int num;
{
	register struct netisr *netisr;
	int s, err = 0;
	struct domain *dp = 0;
	NETISR_LOCK_DECL()
	IFQ_LOCK_DECL()
	DOMAINRC_LOCK_DECL()

	if (num < 0)
		return EINVAL;
	NETISR_LOCK();
	if ((netisr = which_netisr(num)) == NULL || netisr->isr == NULL)
		err = ENOENT;
	else if (netisr->active)
		err = EBUSY;
	else {
		if (netisr->ifq) {
			IFQ_LOCK(netisr->ifq);
			for (;;) {
				register struct mbuf *m;
				IF_DEQUEUE_NOLOCK(netisr->ifq, m);
				if (m == NULL) break;
				m_freem(m);
				IF_DROP(netisr->ifq);
			}
			IFQ_UNLOCK(netisr->ifq);
		}
		dp = netisr->dom;
		netisr->active = 0;
		netisr->pending = 0;
		netisr->dom = NULL;
		netisr->ifq = NULL;
		netisr->isr = NULL;
		remque(netisr);
		insque(netisr, netisr_inactive.prev);
	}
	NETISR_UNLOCK();
	/* Note: netisr never freed. */
	if (dp)
		DOMAINRC_UNREF(dp);
	return err;
}


/*
 * Receive packet for given isr. Packet is always delivered or freed.
 * Isr == -1 means just look for wildcard receiver.
 * Tries to avoid copies in case not deliverable to intended.
 *
 * Note many protocol stacks step on the packet buffer during
 * processing so we pullup the headers if there's a wildcard
 * for insurance.
 * Also note the loopback interface, among others, leaves few clues
 * to the packet's identity.
 */
netisr_input(num, m, header, hdrlen)
	int num;
	struct mbuf *m;
	caddr_t header;
	int hdrlen;
{
	register struct netisr *netisr;
	register int s, wild, err = 0;
	register struct ifqueue *ifq;
	NETISR_LOCK_DECL()
	IFQ_LOCK_DECL()

	NETISR_LOCK();
	if (num < 0) num = NETISR_WILD;
	netisr = which_netisr(num);
	if (num != NETISR_WILD && netisr_table[NETISR_WILD].isr)
		wild = -1;
	else if (num != NETISR_OTHER && netisr_table[NETISR_OTHER].isr)
		wild = NETISR_OTHER;
	else
		wild = 0;
	if (netisr == NULL || netisr->isr == NULL)
		err = ENOENT;
	else
		ifq = netisr->ifq;
	NETISR_UNLOCK();
	if (err == 0) {
		if (ifq) {
			IFQ_LOCK(ifq);
			if (IF_QFULL(ifq)) {
				IF_DROP(ifq);	/* bump stat, leave err == 0 */
			} else if (wild >= 0) {
				IF_ENQUEUE_NOLOCK(ifq, m);
				m = 0;
			} else {
				struct mbuf *mcopy, *n;
				if (mcopy = m_copym(m,0,M_COPYALL,M_DONTWAIT)) {
					if (n = m_get(M_DONTWAIT, MT_HEADER)) {
						n->m_pkthdr.rcvif = mcopy->m_pkthdr.rcvif;
						n->m_pkthdr.len = mcopy->m_pkthdr.len + hdrlen;
						n->m_len = hdrlen;
						mcopy->m_flags &= ~M_PKTHDR;
						n->m_next = mcopy;
						if (header && hdrlen > 0)
							bcopy(header, mtod(n, caddr_t), hdrlen);
						mcopy = n = m_pullup(n, MIN(max_hdr, n->m_pkthdr.len));
						header = 0;
					} else {
						m_freem(mcopy);
						mcopy = 0;
					}
				}
				IF_ENQUEUE_NOLOCK(ifq, m);
				m = mcopy;
			}
			IFQ_UNLOCK(ifq);
		}
		schednetisr(num);	/* XXX could be more efficient */
	}

	if (m) {
		if (wild) {
			if (header && hdrlen > 0) {
				M_PREPEND(m, hdrlen, M_DONTWAIT);
				if (m == NULL)
					return err;
				bcopy(header, mtod(m, caddr_t), hdrlen);
			}
			/* The OTHER delivery will feed any wildcards */
			(void) netisr_input(wild, m, (caddr_t)0, 0);
		} else
			m_freem(m);
	}
	return err;
}

/*
 * Return ISR appropriate for address family (used by loopback).
 */
netisr_af(af)
{
	static int isrs[] =
		{ -1, -1, NETISR_IP, -1, -1, -1, NETISR_NS, NETISR_ISO };

	if ((unsigned)af >= sizeof isrs / sizeof isrs[0])
		return -1;
	return isrs[af];
}


/*
 * Process network interrupts by type. May be called from a software
 * interrupt callout, or from thread context (see below).
 */
void
Netintr()
{
	register void (*isr)(void);
	register struct netisr *netisr;
	register struct domain *dp;
	register int handled = 0;
	int s;
	NETISR_LOCK_DECL()

	NETISR_LOCK();
again:
	for (;;) {
		for (netisr = netisr_active.next; ; netisr = netisr->next) {
			if (netisr == &netisr_active)
				goto out;
			if (netisr->pending) {
				++netisr->wakeups;
				netisr->events += netisr->pending;
				if ((isr = netisr->isr) == 0) {
					remque(netisr);
#ifdef	_AIX
					queue_init(netisr);
#endif	/* _AIX */
					insque(netisr, netisr_inactive.prev);
					netisr->pending = 0;
					++netisr->extras;
					goto again;
				}
				dp = netisr->dom;
				handled += netisr->pending;
				netisr->pending = 0;
				++netisr->active;
				break;
			}
		}
		NETISR_UNLOCK();
		if (dp) {
			DOMAIN_FUNNEL_DECL(f)
			DOMAIN_FUNNEL(dp, f);
			(*isr)();
			DOMAIN_UNFUNNEL(f);
		} else
			(*isr)();
		NETISR_LOCK();
		if (--netisr->active == 0 && netisr->pending == 0) {
			remque(netisr);
#ifdef	_AIX
			queue_init(netisr);
#endif	/* _AIX */	
			insque(netisr, netisr_inactive.prev);
		}
	}
out:
	++netisr_active.wakeups;
	if (handled)
		netisr_active.events += handled;
	else
		++netisr_active.extras;
#if	NETISR_THREAD
#ifdef	_AIX_FULLOSF
	assert_wait_mesg(0, FALSE, "netisr");
	/* Enqueue ourselves FIFO */
	current_thread()->ipc_wait_queue.next =
			(struct queue_entry *)netisr_servers;
	current_thread()->t_threadlink = netisr_servers;
	netisr_servers = current_thread();
#endif
#endif
	NETISR_UNLOCK();
}

/*
 * Common code for service thread mainlines. 
 */

#if	!NETISR_THREAD

void
netisr_timeout(n)
	caddr_t n;
{
#undef	setsoftnet		/* Save a needless software interrupt */
#define	setsoftnet()
	schednetisr((int)n);
	Netintr();
}

#else	/* NETISR_THREAD */

void
netisr_timeout(n)
	caddr_t n;
{
	schednetisr((int)n);
}

void
netisr_thread()
{
	spl0();

	/* Process network events, registered with netisr_add. */
	for (;;) {
		Netintr();		/* Process packets */
		assert(curthread->t_lockcount == 0);
		e_sleep_thread(&netisr_servers, NULL, 0);
	}
	/* NOTREACHED */
}

netthread_init()
{
	pid_t		pid;

        pid = creatp();
        assert(pid != -1);
        initp(pid, netthread_start, 0, 0, "gil");
        assert(setpri(pid, NETPCL_RTPRI) != -1);
}

netthread_start()
{
	int		s;
	tid_t		tid;
	sigset_t	sig;
	int		d[16];
	u_int		stacklim;

	bzero(&sig, sizeof(sig));	
	stacklim = ulimit(GET_STACKLIM, 0);

	s = netisrthreads;
	netisrthreads = 0;
	do {
		stacklim += (256 * 1024);	/* 256K stack for each thread */
		tid = thread_create();
		if (tid == -1)
			break;
		if (kthread_start(tid, netisr_thread, NULL, 0, stacklim, &sig))
			break;
		
		++netisrthreads;
	} while (--s > 0);
	if (netisrthreads == 0)
		panic("netthread_start");
	for (;;)
		et_wait(EVENT_NETISR, EVENT_NETISR, EVENT_SHORT);
	/* NOTREACHED */
}
#endif	/* NETISR_THREAD */
