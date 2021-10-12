/* @(#)15	1.6  src/bos/kernel/net/net_malloc.h, sysnet, bos41J, 9511A_all 3/15/95 14:52:40 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: BUCKETINDX
 *		FREE
 *		MALLOC
 *		NET_FREE
 *		NET_MALLOC
 *		btokup
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
/*
 * Copyright (c) 1987 Regents of the University of California.
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
 *	Base: malloc.h	7.16 (Berkeley) 6/28/90
 */

#ifndef	_NET_NET_MALLOC
#define _NET_NET_MALLOC

#define KMEMSTATS

/*
 * flags to malloc
 */
#define M_WAITOK	0x0000
#define M_NOWAIT	0x0001

/*
 * Types of memory to be allocated.
 */
#define	M_FREE		0	/* should be on free list */
#define M_MBUF		1	/* mbuf */
#define	M_CLUSTER	2	/* mbuf cluster page */
#define	M_SOCKET	3	/* socket structure */
#define	M_PCB		4	/* protocol control block */
#define	M_RTABLE	5	/* routing tables */
#define	M_FTABLE	6	/* fragment reassembly header */
#define	M_IFADDR	7	/* interface address */
#define	M_SOOPTS	8	/* socket options */
#define	M_SONAME	9	/* socket name */
#define	M_MBLK		10	/* mblk */
#define	M_MBDATA	11	/* mblk data */
#define	M_STRHEAD	12	/* Stream head */
#define	M_STRQUEUE	13	/* Streams queue pair */
#define	M_STRQBAND	14	/* Streams queue band */
#define	M_STRMODSW	15	/* Streams modsw */
#define	M_STRSIGS	16	/* Streams setsig */
#define	M_STRPOLLS	17	/* Streams poll */
#define	M_STROSR	18	/* Streams osr */
#define	M_STRSQ		19	/* Streams synch queue */
#define	M_STREAMS	20	/* misc Streams memory */
#define M_IOV		21	/* large iov's */
#define M_FHANDLE	22	/* network file handle */
#define	M_NFSREQ	23	/* NFS request header */
#define	M_NFSMNT	24	/* NFS mount structure */
#define M_FILE		25	/* file struct  */
#define M_FILEDESC	26	/* filedesc struct */
#define M_IOCTLOPS	27	/* ioctl data */
#define M_SELPOLL	28	/* select/poll arrays */
#define M_DEVBUF	29	/* device driver memory */
#define M_PATHNAME	30	/* pathname */
#define M_KTABLE	31	/* kernel table */
#define M_MOUNT		32	/* vfs mount struct */
#define M_SUPERBLK	33	/* super block data */
#define M_UFSMNT	34	/* UFS mount structure */
#define M_S5MNT		35	/* S5FS mount structure */
#define M_S5SUPERBLK	36	/* S5FS super block data */
#define M_VNODE		37	/* dynamically allocated vnodes */
#define M_SPECINFO	38	/* special file information */
#define M_SPECALIAS	39	/* special file alias */
#define M_SPECBUF	40	/* special file buffer */
#define M_LOCKF		41	/* byte-range locking structures */
#define M_DEVSW		42	/* device switch tables */
/* Available */
#define M_KALLOC	48	/* kalloc - obsolescent */
#define M_TEMP		49	/* misc temporary data buffers */
#define M_LAST		50

