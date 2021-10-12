/* @(#)48	1.12  src/bos/kernext/inet/in_pcb.h, sockinc, bos411, 9433A411a 8/12/94 10:43:03 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: INHEAD_LOCKINIT
 *		INHEAD_READ_LOCK
 *		INHEAD_READ_UNLOCK
 *		INHEAD_WRITE_LOCK
 *		INHEAD_WRITE_UNLOCK
 *		INPCBRC_LOCK
 *		INPCBRC_LOCKINIT
 *		INPCBRC_REF
 *		INPCBRC_UNLOCK
 *		INPCBRC_UNREF
 *		INPCB_ISLOCKED
 *		INPCB_LOCK
 *		INPCB_LOCKINIT
 *		INPCB_UNLOCK
 *		sotoinpcb
 *		sotorawinpcb
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
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
 *      Base:   in_pcb.h        7.4 (Berkeley) 4/22/89
 *      Merged: in_pcb.h        7.6 (Berkeley) 6/28/90
 */

/*
 * Internet protocol hash queue structure
 */
struct inpcb_hash {
	struct inpcb_hash	*next;
	struct inpcb_hash	*prev;
};

#include <stddef.h>
#define	inhashtopcb(hp)		\
	(struct inpcb *)((char *)(hp) - offsetof(struct inpcb, inp_hash))
#define inpcbtohash(inp)	(&(inp->inp_hash))
#define INPCB_HASHSZ		65

#define INPCB_TCPHASH(faddr, fport, laddr, lport) \
	( \
	((u_long)(faddr) + \
	 (u_long)(laddr) + \
	 (u_long)((fport<<16) + lport)) % INPCB_HASHSZ \
	)

#define INPCB_UDPHASH(lport) ((lport) % INPCB_HASHSZ)

struct inpcb_hash_table {
	struct inpcb_hash 	head;
	struct inpcb_hash	*cache;
#ifdef	DEBUG
	int 			qlen;
	int 			maxq;
	int			cachemiss;
	int			cachehit;
#endif	/* DEBUG */
};

extern struct inpcb_hash_table tcp_pcb_hash_table[INPCB_HASHSZ];
extern struct inpcb_hash_table udp_pcb_hash_table[INPCB_HASHSZ];

/*
 * Common structure pcb for internet protocol implementation.
 * Here are stored pointers to local and foreign host table
 * entries, local and foreign socket numbers, and pointers
 * up (to a socket structure) and down (to a protocol-specific)
 * control block.
 */
struct inpcb {
	struct	inpcb *inp_next,*inp_prev;
					/* pointers to other pcb's */
	struct	inpcb *inp_head;	/* pointer back to chain of inpcb's
					   for this protocol */
	struct	in_addr inp_faddr;	/* foreign host table entry */
	u_short	inp_fport;		/* foreign port */
	struct	in_addr inp_laddr;	/* local host table entry */
	u_short	inp_lport;		/* local port */
	struct	socket *inp_socket;	/* back pointer to socket */
	caddr_t	inp_ppcb;		/* pointer to per-protocol pcb */
	struct	route inp_route;	/* placeholder for routing entry */
	int	inp_flags;		/* generic IP/datagram flags */
	struct	ip inp_ip;		/* header prototype; should have more */
	struct	mbuf *inp_options;	/* IP options */
	int	inp_refcnt;		/* reference count */
#if	defined(_KERNEL) && NETSYNC_LOCK
	simple_lock_data_t inp_lock;	/* structure lock */
	simple_lock_data_t inp_rc_lock;	/* refcnt lock */
#endif
#ifdef IP_MULTICAST
	struct	mbuf *inp_moptions;	/* IP multicast options */
#endif
	struct inpcb_hash inp_hash;	/* hash links			*/
};

/* flags in inp_flags: */
#define	INP_RECVOPTS		0x01	/* receive incoming IP options */
#define	INP_RECVRETOPTS		0x02	/* receive IP options for reply */
#define	INP_RECVDSTADDR		0x04	/* receive IP dst address */
#define	INP_CONTROLOPTS		(INP_RECVOPTS|INP_RECVRETOPTS|INP_RECVDSTADDR)

#ifdef sotorawcb
/*
 * Common structure pcb for raw internet protocol access.
 * Here are internet specific extensions to the raw control block,
 * and space is allocated to the necessary sockaddrs.
 */
struct raw_inpcb {
	struct	rawcb rinp_rcb;	/* common control block prefix */
	struct	mbuf *rinp_options;	/* IP options */
	int	rinp_flags;		/* flags, e.g. raw sockopts */
#define	RINPF_HDRINCL	0x1		/* user supplies entire IP header */
	struct	sockaddr_in rinp_faddr;	/* foreign address */
	struct	sockaddr_in rinp_laddr;	/* local address */
	struct	route rinp_route;	/* placeholder for routing entry */
#ifdef IP_MULTICAST
	struct	mbuf *rinp_moptions;	/* specific multicast options */
#endif
};
#endif

