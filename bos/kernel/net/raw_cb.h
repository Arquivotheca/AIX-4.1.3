/* @(#)63	1.13  src/bos/kernel/net/raw_cb.h, sockinc, bos411, 9428A410j 4/25/94 17:11:59 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: RAW_LOCK
 *		RAW_LOCKINIT
 *		RAW_UNLOCK
 *		sotorawcb
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
 * OSF/1 1.1 Snapshot 2
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
 * Copyright (c) 1980, 1986 Regents of the University of California.
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
 *	Base:	raw_cb.h	7.5 (Berkeley) 4/22/89
 *	Merged:	raw_cb.h	7.6 (Berkeley) 6/28/90
 */

/*
 * Raw protocol interface control block.  Used
 * to tie a socket to the generic raw interface.
 */
struct rawcb {
	struct	rawcb *rcb_next;	/* doubly linked list */
	struct	rawcb *rcb_prev;
	struct	socket *rcb_socket;	/* back pointer to socket */
	struct	sockaddr *rcb_faddr;	/* destination address */
	struct	sockaddr *rcb_laddr;	/* socket's address */
	struct	sockproto rcb_proto;	/* protocol family, protocol */
};

#define	sotorawcb(so)		((struct rawcb *)(so)->so_pcb)

/*
 * Nominal space allocated to a raw socket.
 */
#define	RAWSNDQ		8192
#define	RAWRCVQ		8192

#ifdef	_KERNEL
#if	NETSYNC_LOCK
extern simple_lock_data_t global_raw_lock;
extern int		  global_raw_intpri;
#define	RAW_LOCK_DECL()	
#define RAW_LOCKINIT()	{						\
	lock_alloc(&global_raw_lock, LOCK_ALLOC_PIN, RAW_LOCK_FAMILY, -1);\
	simple_lock_init(&global_raw_lock);			\
}
#define RAW_LOCK()   global_raw_intpri = disable_lock(PL_IMP, &global_raw_lock);
#define RAW_UNLOCK() unlock_enable(global_raw_intpri, &global_raw_lock);
#else
#define RAW_LOCKINIT()
#define RAW_LOCK()
#define RAW_UNLOCK()
#endif

extern struct rawcb rawcb;		/* head of list */
#endif