#define	KMEMNAMSZ	12
#define INITKMEMNAMES { \
	"free",		/* 0 M_FREE */ \
	"mbuf",		/* 1 M_MBUF */ \
	"mcluster",	/* 2 M_CLUSTER */ \
	"socket",	/* 3 M_SOCKET */ \
	"pcb",		/* 4 M_PCB */ \
	"routetbl",	/* 5 M_RTABLE */ \
	"fragtbl",	/* 6 M_FTABLE */ \
	"ifaddr",	/* 7 M_IFADDR */ \
	"soopts",	/* 8 M_SOOPTS */ \
	"soname",	/* 9 M_SONAME */ \
	"mblk",		/* 10 M_MBLK */ \
	"mblkdata",	/* 11 M_MBDATA */ \
	"strhead",	/* 12 M_STRHEAD */ \
	"strqueue",	/* 13 M_STRQUEUE */ \
	"strqband",	/* 14 M_STRQBAND */ \
	"strmodsw",	/* 15 M_STRMODSW */ \
	"strsigs",	/* 16 M_STRSIGS */ \
	"strpoll",	/* 17 M_STRPOLLS */ \
	"strosr",	/* 18 M_STROSR */ \
	"strsyncq",	/* 19 M_STRSQ */ \
	"streams",	/* 20 M_STREAMS */ \
	"iov",		/* 21 M_IOV */ \
	"fhandle",	/* 22 M_FHANDLE */ \
	"NFS req",	/* 23 M_NFSREQ */ \
	"NFS mount",	/* 24 M_NFSMNT */ \
	"file",		/* 25 M_FILE */ \
	"file desc",	/* 26 M_FILEDESC */ \
	"ioctlops",	/* 27 M_IOCTLOPS */ \
	"select/poll",	/* 28 M_SELPOLL */ \
	"devbuf",	/* 29 M_DEVBUF */ \
	"pathname",	/* 30 M_PATHNAME */ \
	"kernel table",	/* 31 M_KTABLE */ \
	"mount",	/* 32 M_MOUNT */ \
	"superblock",	/* 33 M_SUPERBLK */ \
	"UFS mount",	/* 34 M_UFSMNT */ \
	"S5FS mount",	/* 35 M_S5MNT */ \
	"S5FS superbk",	/* 36 M_S5SUPERBLK */ \
	"vnode",	/* 37 M_VNODE */ \
	"spec info",	/* 38 M_SPECINFO */ \
	"spec alias", 	/* 39 M_SPECALIAS */ \
	"spec buf",	/* 40 M_SPECBUF */ \
	"locking",	/* 41 M_LOCKF */ \
	"devsw",	/* 42 M_DEVSW */ \
	"", "", "", "", "", \
	"kalloc",	/* 48 M_KALLOC */ \
	"temp",		/* 49 M_TEMP */ \
}

#ifdef	_KERNEL
#ifdef	_AIX_FULLOSF
#include <kern/lock.h>
#else
#include <sys/param.h>
#endif
#endif

#define	MINBUCKET	5			/* param.h in osf */

struct kmemstats {
	long	ks_inuse;	/* # of packets of this type currently in use */
	long	ks_calls;	/* total packets of this type ever allocated */
	long 	ks_memuse;	/* total memory held in bytes */
	long	ks_limblocks;	/* number of times blocked for hitting limit */
	long	ks_mapblocks;	/* number of times blocked for kernel map */
	long	ks_maxused;	/* maximum number ever used */
	long	ks_limit;	/* most that are allowed to exist */
	long	ks_failed;	/* total failed allocations */
#ifdef	_KERNEL
	simple_lock_data_t	ks_lock;
#endif
};

/*
 * Array of descriptors that describe the contents of each page
 */
struct kmemusage {
	short ku_indx;		/* bucket index */
	union {
		u_short freecnt;/* for small allocations, free pieces in page */
		u_short pagecnt;/* for large allocations, pages alloced */
	} ku_un;
	int ku_cpu;		/* Indicates which bucket array to free on */
};
#define ku_freecnt ku_un.freecnt
#define ku_pagecnt ku_un.pagecnt

/*
 * Set of buckets for each size of memory block that is retained
 */
struct kmembuckets {
	void *	kb_next;	/* list of free blocks */
	long	kb_calls;	/* total calls to allocate this size */
	long	kb_total;	/* total number of blocks allocated */
	long	kb_totalfree;	/* # of free elements in this bucket */
	long	kb_elmpercl;	/* # of elements in this sized allocation */
	long	kb_highwat;	/* high water mark */
	long	kb_couldfree;	/* over high water mark and could free */
	long	kb_failed;	/* total failed allocations */
#ifdef	_KERNEL
	simple_lock_data_t	kb_lock;
#endif
};

