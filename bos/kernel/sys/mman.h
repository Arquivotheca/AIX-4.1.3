/* @(#)35	1.7  src/bos/kernel/sys/mman.h, sysvmm, bos411, 9428A410j 12/7/93 18:36:45 */
#ifndef	_H_MMAN
#define _H_MMAN

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 65 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mman.h	7.1 (Berkeley) 6/4/86
 */

/* protections are chosen from these bits, or-ed together */
#define PROT_NONE	0		/* no access to these pages */
#define PROT_READ	0x1		/* pages can be read */
#define PROT_WRITE	0x2		/* pages can be written */
#define PROT_EXEC	0x4		/* pages can be executed */

/* flags contain sharing type, mapping type, and options */

/* mapping visibility: choose either SHARED or PRIVATE */
#define MAP_SHARED	0x1		/* share changes */
#define MAP_PRIVATE	0x2		/* changes are private */

/* mapping region: choose either FILE or ANONYMOUS */
#define	MAP_FILE	0x00		/* map from a file */
#define	MAP_ANONYMOUS	0x10		/* map an unnamed region */
#define	MAP_ANON	0x10		/* map an unnamed region */
#define	MAP_TYPE	0xf0		/* the type of the region */

/* mapping placement: choose either FIXED or VARIABLE */
#define	MAP_FIXED	0x100		/* map addr must be exactly as specified */
#define	MAP_VARIABLE	0x00		/* system can place new region */

/* advice to madvise */
#define MADV_NORMAL	0		/* no further special treatment */
#define MADV_RANDOM	1		/* expect random page references */
#define MADV_SEQUENTIAL	2		/* expect sequential page references */
#define MADV_WILLNEED	3		/* will need these pages */
#define MADV_DONTNEED	4		/* dont need these pages */
#define	MADV_SPACEAVAIL	5		/* ensure that resources are available */

/* msem conditions and structure */
typedef struct {
	int msem_state;		/* The semaphore is locked if non-zero. */
	int msem_wanted;	/* Processes are waiting on the semaphore. */
}msemaphore;

#define MSEM_UNLOCKED 	0	/* Initialize the semaphore to unlocked */
#define MSEM_LOCKED 	1	/* Initialize the semaphore to locked */
#define MSEM_IF_NOWAIT	2	/* Do not wait if semaphore is locked */
#define MSEM_IF_WAITERS	3	/* Unlock only if there are waiters */

/* msync flags */
#define MS_ASYNC 1		/* Asynchronous cache flush */
#define MS_SYNC  3		/* Synchronous cache flush */
#define MS_INVALIDATE 4		/* Invalidate cached pages */

#if !defined(_NO_PROTO) && !defined(_KERNEL)

extern int	madvise(caddr_t addr, size_t len, int behav);
extern int	mincore(caddr_t addr, size_t len, char *vec);
extern void 	*mmap(void *addr, size_t len, int prot, int flags, int filedes, off_t off);
extern int	mprotect(void *addr, size_t len, int prot);
extern msemaphore 	*msem_init(msemaphore *sem, int initial_value);
extern int	msem_lock(msemaphore *sem, int condition);
extern int	msem_remove(msemaphore *sem);
extern int	msem_unlock(msemaphore *sem, int condition);
extern int	msync(void *addr, size_t len, int flags);
extern int	munmap(void *addr, size_t len);

#else /* _NO_PROTO || _KERNEL */

extern int	madvise();
extern int	mincore();
extern caddr_t	mmap();
extern int	mprotect();
extern msemaphore 	*msem_init();
extern int	msem_lock();
extern int	msem_remove();
extern int	msem_unlock();
extern int	msync();
extern int	munmap();

#endif /* _NO_PROTO || _KERNEL */

#endif	/* _H_MMAN */

