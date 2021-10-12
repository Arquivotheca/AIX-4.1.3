/* @(#)90	1.11.1.7  src/bos/usr/include/search.h, libcgen, bos411, 9428A410j 7/1/94 17:11:47 */

/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_SEARCH
#define _H_SEARCH

#ifndef _H_STANDARDS
#include <standards.h>
#endif /* _H_STANDARDS */

#ifndef _H_TYPES
#include <sys/types.h>
#endif /* _H_TYPES */

#ifdef _XOPEN_SOURCE

/* HSEARCH(3C) */
typedef struct entry { char *key; void *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

#ifdef	_NONSTD_TYPES
extern char *lsearch();
extern char *lfind();
extern char *tsearch();
extern char *tfind();
extern char *tdelete();

#elif	defined	_NO_PROTO

extern void *lsearch();
extern void *lfind();
extern void *tsearch();
extern void *tfind();
extern void *tdelete();

#else	/* _NONSTD_TYPES, _NO_PROTO */

extern void *lsearch(const void *, void *, size_t *, size_t,
		     int (*)(const void *, const void *));
extern void *lfind(const void *, const void *, size_t *, size_t, 
		   int (*)(const void *, const void *));
extern void *tsearch(const void *, void **, int (*)(const void *,const void *)); 
extern void *tfind(const void *, void * const *, int (*)(const void *,const void *));
extern void *tdelete(const void *, void **, int (*)(const void *,const void *));
#endif	/* _NONSTD_TYPES, _NO_PROTO */
 
#ifdef _NO_PROTO
extern int hcreate();
extern void hdestroy();
extern ENTRY *hsearch();
extern void twalk();

#ifdef _THREAD_SAFE
extern int hcreate_r();
extern void hdestroy_r();
extern ENTRY *hsearch_r();

#endif /* _THREAD_SAFE */

#else /* _NO_PROTO */

extern int hcreate(size_t);
extern void hdestroy(void);
extern ENTRY *hsearch(ENTRY, ACTION);
extern void twalk(const void *, void (*)(const void *,VISIT,int));

#ifdef _THREAD_SAFE
extern int hcreate_r(size_t size, struct hsearch_data *hsearch_data);
extern void hdestroy_r(struct hsearch_data *hsearch_data);
extern int hsearch_r(ENTRY item, ACTION action, ENTRY ** target,
			struct hsearch_data *hsearch_data);
#endif /* _THREAD_SAFE */

#endif /* _NO_PROTO */

#ifdef _THREAD_SAFE
struct hsearch_data{
	void *table;
	uint_t length;
	uint_t dim;
	uint_t prcnt;
	uint_t count;
};
#endif /* _THREAD_SAFE */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#ifdef _NO_PROTO
extern void insque();
extern void remque();
#else /* _NO_PROTO */
extern void insque(void *, void *);
extern void remque(void *);
#endif /* _NO_PROTO */
#endif /* _ALL_SOURCE */

#endif /* _H_SEARCH */
