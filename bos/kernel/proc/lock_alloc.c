static char sccsid[] = "@(#)83	1.15.1.3  src/bos/kernel/proc/lock_alloc.c, sysproc, bos412, 9447B 11/21/94 09:44:40";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: lock_alloc
 *		lock_alloc_instr
 *		lock_free
 *		lock_free_instr
 *		lockd_timer_post
 *
 *   ORIGINS: 27 83
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
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * NOTES:
 */

#include <sys/intr.h>
#include <sys/param.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/atomic_op.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/systm.h>
#include <sys/sleep.h>
#include <sys/syspest.h>

#ifdef _INSTRUMENTATION

/* to be defined: with this value all locks are spinning unless the lock's
   owner is not running */
unsigned int maxspin = MAXSPIN_UP;

/* tunable variables */
unsigned int lock_highwatermark = LOCK_HIGHWATERMARK;

/* pinned and pageable pools control block */
struct	lock_pool	pinned_cb;
struct	lock_pool	pageable_cb;

/* lock on family lock statistics table */
Simple_lock	family_table_lock;

#ifdef DEBUG
/*
 * in this file, under DEBUG, we use the
 * link_register for more than just trace hooks
 */
#ifdef GET_RETURN_ADDRESS
#undef GET_RETURN_ADDRESS
#endif
#define GET_RETURN_ADDRESS(x)				\
{							\
	extern int *get_stkp();				\
	struct stack_frame {				\
		struct stack_frame *next;		\
		int                 unused;		\
		int                 link_register;	\
	} *sf;						\
	sf = (struct stack_frame *)get_stkp();		\
	sf = sf->next;					\
	x = sf->link_register;				\
}
#endif

/*
 * NAME:     lock_alloc_instr
 *
 * FUNCTION: allocate a lock_data_instrumented structure from a lock pool
 *          
 *
 * EXECUTION ENVIRONMENT: called by process environment; can be invoked by
 *			  interrupt handler but this should be ready to get
 *			  an error return code if no more structure are
 *			  available.
 *        
 *
 * RETURNS:  nothing
 *           
 *
 */

void
lock_alloc_instr(void *lockaddr, int flags, short class, short occurrence)

