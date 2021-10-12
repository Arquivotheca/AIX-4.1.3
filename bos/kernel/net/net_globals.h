/* @(#)14	1.18  src/bos/kernel/net/net_globals.h, sysnet, bos411, 9428A410j 6/17/94 09:35:59 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: 
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
 * Global #defines for OSF/1 networking.
 *
 * Ugly as this file is, it makes the code a lot cleaner!
 *
 * The following major #defines are supported:
 *	NCPUS		number of processors
 *	NNETTHREADS	pseudo-device specifying threads
 *	UNIX_LOCKS	configure with locking
 *	VAGUE_STATS	no locking around stats
 *
 * These defines yield the following internal defines:
 *	NETNCPUS	number of processors doing network
 *	NETSYNC_SPL	do process synch with spl's (may co-exist w/locks)
 *	NETSYNC_LOCK	do process sync with locks (may co-exist w/spl)
 *	NETISR_THREAD	do isr's via thread (else via software interrupt)
 *	NETSYNC_LOCKTEST	turn on extra lock debugging (UNIX, too)
 *
 * Current prerequisites (not enforced!):
 *	One or both of NETSYNC_SPL && NETSYNC_LOCK
 *	NETSYNC_LOCK requires NETISR_THREAD
 *	UNIX requires NETSYNC_SPL
 */

#ifndef	_NET_GLOBALS_H_
#define _NET_GLOBALS_H_

#include <sys/lock_def.h>
#ifdef	_KERNEL
#include <sys/sleep.h>
#include <sys/user.h>
#include <net/spl.h>
#include <sys/syspest.h>
#include <sys/lockname.h>
#include <sys/lock_alloc.h>
#endif	/* _KERNEL */

/* 
 * Stuff to fix #defines in if.h.
 */
#ifdef simple_lock_data_t
#undef simple_lock_data_t
#endif
#ifdef lock_data_t
#undef lock_data_t
#endif
typedef	Simple_lock	simple_lock_data_t;
typedef	Complex_lock	lock_data_t;
typedef	int *task_t;
typedef	struct thread	*thread_t;

#define	lock_init2(lp, s, type)	lock_init(lp, s)

#define NETNHSQUE       128
#define netsqhash(X)    (&nethsque[( (((u_int) X) >> 12) + (((u_int) X) >> 8) + ((u_int) X) ) & (NETNHSQUE-1)])
extern	int             nethsque[];

#define	assert_wait(addr, intr) 				\
	e_assert_wait(netsqhash(addr), intr)

#define	assert_wait_mesg(addr, intr, msg) 			\
	e_assert_wait(netsqhash(addr), intr)

#define	clear_wait(thread, result, flag)			\
	e_clear_wait((thread)->t_tid, result)

#define	wakeup_one(addr)	 				\
	e_wakeup_one(netsqhash(addr))

#define	wakeup(addr)	 					\
	e_wakeup(netsqhash(addr))

#define	thread_wakeup(addr)					\
	e_wakeup(netsqhash(addr))

#define	current_thread()	(curthread)
#define	thread_block()		e_block_thread()
#define	thread_swappable(a, b)

thread_t kernel_isrthread(task_t task, void (*start)(void), int);

#define	PAGE_SIZE	4096
#define	MAXALLOCSAVE	(4 * PAGE_SIZE)	/* param.h in osf */
#define THEWALL_MAX	(64*1024*1024) 	/* 64Meg */
#define MAX_INITIAL_PIN	((512*1024)/ MAXALLOCSAVE)
#define	NNETTHREADS	4

/*
 * So everyone sees the same structures (ifnet, etc)...
 */
#ifndef IP_MULTICAST
#define IP_MULTICAST
#endif

/* These are for OSF compat. Not actually used. */
#define	LTYPE_RAW	1
#define	LTYPE_ROUTE	2
#define	LTYPE_SOCKET	3
#define	LTYPE_SOCKBUF	4
#define	LTYPE_DOMAIN	5
#define LTYPE_ARPTAB	6

