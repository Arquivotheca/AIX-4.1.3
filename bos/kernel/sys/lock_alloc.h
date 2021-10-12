/* @(#)87	1.8  src/bos/kernel/sys/lock_alloc.h, sysproc, bos412, 9446B 11/14/94 16:33:07 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_LOCK_ALLOC
#define _H_LOCK_ALLOC

#include <sys/param.h>

/* lock allocation/deallocation primitives */
#if defined(_POWER_MP) || !defined(_KERNSYS)
#ifdef _NO_PROTO

void lock_alloc();
void lock_free();

#else /* _NO_PROTO */

void lock_alloc(void *,int,short,short);
void lock_free(void *);

#endif /* _NO_PROTO */
#else /* _POWER_MP || !_KERNSYS */

#define lock_alloc(lockaddr,flags,class,occurrence)
#define lock_free(l)

#endif /* _POWER_MP || !_KERNSYS */

/* lock_alloc: flags define */
#define LOCK_ALLOC_PIN		   1
#define	LOCK_ALLOC_PAGED	   2
#define LOCK_ALLOC_REINIT	   4	/* re-initialize if necessary */

#ifdef _KERNSYS
#include <sys/lock_def.h>

extern struct lock_data_instrumented 	family_lock_statistics[];
extern struct lock_data_instrumented	lock_pinned[];
extern struct lock_data_instrumented	lock_pageable[];
extern struct lock_data_instrumented 	problem_lock[];

/* lock allocation highwater mark: to be tuned */
#define LOCK_HIGHWATERMARK	  32

#define MAX_FAMILY              2048    /* max number of lock classes */
#define MAX_PROBLEMLOCKS        1024    /* threshold on number of lock being overused */

/*
 * maximum number of locks per table
 * note: this assumes that the pinned and pageable tables are the same size
 * and that the pageable table immediately follows the pinned table
 */
#define MAX_LOCK (((uint)lock_pageable - (uint)lock_pinned) / sizeof(struct lock_data_instrumented))

/* number of instrumented locks per page */
#define LOCK_PER_PAGE		(PAGESIZE/sizeof(struct lock_data_instrumented))

/* klockd: defines for threshold on use (initial setting) */
#define LOCK_MISSPERCENT           1
#define MINLOCK_ACQUIRE         1000

/* klockd 15 minutes interval timer */
#define LOCKD_INTERVAL           900	/* seconds */
#define LOCKD_EVENT_TIMER       0x00000800 /* something larger than EVENT_KERNEL */

/* lock pool control block */

struct lock_pool {
	int 				allocated;	/* # of allocated entry */
	int 				free;		/* # of free entry      */
	struct lock_data_instrumented	*free_list;	/* pointer to first free entry */
	struct lock_data_instrumented	*end_list; 	/* pointer to last allocated entry */
	struct lock_data_instrumented	*start; 	/* pointer to beginning of area to be allocated */
	Simple_lock			pool_lock;	/* lock on control block */
	};

#endif /* _KERNSYS */
#endif /* _H_LOCK_ALLOC */