{
	struct lock_data_instrumented *l;
	int 	old_pri;
	register int link_register;
	register int pin_rc = -1;
	register struct lock_data_instrumented *address_pinned = 0;
#ifdef DEBUG
	register struct thread *myself = curthread;
#endif

	/* Get caller of this routine */
	GET_RETURN_ADDRESS(link_register);

	/* trace lock allocation */
	TRCHKL4T(HKWD_KERN_LOCKALLOC,lockaddr,class,occurrence,link_register);
	
	/*
	 * validate flags:
	 * 	1. either LOCK_ALLOC_PIN or LOCK_ALLOC_PAGED should be on, 
	 *	   but not both
	 * 	2. if LOCK_ALLOC_REINIT is on, either *lockaddr is zero, 
	 *	   or it points to a valid lock structure.
	 */
	assert(flags & (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED));
	ASSERT((flags & (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED)) != (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED));

	if ( !(flags & LOCK_ALLOC_REINIT)) {
		/*
		 * Initialize lock in case allocation fails.
		 * Not necessary in re-init case, since it should have worked
		 * the first time.
		 */
		((simple_lock_t)lockaddr)->_slock = 0;
	}

	/*
	 * Allocate a secondary structure to hold lock instrumentation data,
	 * if one is not already present.  In the re-init case, where it might
	 * be present, we depend on the caller to set *lockaddr to zero (as 
	 * opposed to leaving it uninitialized) to allow us to tell if this 
	 * needs to be done.
	 */
	if (!(flags & LOCK_ALLOC_REINIT) || 0 == ((simple_lock_t)lockaddr)->_slock)
	/* allocate from the pinned pool */
	if (flags & LOCK_ALLOC_PIN){
		/* unsafely check if new page is needed */
		if (pinned_cb.free < lock_highwatermark) {
			if ((pinned_cb.allocated < (MAX_LOCK - LOCK_PER_PAGE)) && (csa->intpri == INTBASE)) {
				/* pin page outside of lock */
				address_pinned = pinned_cb.start;
				pin_rc = ltpin(address_pinned, PAGESIZE);
			}
		}

		old_pri = disable_lock(INTMAX, &pinned_cb.pool_lock);
	
		/* if the highwater mark has been crossed, a new page of lock
		 * instrumentation structure needs to be allocated.
		 * The allocation (pinning of the page) is not performed if 
		 * the last page of structures has already been allocated or 
		 * if the requestor is disabled and the free list is empty or 
		 * no more pinnable pages are available
		 */
		
		if ((pin_rc == 0) && (pinned_cb.start == address_pinned)) {
			/* page was pinned prior to taking the lock by caller */
			if (pinned_cb.free) {
				/* end_list points to unallocated lock */
				ASSERT(pinned_cb.free_list != NULL);
				pinned_cb.end_list->lo_next = pinned_cb.start;
			} else {
				ASSERT(pinned_cb.free_list == NULL);
				pinned_cb.free_list = pinned_cb.start;
			}
			for( l = &(pinned_cb.start[0]); l < &(pinned_cb.start[LOCK_PER_PAGE]); l++) {
				l->lo_next = l + 1;
				l->lockname = NULL;
#ifdef DEBUG
				l->dbg_zero = 0;  /* matches 'lockname' field for non-DEBUG locks */
				l->dbg_flags = 0; /* clear debug flags */
#endif
			}
			pinned_cb.start = l;
			(--l)->lo_next = NULL;
			pinned_cb.end_list = l;
			pinned_cb.allocated += LOCK_PER_PAGE;
			pinned_cb.free += LOCK_PER_PAGE;
		}

		/* need to check if free count is non-zero */
		if (pinned_cb.free == 0) {
			unlock_enable(old_pri, &pinned_cb.pool_lock);
			return;
		}

		/* 
		 * At least one free entry has been found--use it
		 * Note the assumption that the structures for simple and
		 * complex locks are sufficiently alike so that the one for
		 * simple locks can be used for both.
		 */
		((simple_lock_t)lockaddr)->_slockp = pinned_cb.free_list;
		pinned_cb.free_list = (((simple_lock_t)lockaddr)->_slockp)->lo_next;	
		pinned_cb.free--;
#ifdef DEBUG
		assert(!((((simple_lock_t)lockaddr)->_slockp)->dbg_flags & LOCK_IS_ALLOCATED));
#endif
		bzero(((simple_lock_t)lockaddr)->_slockp,sizeof(struct lock_data_instrumented));
		(((simple_lock_t)lockaddr)->_slockp)->lock_control_word.s_lock = SIMPLE_LOCK_AVAIL;
#ifdef DEBUG
		(((simple_lock_t)lockaddr)->_slockp)->dbg_flags |= LOCK_IS_ALLOCATED;
		(((simple_lock_t)lockaddr)->_slockp)->lock_lr = link_register;
		(((simple_lock_t)lockaddr)->_slockp)->lock_caller = myself->t_tid;
		(((simple_lock_t)lockaddr)->_slockp)->lock_cpuid = get_processor_id();
#endif
		unlock_enable(old_pri, &pinned_cb.pool_lock);
	}
	
	/* allocate from the pageable pool */
	else {
		assert(csa->intpri == INTBASE);
		simple_lock (&pageable_cb.pool_lock);
		
		/* if the highwater mark has been crossed, a new page of lock
		 * instrumentation structure needs to be allocated.
		 * The allocation is not performed if the last page of structures
		 * has already been allocated; in this case if the free list is empty
		 * lock_alloc returns
		 */
		
		if (pageable_cb.free < lock_highwatermark) {
			if (pageable_cb.allocated < (MAX_LOCK - LOCK_PER_PAGE)) {
				if (pageable_cb.free) {
					/* end_list points to unallocated lock */
					ASSERT(pageable_cb.free_list != NULL);
					pageable_cb.end_list->lo_next = pageable_cb.start;
				} else {
					ASSERT(pageable_cb.free_list == NULL);
					pageable_cb.free_list = pageable_cb.start;
				}
				for( l = &(pageable_cb.start[0]); l < &(pageable_cb.start[LOCK_PER_PAGE]); l++) {
					l->lo_next = l + 1;
					l->lockname = NULL;
#ifdef DEBUG
					l->dbg_zero = 0;  /* matches 'lockname' field for non-DEBUG locks */
					l->dbg_flags = 0; /* clear debug flags */
#endif
				}
				pageable_cb.start = l;
				(--l)->lo_next = NULL;
				pageable_cb.end_list = l;
				pageable_cb.allocated += LOCK_PER_PAGE;
				pageable_cb.free += LOCK_PER_PAGE;
			}
			else if (pageable_cb.free == 0) {
				simple_unlock(&pageable_cb.pool_lock);
				return;
			}
		}
		/* 
		 * At least one free entry has been found--use it
		 * Note the assumption that the structures for simple and
		 * complex locks are sufficiently alike so that the one for
		 * simple locks can be used for both.
		 */
		((simple_lock_t)lockaddr)->_slockp = pageable_cb.free_list;
		pageable_cb.free_list = (((simple_lock_t)lockaddr)->_slockp)->lo_next;	
		pageable_cb.free--;
#ifdef DEBUG
		assert(!((((simple_lock_t)lockaddr)->_slockp)->dbg_flags & LOCK_IS_ALLOCATED));
#endif
		bzero(((simple_lock_t)lockaddr)->_slockp,sizeof(struct lock_data_instrumented));
		(((simple_lock_t)lockaddr)->_slockp)->lock_control_word.s_lock = SIMPLE_LOCK_AVAIL;
#ifdef DEBUG
		(((simple_lock_t)lockaddr)->_slockp)->dbg_flags |= LOCK_IS_ALLOCATED;
		(((simple_lock_t)lockaddr)->_slockp)->lock_lr = link_register;
		(((simple_lock_t)lockaddr)->_slockp)->lock_caller = myself->t_tid;
		(((simple_lock_t)lockaddr)->_slockp)->lock_cpuid = get_processor_id();
#endif
		simple_unlock(&pageable_cb.pool_lock);
	}

	/* set lock class identifier and the occurrence index inside the class */

	(((simple_lock_t)lockaddr)->_slockp)->lock_id = class;
	(((simple_lock_t)lockaddr)->_slockp)->_occurrence = occurrence;

	/* define this family of locks */
	family_lock_statistics[class].lock_id = class;

	if (flags & LOCK_ALLOC_REINIT) {
	/* 
	 * If this lock is being re-initialized, accumulate and clear its
	 * statistics.  This allows the lock to be logically re-used, without
	 * having to be released and freed.
	 */
	l=((simple_lock_t)lockaddr)->_slockp;
	if (l->acquisitions) {
		fetch_and_add((atomic_p)&family_lock_statistics[l->lock_id].acquisitions,
			l->acquisitions);
		l->acquisitions = 0;
	}
	if (l->misses) {
		fetch_and_add((atomic_p)&family_lock_statistics[l->lock_id].misses, 
			l->misses);
		l->misses = 0;
	}
	if (l->sleeps) {
		fetch_and_add((atomic_p)&family_lock_statistics[l->lock_id].sleeps,
			l->sleeps);
		l->sleeps = 0;
	}
    }

	return;
}