#ifdef _KERNEL
#define MINALLOCSIZE	(1 << MINBUCKET)
#define BUCKETINDX(size) \
	( (size) == 256 ? (MINBUCKET + 3) \
	: (size) == 4096 ? (MINBUCKET + 7) \
	: (size) <= (MINALLOCSIZE * 128) \
		? (size) <= (MINALLOCSIZE * 8) \
			? (size) <= (MINALLOCSIZE * 2) \
				? (size) <= (MINALLOCSIZE * 1) \
					? (MINBUCKET + 0) \
					: (MINBUCKET + 1) \
				: (size) <= (MINALLOCSIZE * 4) \
					? (MINBUCKET + 2) \
					: (MINBUCKET + 3) \
			: (size) <= (MINALLOCSIZE* 32) \
				? (size) <= (MINALLOCSIZE * 16) \
					? (MINBUCKET + 4) \
					: (MINBUCKET + 5) \
				: (size) <= (MINALLOCSIZE * 64) \
					? (MINBUCKET + 6) \
					: (MINBUCKET + 7) \
		: (size) <= (MINALLOCSIZE * 2048) \
			? (size) <= (MINALLOCSIZE * 512) \
				? (size) <= (MINALLOCSIZE * 256) \
					? (MINBUCKET + 8) \
					: (MINBUCKET + 9) \
				: (size) <= (MINALLOCSIZE * 1024) \
					? (MINBUCKET + 10) \
					: (MINBUCKET + 11) \
			: (size) <= (MINALLOCSIZE * 8192) \
				? (size) <= (MINALLOCSIZE * 4096) \
					? (MINBUCKET + 12) \
					: (MINBUCKET + 13) \
				: (size) <= (MINALLOCSIZE * 16384) \
					? (MINBUCKET + 14) \
					: (MINBUCKET + 15) )

/*
 * Turn virtual addresses into kmem map indicies
 */
#define btokup(addr)	(&kmemusage[((char *)(addr) - (char *)kmembase) / PAGE_SIZE])

/*
 * Macro versions for the usual cases of malloc/free
 * See malloc source for discussion of operation if !KMEMSTATS.
 */
#ifdef KMEMSTATS

#define MALLOC(space, cast, size, type, flags) \
	(space) = (cast)net_malloc((u_long)(size), type, flags)
#define FREE(addr, type) net_free((void *)(addr), type)

extern struct kmemstats kmemstats[M_LAST];
extern const char kmemnames[M_LAST][KMEMNAMSZ];

#else /* do not collect statistics or garbage collect */

#define MALLOC(space, cast, size, type, flags) { \
	register struct kmembuckets *kbp = &bucket[BUCKETINDX(size)]; \
	int s = splhigh(); \
	simple_lock(&kbp->kb_lock); \
	if (kbp->kb_next == NULL) { \
		simple_unlock(&kbp->kb_lock); \
		splx(s); \
		(space) = (cast)net_malloc((u_long)(size), type, flags); \
	} else { \
		(space) = (cast)kbp->kb_next; \
		kbp->kb_next = *(void **)(space); \
		simple_unlock(&kbp->kb_lock); \
		splx(s); \
	} \
}

#define FREE(addr, type) { \
	register struct kmembuckets *kbp; \
	register struct kmemusage *kup = btokup(addr); \
	if (1 << kup->ku_indx > MAXALLOCSAVE) \
		free((void *)(addr), type); \
	else { \
		int s = splhigh(); \
		simple_lock(&kbp->kb_lock); \
		kbp = &bucket[kup->ku_indx]; \
		*(void **)(addr) = kbp->kb_next; \
		kbp->kb_next = (void *)(addr); \
		simple_unlock(&kbp->kb_lock); \
		splx(s); \
	} \
}
#endif /* do not collect statistics or garbage collect */

extern void *kmembase;
extern struct kmemusage *kmemusage;
extern struct kmembuckets bucket[][MINBUCKET + 16];

extern void *net_malloc(u_long, int, int);
extern void net_free(void *, int);
extern int  kmemsetlimit(int, long);
extern void kmeminit(void);
extern void kmeminit_thread(int);

#define	NET_MALLOC(p,c,s,t,f)	MALLOC(p,c,s,t,f)
#define NET_FREE(p,t)		FREE(p,t)

#endif	/* _KERNEL */
#endif	/* _NET_NET_MALLOC */