#define	INPLOOKUP_WILDCARD	1
#define	INPLOOKUP_SETLOCAL	2
#define	INPLOOKUP_USECACHE	4

#define	sotoinpcb(so)		((struct inpcb *)(so)->so_pcb)
#define	sotorawinpcb(so)	((struct raw_inpcb *)(so)->so_pcb)

#ifdef _KERNEL

extern CONST struct in_addr	zeroin_addr;

#if	NETSYNC_LOCK
/*
 * The INPCB lock is used to protect each connection on the system.
 *  It implicitly also protects the TCPCB or UDPCB control block
 *  it points to as well.  The socket it points to is protected
 *  separately. Note, however, that in the present code the INPCB
 *  lock is always taken after the socket lock (and never without),
 *  and so is redundant. For this reason the locks are present only
 *  if debugging. Additional parallelism may be achieved with them
 *  in the future.
 *
 * Taking the INPCB lock is a pain because the structure is linked onto
 *  a global (e.g. UDP or TCP) chain.  Holding the INPCB lock doesn't
 *  mean that someone else can't dequeue themselves from the chain;
 *  i.e. the "next" and "prev" pointers are protected by the global
 *  lock. For this reason the inpcb is reference counted.
 *
 * The lock hierarchy is socket first, inpcb second. A reference on an
 *  inpcb also protects the attached socket. Reference counts are
 *  incremented with the pcb head locked (for read or write). When
 *  the reference count goes to 0, the inpcb is deallocated, and
 *  sofree() is called. However, because sockets may live on from
 *  file table references, the socket unlock is left to the caller.
 *  
 * The chain heads are locked for write recursively to alleviate
 *  the situation in in_pcb.c and unconnected udp sends.
 */
#define	INPCB_LOCKINIT(inp)		simple_lock_init(&((inp)->inp_lock))
#define	INPCB_LOCK(inp)
#define	INPCB_UNLOCK(inp)
#define INPCB_ISLOCKED(inp)		1
#define	INHEAD_LOCK_DECL()		int	_inhl;
#define INHEAD_READ_LOCK(head)		_inhl = disable_lock(PL_IMP, &((head)->inp_lock))
#define	INHEAD_READ_UNLOCK(head) 	unlock_enable(_inhl, &((head)->inp_lock))
#define INHEAD_WRITE_LOCK(head)		_inhl = disable_lock(PL_IMP, &((head)->inp_lock))
#define	INHEAD_WRITE_UNLOCK(head) 	unlock_enable(_inhl, &((head)->inp_lock))
#define INHEAD_LOCKINIT(head)		INPCB_LOCKINIT(head)
#define INHEAD_ISLOCKED(head) 		1

/*
 * Locks and operations on refcounts.
 * Use of reference lock is not needed in AIX since we use atomic fetch and
 * add primitives for the reference/dereference defines.
 */
#define	INPCBRC_LOCKINIT(inp)
#define	INPCBRC_LOCK(inp)
#define	INPCBRC_UNLOCK(inp)

#define	INPCBRC_REF(inp) { \
	fetch_and_add(&((inp)->inp_refcnt), 1); \
}

#define	INPCBRC_UNREF(inp) { \
	if (fetch_and_add(&((inp)->inp_refcnt), -1) == 1) { \
		in_pcbfree(inp); \
	} \
}

#define	INPCBRC_UNREF_NOLOCK(inp) { \
	if (fetch_and_add(&((inp)->inp_refcnt), -1) == 1) { \
		in_pcbfree_nolock(inp); \
	} \
}

#else	/* !NETSYNC_LOCK */
#define	INPCB_LOCK(inp)
#define	INPCB_UNLOCK(inp)
#define	INPCB_LOCKINIT(inp)
#define	INPCB_ISLOCKED(inp)	1
#define INHEAD_READ_LOCK(head)
#define INHEAD_WRITE_LOCK(head)
#define	INHEAD_READ_UNLOCK(head)
#define	INHEAD_WRITE_UNLOCK(head)
#define INHEAD_LOCKINIT(head)
#define	INPCBRC_LOCKINIT(inp)
#define	INPCBRC_LOCK(inp)
#define	INPCBRC_UNLOCK(inp)
#define	INPCBRC_REF(inp)	(inp)->inp_refcnt++
#define	INPCBRC_UNREF(inp)	{if (--(inp)->inp_refcnt == 0) in_pcbfree(inp);}
#endif
#endif