#define	STATS_ACTION(lock, action)	action

#define	queue_init(q)   ((q)->next = (q)->prev = q)

#define	pfind(pgid)	   		(pgid)
#define	gsignal(pgid, sig)   		pgsignal((pgid), (sig))
#define	psignal(p, sig)   		pidsig((p), (sig))
#define	psignal_inthread(p, sig)   	pidsig((p), (sig))

#define	BM(x)			x
#define	P_UNREF(p)

#define	round_page(x)	((((x)+((PAGE_SIZE)-1))/(PAGE_SIZE))*(PAGE_SIZE))
#define	trunc_page(x)	((((int)(x)) / PAGE_SIZE) * PAGE_SIZE)
typedef int vm_offset_t;
typedef int vm_size_t;
typedef int vm_map_t;
task_t	first_task;

/*
 * These are default settings. Either or both of locking and spl are valid
 * for 1 or more cpus. However, recommend locks for multis, non-locks for unis.
 * The thread decision is dependent on several things. To configure both
 * sockets and streams to use softnets requires locore or hardware support.
 */
#define NETNCPUS	NCPUS
#define NETSYNC_LOCK	1		/* maybe locks for synch */
#define NETSYNC_SPL	!NETSYNC_LOCK	/* else spl for synch */

#define NETISR_THREAD	(NETSYNC_LOCK || (NETNCPUS > 1) || (NNETTHREADS > 0))

typedef int	spl_t;
#define LOCK_ASSERTL_DECL
#ifdef	DEBUG
#define LOCK_ASSERT(string, cond)	assert(cond)
#else	/* DEBUG */
#define LOCK_ASSERT(string, cond)
#endif	/* DEBUG */
#define NETSYNC_LOCKTEST DEBUG
#define LOCK_NETSTATS	 0
#define	LOCK_FREE(lp)			lock_free(lp)

#define NETSPL_DECL(s)		spl_t s;
#define NETSPL(s,level)		s = spl##level()
#define NETSPLX(s)		splx(s)

#if	LOCK_NETSTATS
#define NETSTAT_LOCK_DECL()	int	_stats;
#define NETSTAT_LOCKINIT(lockp)	{					\
	lock_alloc(lockp, LOCK_ALLOC_PIN, IF_SLOCK, lockp);		\
	simple_lock_init(lockp);					\
}
#define NETSTAT_LOCK(lockp)	_stats = disable_lock(PL_IMP, lockp)
#define NETSTAT_UNLOCK(lockp)	unlock_enable(_stats, lockp)
#else
#define NETSTAT_LOCK_DECL()
#define NETSTAT_LOCKINIT(lockp)
#define NETSTAT_LOCK(lockp)
#define NETSTAT_UNLOCK(lockp)
#endif

/* ANSI-C compatibility */
#ifndef	CONST
#define CONST		const
#endif
#ifndef	VOLATILE
#define VOLATILE	volatile
#endif

/* Global function prototypes */
#include "sys/types.h"
#include "net/proto_net.h"
#include "net/proto_uipc.h"

#ifdef _KERNEL
#define RTO_DFLT_LOW 1
#define RTO_DFLT_HIGH 64
#define RTO_DFLT_LIMIT 7
#define RTO_DFLT_LENGTH 13
#define RTO_DFLT_SHIFT 7
#endif /* _KERNEL */

struct iftrace {
	int	kmid;
	int	promisc;
};

#define TCP_NDEBUG 100

#define IF_SIZE 8;
extern int ifsize;

CONST u_char	etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
CONST u_char	ie5_broadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
CONST u_char	fddi_broadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#ifdef	PANIC2ASSERT
#define panic(p)	{__assert2(__assert1((unsigned)(0),0,99));}
#endif	/* PANIC2ASSERT */

#endif	/* _NET_GLOBALS_H_ */
