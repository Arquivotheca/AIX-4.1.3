static char sccsid[] = "@(#)53  1.8  src/bos/kernel/proc/pmzone.c, sysproc, bos41J, 9507A 2/6/95 15:03:59";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: pm_alloc
 *		pm_free
 *		pm_init
 *		pm_release
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


/*
 * NOTES:
 *	Zone-based memory allocator.  A zone is a collection of fixed size
 *	data blocks for which quick allocation/deallocation is possible.
 *
 *	A zone is pinned up to its highwater mark. That mark never decreases.
 *	The step of allocation is a page, which implies that the size of
 *	element is less or equal than a page, but it does not have to accomodate
 *	a whole number of elements per page.
 *	The highwater mark points to the first unpinned entry, not the first
 *	never used so far entry.
 *
 *	When the allocation scheme used is LIFO, the lastin is the first
 *	available and the list of free entries is NULL terminated. Allocating
 *	an element means removing lastin and the next in the list becomes
 *	lastin. Freeing an element means adding it before lastin and it becomes
 *	the new lastin.
 *
 *	When the allocation scheme used is FIFO, the lastin is the last
 *	available and the list of free entries is circular. Allocating an
 *	element means removing the lastin's follower; lastin does not change
 *	unless the list has been emptied, in which case it becomes NULL.
 *	Freeing an element means adding it after lastin and it becomes the new
 *	lastin.
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/pmzone.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/syspest.h>
#include <sys/proc.h>
#include <sys/var.h>

/* reserve dump table slots */
#define REC_PER_PROC    2               /* records per process          */
#define REC_PER_THREAD  3               /* records per thread           */

static ulong proc_gen = 0;              /* PID generation counter       */
static ulong thread_gen = 1;            /* TID generation counter       */
static int nproc_to_dump;               /* number of items to dump      */
static int nthread_to_dump;             /* number of items to dump      */

extern struct pm_heap proc_cb;
extern struct pm_heap thread_cb;
extern void alloc_pcdt();    	     /* allocate proc component dump tab */
extern void alloc_tcdt();          /* allocate thread component dump tab */


/*
 * NAME: pm_init
 *
 * FUNCTION: initializes a zone descriptor.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from process environment.
 *
 * RETURNS:
 *	Maximum number of elements the zone can hold.
 */
int
pm_init(struct pm_heap *zone, char *start, char *end, unsigned short size,
	unsigned short link, short class, short occurrence, long flags)
/* struct pm_heap *zone;	zone descriptor */
/* char *start;			zone beginning */
/* char *end;			zone end */
/* unsigned short size;		size of an element */
/* unsigned short link;		offset in element for free entries linkage */
/* short class;			class of the lock protecting the zone */
/* short occurrence;		occurrence of the lock protecting the zone */
/* long flags;			zone flags */
{
	ASSERT(!((int)start & (PAGESIZE-1)));	/* page aligned */
	ASSERT(!((int)end & (PAGESIZE-1)));	/* page aligned */
	ASSERT(end > start);			/* not empty */
	zone->start = start;
	zone->end   = end;

	ASSERT(!(size & (sizeof(int)-1)));	/* word aligned */
	ASSERT(size <= PAGESIZE);		/* element must fit in a page */
	zone->size  = size;

	ASSERT(!(link & (sizeof(int)-1)));	/* word aligned */
	ASSERT(link < size);			/* link must be inside element*/
	zone->link  = link;

	zone->flags = flags;
	zone->lastin = NULL;
	zone->highwater = zone->start;

	lock_alloc(&zone->lock, LOCK_ALLOC_PIN, class, occurrence);
	simple_lock_init(&zone->lock);

	return((end - start) / size);
}

/*
 * NAME: pm_release
 *
 * FUNCTION: releases a zone descriptor.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from process environment.
 *
 * NOTES:
 *	Must never be called if somebody is still using the zone.
 *
 * RETURNS:
 * 	None.
 */
void
pm_release (struct pm_heap *zone)
/* struct pm_heap *zone;	zone descriptor */
{
	char *start = zone->start;
	char *highwater = zone->highwater;

	/* Free the lock */
	lock_free(&zone->lock);

	/* Unpin the zone */
	if (highwater - start)
		(void)ltunpin(start, highwater - start);
}

/*
 * NAME: pm_alloc
 *
 * FUNCTION: allocates an element from the specified zone.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from process environment.
 *
 * NOTES:
 *	Must never be called before pm_init() or after pm_release().
 *
 * RETURNS:
 *	NULL if unsuccessful, error may be
 *		EAGAIN : no more room in the zone.
 *		ENOMEM : no more pinnable memory in the system.
 *	address of the allocated entry otherwise.
 */
