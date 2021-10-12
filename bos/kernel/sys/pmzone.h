/* @(#)54	1.5  src/bos/kernel/sys/pmzone.h, sysproc, bos411, 9428A410j 5/10/94 14:58:28 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   ORIGIN: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef	_H_PMZONE
#define _H_PMZONE

#include <sys/lock_def.h>

/*
 *	A zone is a collection of fixed size blocks for which there
 *	is fast allocation/deallocation access.  Kernel routines can
 *	use zones to manage data structures dynamically, creating a zone
 *	for each type of data structure to be managed.
 *
 *	Note: The link word must be unused when the element is free. Any other
 *	words may be used even when the element is free (like p_stat which
 *	holds SNONE).
 */

#define PM_ZERO		0x00000001	/* when allocated, an entry is zeroed */
#define PM_FIFO		0x00000002	/* the allocation scheme is FIFO */

typedef struct pm_heap {
	Simple_lock	lock;		/* zone lock */
	char		*lastin;	/* last freed entry */
	char		*highwater;	/* just beyond last used (monotonic) */
	char		*start;		/* beginning of the zone */
	char		*end;		/* just beyond end of the zone */
	unsigned short	size;		/* size of an element */
	unsigned short	link;		/* offset of link word in an element */
	long		flags;		/* zone flags */
	long		pad;
};

#ifdef _KERNEL
int pm_init(struct pm_heap *zone, char *start, char *end, unsigned short size,
		unsigned short link, short class, short occurrence, long flags);
void pm_release(struct pm_heap *zone);
char *pm_alloc(struct pm_heap *zone, char *error);
void pm_free(struct pm_heap *zone, char *element);

#define round_down(addr,size)	(char *)((int)(addr) & ~(size-1))
#define round_up(addr,size)	round_down((int)(addr)+(size)-1,(size))
#endif

#endif	/* _H_PMZONE */