/*
 * NAME:     lock_free_instr
 *
 * FUNCTION: return a lock_data_instrumented structure to the corresponding
 *	     pool
 *          
 */


void
lock_free_instr(void *l)

{
	struct lock_data_instrumented *lockp;
	int 	old_pri;
#ifdef DEBUG
	register int link_register;
	register struct thread *myself = curthread;
	register int lockword;

	GET_RETURN_ADDRESS(link_register);
	if ( *((int *)l) & INSTR_ON) {
		lockp = (((simple_lock_t)l)->_slockp);
		lockword = *((int *)lockp);
	} else {
		lockword = *((int *)l);
	}
	assert(lockword == SIMPLE_LOCK_AVAIL); /* okay for complex locks, too */
#endif

	if ( *((int *)l) & INSTR_ON) {
		lockp = (((simple_lock_t)l)->_slockp);

		fetch_and_add((atomic_p)&family_lock_statistics[lockp->lock_id].acquisitions, lockp->acquisitions);
		fetch_and_add((atomic_p)&family_lock_statistics[lockp->lock_id].misses, lockp->misses);
		fetch_and_add((atomic_p)&family_lock_statistics[lockp->lock_id].sleeps, lockp->sleeps);

		if (lockp < lock_pageable) {
			old_pri = disable_lock (INTMAX, &pinned_cb.pool_lock);
#ifdef DEBUG
			assert(lockp->dbg_flags & LOCK_IS_ALLOCATED);
			lockp->unlock_lr = link_register;
			lockp->unlock_caller = myself->t_tid;
			lockp->unlock_cpuid = get_processor_id();
#endif
			lockp->lo_next = pinned_cb.free_list;
			if (pinned_cb.free_list == NULL) {
				ASSERT(pinned_cb.free == 0);
				pinned_cb.end_list = lockp;
			}
			lockp->lockname = NULL;
			pinned_cb.free_list = lockp;
			pinned_cb.free++;
			ASSERT(pinned_cb.free_list != NULL);
#ifdef DEBUG
			lockp->dbg_flags &= ~LOCK_IS_ALLOCATED;
#else
			/*
			 * by zeroing out the lock word, we effectively un-instrument
			 * the lock--allowing multiple lock frees to be done without
			 * clobbering our control blocks.  This should not be done
			 * with DEBUG defined so as to catch the culprits
			 */
			*(int *)l = 0;
#endif
			unlock_enable(old_pri, &pinned_cb.pool_lock);
		}
		else {
			assert(csa->intpri == INTBASE);
			simple_lock (&pageable_cb.pool_lock);
#ifdef DEBUG
			assert(lockp->dbg_flags & LOCK_IS_ALLOCATED);
			lockp->unlock_lr = link_register;
			lockp->unlock_caller = myself->t_tid;
			lockp->unlock_cpuid = get_processor_id();
#endif
			lockp->lo_next = pageable_cb.free_list;
			if (pageable_cb.free_list == NULL) {
				ASSERT(pageable_cb.free == 0);
				pageable_cb.end_list = lockp;
			}
			lockp->lockname = NULL;
			pageable_cb.free_list = lockp;
			pageable_cb.free++;
			ASSERT(pageable_cb.free_list != NULL);
#ifdef DEBUG
			lockp->dbg_flags &= ~LOCK_IS_ALLOCATED;
#endif
			simple_unlock(&pageable_cb.pool_lock);
#ifndef DEBUG
			/*
			 * by zeroing out the lock word, we effectively un-instrument
			 * the lock--allowing multiple lock frees to be done without
			 * clobbering our control blocks.  This should not be done
			 * with DEBUG defined so as to catch the culprits
			 */
			*(int *)l = 0;
#endif
		}
	}
}