char *
pm_alloc(struct pm_heap *zone, char *error)
{
	char *element;

	/* Lock access to the table */
	simple_lock(&zone->lock);

	/* No more free pinned entries ? */
	if (zone->lastin == NULL)
	{
		/* Not enough room in the zone to accomodate a new element ? */
		if (zone->highwater + zone->size > zone->end) {
			simple_unlock(&zone->lock);
			*error = EAGAIN;
			return(NULL);
		}

		/*
		 * Since zone->highwater is the first unpinned entry, we are
		 * sure that we will cross a page boundary for the next entry
		 * and since zone->end is page-aligned, we are sure that the
		 * next page is fully in the zone.
		 */
		if (ltpin(round_down(zone->highwater+zone->size-1, PAGESIZE),
								    PAGESIZE)) {
			simple_unlock(&zone->lock);
			*error = ENOMEM;
			return(NULL);
		}

		/* Link the new pinned entries together in the free list */
		{
			char *current;
			char *next, *limit;

			current = zone->highwater;
			next = current + zone->size;
			limit = round_up(next, PAGESIZE) - zone->size;
			while (next <= limit) {
			    *(char **)(current + zone->link) = next;
			    current = next;
			    next += zone->size;
			}
			if (zone->flags & PM_FIFO) {
			    *(char **)(current + zone->link) = zone->highwater;
			    zone->lastin = current;
			} else {
			    *(char **)(current + zone->link) = NULL;
			    zone->lastin = zone->highwater;
			}
			zone->highwater = next;
		}
	}

	/* Get an element and update zone descriptor */
	if (zone->flags & PM_FIFO) {
		element = *(char **)(zone->lastin + zone->link);
		if (element == zone->lastin)
			zone->lastin = NULL;
		else
			*(char **)(zone->lastin + zone->link) =
					       *(char **)(element + zone->link);
	} else {
		element = zone->lastin;
		zone->lastin = *(char **)(element + zone->link);
	}

	/* Zero out the new element if requested */
	if (zone->flags & PM_ZERO)
		bzero (element, zone->size);

	if (zone == &proc_cb) {
		struct proc *p;

        	/* allocate pinned memory for kernel dump */
        	nproc_to_dump += REC_PER_PROC;
        	alloc_pcdt(nproc_to_dump);

        	v.ve_proc = proc_cb.highwater;

        	/* assign a unique process ID */
        	proc_gen &= PGENMASK;
		p = (struct proc *)element;
        	p->p_pid = ((p-proc) << PGENSHIFT) + (int)(proc_gen);
        	proc_gen += 2;
	}
	else if (zone == &thread_cb) {
		struct thread *t;

        	/* allocate pinned memory for kernel dump */
        	nthread_to_dump += REC_PER_THREAD;
        	alloc_tcdt(nthread_to_dump);

        	v.ve_thread = thread_cb.highwater;

        	/* assign a unique thread ID */
        	thread_gen &= TGENMASK;
		t = (struct thread *)element;
        	t->t_tid = ((t-thread) << TGENSHIFT) + (int)(thread_gen);
        	thread_gen += 2;
	}

	/* Unlock access to the table */
	simple_unlock(&zone->lock);

	return (element);
}

/*
 * NAME: pm_free
 *
 * FUNCTION: deallocates an element from the specified zone.
 *
 * EXECUTION ENVIRONMENT:
 *      Called from process environment.
 *
 * NOTES:
 *	Must never be called before pm_init() or after pm_release().
 *	It is impossible to check for something being freed twice!
 *
 * RETURNS:
 *	None.
 */
void
pm_free (struct pm_heap *zone, char *element)
{

	/* Lock access to the table */
	simple_lock(&zone->lock);

	/* Free the element and update zone descriptor */
	if (zone->flags & PM_FIFO) {
		if (zone->lastin == NULL)
			*(char **)(element + zone->link) = element;
		else {
			*(char **)(element + zone->link) =
					  *(char **)(zone->lastin + zone->link);
			*(char **)(zone->lastin + zone->link) = element;
		}
	} else
		*(char **)(element + zone->link) = zone->lastin;
	zone->lastin = element;

	if (zone == &proc_cb) {
        	/* deallocate pinned memory for kernel dump of procs and u */
        	nproc_to_dump -= REC_PER_PROC;
        	alloc_pcdt(nproc_to_dump);
	}
	else if (zone == &thread_cb) {
        	/* deallocate pinned memory for kernel dump of threads */
        	nthread_to_dump -= REC_PER_THREAD;
        	alloc_tcdt(nthread_to_dump);
	}
		
	/* Unlock access to the table */
	simple_unlock(&zone->lock);
}
