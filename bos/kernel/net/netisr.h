/* @(#)62	1.29  src/bos/kernel/net/netisr.h, sockinc, bos411, 9428A410j 3/25/94 23:23:01 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: NETISR_LOCK
 *		NETISR_LOCKINIT
 *		NETISR_UNLOCK
 *		schednetisr
 *		setsoftnet
 *		which_netisr
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1980, 1986, 1989 Regents of the University of California.
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
 *	Base:	netisr.h	7.5 (Berkeley) 4/22/89
 *	Merged:	netisr.h	7.6 (Berkeley) 6/28/90
 */

#ifndef	_NET_NETISR_H
#define	_NET_NETISR_H

/*
 * The networking code is in separate kernel threads or in software
 * interrupts. When running in threads, all events are delivered in
 * thread context, at splnet() or spl0() depending on lock configuration.
 * When running in software interrupts, events are delivered at splnet()
 * or splsoftclock().
 *
 * Clients of the isr framework use a "netisr" structure to
 * enqueue events and maintain active status. Some default
 * structures are defined globally (especially those used in
 * non-local contexts e.g. packet receive), others may be
 * registered with netisr_add(). See net/netisr.c.
 */

struct netisr {
	struct	netisr *next, *prev;	/* Link in {in}active isr list */
	int	active;			/* Softnet active count */
	int	pending;		/* Interrupt pending on queue */
	void	(*isr)(void);		/* Isr to process input */
	struct	ifqueue *ifq;		/* Queue to receive packets, or NULL */
	struct	domain *dom;		/* Domain isr belongs to, or NULL */
	u_short	flags;			/* Flags */
	short	id;			/* Integer id */
	u_long	events;			/* Event count */
	u_long	wakeups;		/* Wakeup count */
	u_long	extras;			/* Wakeups with nothing to do */
	u_long	noserver;		/* No servers ready - NETISR_THREAD */
};

/*
 * Isr's may be used to deliver data to protocols, or simply events.
 */

#define NETISR_WILD	0	/* Copy of all packets */
#define NETISR_OTHER	1	/* Packets otherwise tossed */
#define	NETISR_ARP	2	/* ARP packets */
#define	NETISR_IP	3	/* IP packets */
#define	NETISR_NS	4	/* XNS packets */
#define	NETISR_ISO	5	/* ISO packets */

#define	NETISR_MB	6	/* Mbuf event */
#define NETISR_STREAMS	7	/* Streams scheduler */
#define NETISR_STRTO	8	/* Streams timeout */
#define NETISR_STRWELD	9	/* Streams "weld" */
#define NETISR_PFFAST	10	/* Sockets fasttimeout */
#define NETISR_PFSLOW	11	/* Sockets slowtimeout */
#define NETISR_IFSLOW	12	/* Sockets interface watchdog */
#define NETISR_ARPTMO	13	/* Sockets ARP timer */
#define NETISR_WRITE	14	/* Delayed comio write (compat only) */
#define	NETISR_MBLK	15	/* Mblk event */
#define NETISR_NDD    	16      /* AF_NDD sockets */
#define NETISR_STRFUNNEL 17	/* Streams funneler */
#define NETISR_PFCTLINPUT 18	/* Used for calling pfctlinput on a thread */

/* The number of preallocated isr's is arbitrary, but
 * should be large enough for the above predefines. */
#define NNETISR		(1024/sizeof(struct netisr))


#ifdef	_KERNEL

#ifdef	_AIX_FULLOSF
#include "kern/queue.h"
#include "kern/thread.h"
#include "kern/sched_prim.h"
#else
#define	EVENT_NETISR	0x0726
#endif

#define schednetisr(num) do {						\
	struct netisr *netisr;						\
	int server = 0;							\
	NETISR_LOCK_DECL()						\
	NETISR_LOCK();							\
	if ((netisr = which_netisr(num)) &&				\
	    (++netisr->pending + netisr->active) == 1) {		\
		remque(netisr);						\
		queue_init(netisr);					\
		insque(netisr, netisr_active.prev);			\
		if (netisr_servers != EVENT_NULL)			\
			server = 1;					\
		else							\
			++netisr_active.noserver;			\
	}								\
	NETISR_UNLOCK();						\
	if (server)							\
		e_wakeup_one(&netisr_servers);				\
} while (0)

extern struct netisr netisr_active, *netisr_table;

#define which_netisr(num)	((unsigned)(num) < NNETISR ? \
					&netisr_table[num] : \
					netisr_lookup(num))

extern simple_lock_data_t	netisr_slock;
extern int			netisr_servers;
#define NETISR_LOCK_DECL()	int	_ns;
#define NETISR_LOCKINIT()	{					\
	lock_alloc(&netisr_slock, LOCK_ALLOC_PIN, NETISR_LOCK_FAMILY, -1);\
	simple_lock_init(&netisr_slock);				\
}
#define NETISR_LOCK()		_ns = disable_lock(INTMAX, &netisr_slock)
#define NETISR_UNLOCK()		unlock_enable(_ns, &netisr_slock)

#ifdef	_AIX
/* AIX 3.x compatibility defines */
#define NETISR_MAX	(sizeof(unsigned long) * NBPB)
#define NET_KPROC	0
#define NET_OFF_LEVEL	1
#define NET_WILD_TYPE	0xffff
#define MAX_NITS	32

struct nit_ent {	/* entry in the Network Input Table	*/
	u_char		used;		/* 1 = used, 0 = free	*/
	u_char		ref_cnt;	/* # sharing this type  */
	u_short         type;		/* input packet type	*/
        struct ifqueue  *ifq_addr;	/* queue, may be NULL	*/
	int		(*handler)();	/* handler for unqueued */
	u_short		af;		/* address family number*/
};

#endif
#endif
#endif
