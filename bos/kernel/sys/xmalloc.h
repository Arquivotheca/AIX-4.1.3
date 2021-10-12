/* @(#)52	1.13  src/bos/kernel/sys/xmalloc.h, sysldr, bos41J, 9508A 2/15/95 10:34:28 */

#ifndef _H_XMALLOC
#define _H_XMALLOC

/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: data structures used in xmalloc() which are of interest
 *            to other programs, e.g. crash. see also malloc.h for 
 *            external definition of xmalloc().
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct record {
	struct record *	next;
	char *	addr;
	uint	req_size;
	uint	from;
	} ;

struct	heap {
	int	sanity;		/*HEAP*/
	ulong	base;		/*offset of start of heap region from this heap*/
	Simple_lock lock;	/*allocation lock*/
	int	alt;		/*zero for primary heap, 1 for alternate heap */
	uint	numpages;	/*size of useable heap - for checking */
	uint	amount;		/*amount allocated for checking*/
	uint	pinflag:1;	/*allocated storage should be pinned*/
	uint	vmrelflag:1;	/*release virtual pages after freeing space*/
	struct record ** rhash; /* pointer to rec_hash array */
	uint	pagtot;		/*measurement data*/
	uint	pagused;	/*...*/
	uint	frtot[PGSHIFT];	/*measurement data*/
	uint	frused[PGSHIFT];	/*...*/
	ulong	fr[PGSHIFT];	/*table of size chains*/
};

#endif   /* _H_XMALLOC */