#else /* _INSTRUMENTATION */

#if defined(_KERNSYS)
#ifdef lock_alloc
/* need to undo MACRO definition--if this function changes, so must MACRO */
#undef lock_alloc
#endif

#ifdef lock_free
/* need to undo MACRO definition--if this function changes, so must MACRO */
#undef lock_free
#endif
#endif

/*
 * NAME:     lock_alloc
 *
 * FUNCTION: not instrumented version; lock allocation is not needed.
 *	     just returns
 *
 *
 */

void
lock_alloc(void *lockaddr, int flags, short class, short occurrence)
{
	/* validate flags enforced when INSTRUMENTATION is enabled */
	assert(flags & (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED));
	ASSERT((flags & (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED)) != (LOCK_ALLOC_PIN | LOCK_ALLOC_PAGED));

	/* this is just to enforce the necessary priority level when INSTRUMENTATION is enabled */
	if (flags & LOCK_ALLOC_PAGED)
		assert(csa->intpri == INTBASE);

#ifdef DEBUG
	if ( !(flags & LOCK_ALLOC_REINIT)) {
		/* make sure assert in lock_free doesn't fire unnecessarily */
		((simple_lock_t)lockaddr)->_slock = 0;
	}
#endif
	
	return;
}


/*
 * NAME:     lock_free
 *
 * FUNCTION: not instrumented version; lock allocation is not needed.
 *	     just returns
 *          
 */

void
lock_free(void *l)
{
#ifdef DEBUG
	register int lockword;

	lockword = *((int *)l);
	assert(lockword == SIMPLE_LOCK_AVAIL); /* okay for complex locks, too */
#endif

	/* we cannot enforce priority levels here, unfortunately */
	return;
}

#endif
