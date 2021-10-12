static char sccsid[] = "@(#)31	1.8.1.18  src/bos/kernel/vmm/POWER/vm_map.c, sysvmm, bos412, 9445C412a 10/24/94 07:05:31";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: vm_map_create, vm_map_deallocate, vm_map_enter,
 *	      vm_map_protect, vm_map_remove, vm_map_fork
 *	      vm_map_incore, vm_msleep, vm_mwakeup,
 *	      vm_msem_remove, vm_map_core
 *
 * ORIGINS: 65, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
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
 *	File:	vm/vm_map.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory mapping module.
 */

#ifndef	_AIX

#include <mach_old_vm_copy.h>
#include <mach_ldebug.h>

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <kern/zalloc.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <mach/port.h>
#include <mach/vm_attributes.h>
#include <vm/vm_fault.h>
#if	multimax
#include <confdep.h>
#endif	/* multimax */

#else	/* _AIX */

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/lockl.h>
#include "vmsys.h"
#include "vm_mmap.h"
#include "vm_map.h"
#include <sys/mman.h>
#include <sys/syspest.h>
#include <sys/sleep.h>
#include <sys/shm.h>
#include <sys/core.h>

/*
 * This is an array of event list anchors used by vm_msleep/vm_mwakeup.
 * Each element needs to be initialized to EVENT_NULL.
 */
#define	NMSEMQ		15		/* should be "prime like" */
#define	msemqhash(X)	(&msemq[ ((uint)(X)&0x7fffffff) % NMSEMQ ])
int			msemq[NMSEMQ] = {
 EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL,
 EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL,
 EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL, EVENT_NULL,
};

/*
 * Lock for msemaphore operations.
 */
lock_t	vm_msem_lock	= LOCK_AVAIL;

#endif	/* _AIX */

/*
 *	Virtual memory maps provide for the mapping, protection,
 *	and sharing of virtual memory objects.  In addition,
 *	this module provides for an efficient virtual copy of
 *	memory from one map to another.
 *
 *	Synchronization is required prior to most operations.
 *
 *	Maps consist of an ordered doubly-linked list of simple
 *	entries; a single hint is used to speed up lookups.
 *
 *	In order to properly represent the sharing of virtual
 *	memory regions among maps, the map structure is bi-level.
 *	Top-level ("address") maps refer to regions of sharable
 *	virtual memory.  These regions are implemented as
 *	("sharing") maps, which then refer to the actual virtual
 *	memory objects.  When two address maps "share" memory,
 *	their top-level maps both have references to the same
 *	sharing map.  When memory is virtual-copied from one
 *	address map to another, the references in the sharing
 *	maps are actually copied -- no copying occurs at the
 *	virtual memory object level.
 *
 *	Since portions of maps are specified by start/end addreses,
 *	which may not align with existing map entries, all
 *	routines merely "clip" entries to these start/end values.
 *	[That is, an entry is split into two, bordering at a
 *	start or end value.]  Note that these clippings may not
 *	always be necessary (as the two resulting entries are then
 *	not changed); however, the clipping is done for convenience.
 *	No attempt is currently made to "glue back together" two
 *	abutting entries.
 *
 *	As mentioned above, virtual copy operations are performed
 *	by copying VM object references from one sharing map to
 *	another, and then marking both regions as copy-on-write.
 *	It is important to note that only one writeable reference
 *	to a VM object region exists in any map -- this means that
 *	shadow object creation can be delayed until a write operation
 *	occurs.
 */

#ifndef	_AIX
zone_t		vm_map_zone;		/* zone for vm_map structures */
zone_t		vm_map_entry_zone;	/* zone for vm_map_entry structures */
zone_t		vm_map_kentry_zone;	/* zone for kernel entry structures */
zone_t		vm_map_copy_zone;	/* zone for vm_map_copy structures */
#endif	/* _AIX */

boolean_t	vm_map_lookup_entry();	/* forward declaration */

/*
 *	Placeholder object for submap operations.  This object is dropped
 *	into the range by a call to vm_map_find, and removed when
 *	vm_map_submap creates the submap.
 */

vm_object_t	vm_submap_object;

/*
 *	vm_map_init: REMOVED
 */

/*
 *	vm_map_create:
 *
 *	Creates and returns a new empty VM map with
 *	the given physical map structure, and having
 *	the given lower and upper address bounds.
 */
vm_map_t vm_map_create(pmap, min, max, pageable)
	pmap_t		pmap;
	vm_offset_t	min, max;
	boolean_t	pageable;
{
	register vm_map_t	result;

#ifdef _AIX
	int	savevmm;

	vm_map_lock(dummy);
#endif	/* _AIX */

#ifndef _AIX
	ZALLOC(vm_map_zone, result, vm_map_t);
#else	/* _AIX */
	result = (vm_map_t) vcs_getame();
#endif	/* _AIX */

	if (result == VM_MAP_NULL)

#ifndef _AIX
		panic("vm_map_create: out of maps");
#else	/* _AIX */
		goto mapexit;
#endif	/* _AIX */

	result->vme_next = result->vme_prev = vm_map_to_entry(result);
	result->nentries = 0;
	result->size = 0;
	result->ref_count = 1;

#ifndef _AIX
	result->pmap = pmap;
	result->is_main_map = TRUE;
#endif	/* _AIX */

	result->min_offset = min;
	result->max_offset = max;
	result->entries_pageable = pageable;

#ifndef _AIX
	result->wait_for_space = FALSE;
#endif	/* _AIX */

	result->first_free = vm_map_to_entry(result);
	result->hint = vm_map_to_entry(result);
	vm_map_lock_init(result);
#ifndef _AIX
	simple_lock_init(&result->ref_lock);
	simple_lock_init(&result->hint_lock);
#endif	/* _AIX */

#ifdef _AIX
mapexit:
	vm_map_unlock(dummy);
#endif	/* _AIX */

	return(result);
}

/*
 *	vm_map_entry_create:	[ internal use only ]
 *
 *	Allocates a VM map entry for insertion in the
 *	given map (or map copy).  No fields are filled.
 */

#ifndef _AIX
#define	vm_map_entry_create(map)			\
	_vm_map_entry_create(				\
		(map)->entries_pageable ?		\
		vm_map_entry_zone :			\
		vm_map_kentry_zone)
vm_map_entry_t _vm_map_entry_create(zone)
	zone_t		zone;
#else	/* _AIX */
vm_map_entry_t vm_map_entry_create(map)
	vm_map_t	map;
#endif	/* _AIX */

{
	vm_map_entry_t	entry;

#ifndef _AIX
	ZALLOC(zone, entry, vm_map_entry_t);

	if (entry == VM_MAP_ENTRY_NULL)
		panic("vm_map_entry_create: out of map entries");
#else	/* _AIX */
	entry = (vm_map_entry_t) vcs_getame();
#endif	/* _AIX */

	return(entry);
}

/*
 *	vm_map_entry_dispose:	[ internal use only ]
 *
 *	Inverse of vm_map_entry_create.
 */

#ifndef _AIX
#define	vm_map_entry_dispose(map, entry) \
	_vm_map_entry_dispose( \
		(map)->entries_pageable ? \
			vm_map_entry_zone : \
			vm_map_kentry_zone, \
		entry)
void _vm_map_entry_dispose(zone, entry)
	zone_t		zone;
	vm_map_entry_t	entry;
#else	/* _AIX */
void vm_map_entry_dispose(map, entry)
	vm_map_t	map;
	vm_map_entry_t	entry;
#endif	/* _AIX */

{

#ifndef _AIX
	ZFREE(zone, (vm_offset_t) entry);
#else	/* _AIX */
	vcs_freeame((caddr_t) entry);
#endif	/* _AIX */

}

/*
 *	vm_map_entry_{un,}link:
 *
 *	Insert/remove entries from maps (or map copies).
 */
#define vm_map_entry_link(map, after_where, entry)		\
		MACRO_BEGIN					\
		(map)->nentries++;				\
		(entry)->vme_prev = (after_where);		\
		(entry)->vme_next = (after_where)->vme_next;	\
		(entry)->vme_prev->vme_next =			\
		 (entry)->vme_next->vme_prev = (entry);		\
		MACRO_END
#define vm_map_entry_unlink(map, entry) \
		MACRO_BEGIN \
		(map)->nentries--; \
		(entry)->vme_next->vme_prev = (entry)->vme_prev; \
		(entry)->vme_prev->vme_next = (entry)->vme_next; \
		MACRO_END

#ifndef _AIX
/*
 *	vm_map_reference:
 *
 *	Creates another valid reference to the given map.
 *
 */
void vm_map_reference(map)
	register vm_map_t	map;
{
	if (map == VM_MAP_NULL)
		return;

	simple_lock(&map->ref_lock);
	map->ref_count++;
	simple_unlock(&map->ref_lock);
}
#else	/* _AIX */
/*
 *	vm_map_entry_reference:
 *
 * We use this routine to increment the use counts on objects referenced
 * by the given address map entry.  It is called when a map entry is
 * being duplicated either from clipping or from a fork operation.
 */
void vm_map_entry_reference(entry)
	register vm_map_entry_t	entry;
{
	int msidx, ssid, ssidx, psidx;
	int waslocked, flags, rc;

	/*
	 * Update reference count on mapping SCB.
	 * If first reference, set SCB lookup hint and address.
	 */
	msidx = STOI(entry->mapping_sid);
	ASSERT(scb_valid(msidx) && scb_mseg(msidx));
	scb_refcnt(msidx) += 1;
	if (scb_refcnt(msidx) == 1) {
		scb_ame(msidx) = entry;
		scb_start(msidx) = entry->vme_start;
	}

	/*
	 * Update reference count on source SCB.
	 * Must be done atomically since other processes
	 * may be updating count too.
	 */
	ssidx = STOI(entry->source_sid);
	ASSERT(scb_valid(ssidx));
	fetch_and_add(&scb_refcnt(ssidx), 1);
	if (entry->object != VM_OBJECT_NULL) {
		struct ucred *crp = crref();
		flags = VNOPFLAGS(entry);
		rc = VNOP_MAP(entry->object, entry->vme_start,
			(entry->vme_end - entry->vme_start),
			entry->offset, flags, crp);
		assert(rc == 0);
		crfree(crp);
	}

	/*
	 * Update reference count on paging SCB if any.
	 */
	if (entry->paging_sid != INVLSID) {
		psidx = STOI(entry->paging_sid);
		ASSERT(scb_valid(psidx) && scb_wseg(psidx));
		scb_refcnt(psidx) += 1;
	}
}
#endif	/* _AIX */

/*
 *	vm_map_deallocate:
 *
 *	Removes a reference from the specified map,
 *	destroying it if no references remain.
 *	The map should not be locked.
 */
void vm_map_deallocate(map)
	register vm_map_t	map;
{
	register int		c;

#ifdef _AIX
	int	savevmm;
#endif	/* _AIX */

	if (map == VM_MAP_NULL)
		return;

#ifndef _AIX
	simple_lock(&map->ref_lock);
	c = --map->ref_count;
	simple_unlock(&map->ref_lock);

	if (c > 0) {
		return;
	}
#endif	/* _AIX */

	/*
	 *	Lock the map, to wait out all other references
	 *	to it.
	 */

	vm_map_lock(map);

	(void) vm_map_delete(map, map->min_offset, map->max_offset);

#ifndef _AIX
	pmap_destroy(map->pmap);
#endif	/* _AIX */

#if	MACH_LDEBUG
	vm_map_unlock(map);
#endif

#ifndef _AIX
	ZFREE(vm_map_zone, (vm_offset_t) map);
#else	/* _AIX */
	vcs_freeame((caddr_t) map);
#endif	/* _AIX */

}

/*
 *	vm_map_insert: REMOVED
 */

/*
 *	SAVE_HINT:
 *
 *	Saves the specified entry as the hint for
 *	future lookups.  Performs necessary interlocks.
 */
#ifndef _AIX
#define SAVE_HINT(map,value) \
		simple_lock(&(map)->hint_lock); \
		(map)->hint = (value); \
		simple_unlock(&(map)->hint_lock);
#else	/* _AIX */
#define SAVE_HINT(map,value) \
		(map)->hint = (value);
#endif	/* _AIX */

/*
 *	vm_map_lookup_entry:	[ internal use only ]
 *
 *	Finds the map entry containing (or
 *	immediately preceding) the specified address
 *	in the given map; the entry is returned
 *	in the "entry" parameter.  The boolean
 *	result indicates whether the address is
 *	actually contained in the map.
 */
boolean_t vm_map_lookup_entry(map, address, entry)
	register vm_map_t	map;
	register vm_offset_t	address;
	vm_map_entry_t		*entry;		/* OUT */
{
	register vm_map_entry_t		cur;
	register vm_map_entry_t		last;

	/*
	 *	Start looking either from the head of the
	 *	list, or from the hint.
	 */

#ifndef _AIX
	simple_lock(&map->hint_lock);
#endif	/* _AIX */
	cur = map->hint;
#ifndef _AIX
	simple_unlock(&map->hint_lock);
#endif	/* _AIX */

	if (cur == vm_map_to_entry(map))
		cur = cur->vme_next;

	if (address >= cur->vme_start) {
	    	/*
		 *	Go from hint to end of list.
		 *
		 *	But first, make a quick check to see if
		 *	we are already looking at the entry we
		 *	want (which is usually the case).
		 *	Note also that we don't need to save the hint
		 *	here... it is the same hint (unless we are
		 *	at the header, in which case the hint didn't
		 *	buy us anything anyway).
		 */
		last = vm_map_to_entry(map);
		if ((cur != last) && (cur->vme_end > address)) {
			*entry = cur;
			return(TRUE);
		}
	}
	else {
	    	/*
		 *	Go from start to hint, *inclusively*
		 */
		last = cur->vme_next;
		cur = vm_map_first_entry(map);
	}

	/*
	 *	Search linearly
	 */

	while (cur != last) {
		if (cur->vme_end > address) {
			if (address >= cur->vme_start) {
			    	/*
				 *	Save this lookup for future
				 *	hints, and return
				 */

				*entry = cur;
				SAVE_HINT(map, cur);
				return(TRUE);
			}
			break;
		}
		cur = cur->vme_next;
	}
	*entry = cur->vme_prev;
	SAVE_HINT(map, *entry);
	return(FALSE);
}

/*
 *	vm_map_find: REMOVED
 */

/*
 *	vm_map_space: REMOVED
 */

/*
 *	vm_map_clip_start: MOVED AHEAD OF vm_map_enter
 *
 *	vm_map_clip_start:	[ internal use only ]
 *
 *	Asserts that the given entry begins at or after
 *	the specified address; if necessary,
 *	it splits the entry into two.
 */
#define vm_map_clip_start(map, entry, startaddr) \
	MACRO_BEGIN					\
	if ((startaddr) > (entry)->vme_start) {		\
		vm_map_entry_t clip_entry =		\
			vm_map_entry_create(map);	\
		if (clip_entry == VM_MAP_ENTRY_NULL)	\
			panic("vm_map_clip_start: out of map entries");	\
		_vm_map_clip_start(			\
			(entry),			\
			startaddr,			\
			clip_entry			\
			);				\
		vm_map_entry_link(map,			\
			(entry)->vme_prev,		\
			clip_entry);			\
	}						\
	MACRO_END

/*
 *	This routine is called only when it is known that
 *	the entry must be split.
 */
void _vm_map_clip_start(entry, start, new_entry)
	register vm_map_entry_t	entry;
	register vm_offset_t	start;
	register vm_map_entry_t	new_entry;
{

#ifdef _AIX
	int msidx;
#endif	/* _AIX */

	/*
	 *	Split off the front portion --
	 *	note that we must insert the new
	 *	entry BEFORE this one, so that
	 *	this entry has the specified starting
	 *	address.
	 */

	*new_entry = *entry;

	new_entry->vme_end = start;
	entry->offset += (start - entry->vme_start);
	entry->vme_start = start;

#ifndef _AIX
	if (entry->is_a_map || entry->is_sub_map)
	 	vm_map_reference(new_entry->object.share_map);
	else
		vm_object_reference(new_entry->object.vm_object);
#else	/* _AIX */
	/*
	 * Increment use counts on objects referenced by new entry.
	 */
	vm_map_entry_reference(new_entry);

	/*
	 * Update mapping SCB search hint if needed.
	 * Note that the new entry gets inserted ahead of
	 * the current one so it's the one we want.
	 */
	msidx = STOI(new_entry->mapping_sid);
	ASSERT(scb_valid(msidx) && scb_mseg(msidx));
	if (scb_start(msidx) == new_entry->vme_start) {
		scb_ame(msidx) = new_entry;
	}
#endif	/* _AIX */

}

/*
 *	vm_map_clip_end:	[ internal use only ]
 *
 *	Asserts that the given entry ends at or before
 *	the specified address; if necessary,
 *	it splits the entry into two.
 */
void _vm_map_clip_end();
#define vm_map_clip_end(map, entry, endaddr)		\
	MACRO_BEGIN					\
	if ((endaddr) < (entry)->vme_end) {		\
		vm_map_entry_t clip_entry =		\
			vm_map_entry_create(map);	\
		if (clip_entry == VM_MAP_ENTRY_NULL)	\
			panic("vm_map_clip_end: out of map entries");	\
		_vm_map_clip_end(			\
			(entry),			\
			(endaddr),			\
			clip_entry			\
			);				\
		vm_map_entry_link(map,			\
			(entry),			\
			clip_entry);			\
	}						\
	MACRO_END

/*
 *	This routine is called only when it is known that
 *	the entry must be split.
 */
void _vm_map_clip_end(entry, end, new_entry)
	register vm_map_entry_t	entry;
	register vm_offset_t	end;
	register vm_map_entry_t	new_entry;
{

	/*
	 *	Fill in an entry to be placed
	 *	AFTER the specified entry
	 */

	*new_entry = *entry;

	new_entry->vme_start = entry->vme_end = end;
	new_entry->offset += (end - entry->vme_start);

#ifndef _AIX
	if (entry->is_a_map || entry->is_sub_map)
	 	vm_map_reference(new_entry->object.share_map);
	else
		vm_object_reference(new_entry->object.vm_object);
#else	/* _AIX */
	/*
	 * Increment use counts on objects referenced by new entry.
	 */
	vm_map_entry_reference(new_entry);

	/*
	 * Note that we don't need to update the mapping
	 * SCB search hint since the new entry is inserted
	 * after the current entry.
	 */
#endif	/* _AIX */

}

/*
 *	Routine:	vm_map_enter
 *
 *	Description:
 *		Allocate a range in the specified virtual address map.
 *		The resulting range will refer to memory defined by
 *		the given memory object and offset into that object.
 *
 *		Arguments are as defined in the vm_map call.
 *	Assumptions:
 *		Unlike vm_map_insert, this routine assumes that it
 *		is being called on a main map (not a sharing map).
 */
kern_return_t vm_map_enter(
		map,
		address, size, mask, anywhere,
		object, offset, needs_copy,
		cur_protection, max_protection,	inheritance)
	register
	vm_map_t	map;
	vm_offset_t	*address;	/* IN/OUT */
	vm_size_t	size;
	vm_offset_t	mask;
	boolean_t	anywhere;
	vm_object_t	object;
	vm_offset_t	offset;
	boolean_t	needs_copy;
	vm_prot_t	cur_protection;
	vm_prot_t	max_protection;
	vm_inherit_t	inheritance;
{
	register vm_map_entry_t	entry;
	register vm_offset_t	start;
	register vm_offset_t	end;
	kern_return_t		result = KERN_SUCCESS;

#ifdef _AIX
	vm_offset_t		new_start;
	vm_offset_t		align_mask = mask;
	adspace_t		*adsp;
	unsigned int		srval;
	int			ssid, ssidx, psid, psidx, msid, msidx;
	int			old_msidx, new_msidx;
	int			rc;
	vm_offset_t		clip_start;
	boolean_t		wrapped = FALSE;
	int			flags;
	int			waslocked;
	int			savevmm;
#endif	/* _AIX */

#define RETURN(value)	{ result = value; goto BailOut; }

 StartAgain: ;

	start = *address;

	if (anywhere) {
		vm_map_lock(map);

		/*
		 *	Calculate the first possible address.
		 */

		if (start < map->min_offset)
			start = map->min_offset;
		if (start > map->max_offset)
			RETURN(KERN_NO_SPACE);

		/*
		 *	Look for the first possible address;
		 *	if there's already something at this
		 *	address, we have to start after it.
		 */

#ifndef _AIX
		if (start == map->min_offset) {
			if ((entry = map->first_free) != vm_map_to_entry(map))
				start = entry->vme_end;
#else	/* _AIX */
		/*
		 * Only use free space hint if we are choosing the address.
		 */
		if (*address == VM_OFFSET_NULL) {
			if ((entry = map->first_free) != vm_map_to_entry(map)) {
				*address = entry->vme_end;
			} else {
				/*
				 * No hint, so start at the beginning.
				 */
				*address = map->min_offset;
			}
			start = *address;
#endif	/* _AIX */

		} else {
			vm_map_entry_t	tmp_entry;
			if (vm_map_lookup_entry(map, start, &tmp_entry))
				start = tmp_entry->vme_end;
			entry = tmp_entry;
		}

		/*
		 *	In any case, the "entry" always precedes
		 *	the proposed new region throughout the
		 *	loop:
		 */

		while (TRUE) {
			register vm_map_entry_t	next;

		    	/*
			 *	Find the end of the proposed new region.
			 *	Be sure we didn't go beyond the end, or
			 *	wrap around the address.
			 */

#ifndef _AIX
			start = ((start + mask) & ~mask);
#else	/* _AIX */
			start = ((start + align_mask) & ~align_mask);
#endif	/* _AIX */

			end = start + size;

			if ((end > map->max_offset) || (end < start)) {

#ifndef _AIX
				if (map->wait_for_space) {
					if (size <= (map->max_offset - map->min_offset)) {
						assert_wait((int) map, TRUE);
						vm_map_unlock(map);
						thread_block();
						goto StartAgain;
					}
				}
				
				RETURN(KERN_NO_SPACE);
			}
#else	/* _AIX */
				/*
				 * Continue searching from the beginning.
				 */
				if (wrapped)
					RETURN(KERN_NO_SPACE);

				wrapped = TRUE;
				start = map->min_offset;
				end = start + size;
				if ((end > map->max_offset) || (end < start)) {
					RETURN(KERN_NO_SPACE);
				}
				entry = vm_map_to_entry(map);
			}

			/*
			 * See if we've searched the entire available space
			 * (we've gone beyond the end and then searched from
			 * the beginning back to the original starting point).
			 */
			if (wrapped && start >= *address) {
				if (align_mask) {
					/*
					 * There may be space for the requested
					 * region but we didn't find any in the
					 * first pass because we were aligning
					 * the start address.
					 * Make another pass without performing
					 * any alignment.
					 */
					align_mask = 0;
					wrapped = FALSE;
					goto StartAgain;
				} else {
					/*
					 * No space for the requested region.
					 */
					RETURN(KERN_NO_SPACE);
				}
			}
#endif	/* _AIX */

			/*
			 *	If there are no more entries, we must win.
			 */

			next = entry->vme_next;
			if (next == vm_map_to_entry(map))
				break;

			/*
			 *	If there is another entry, it must be
			 *	after the end of the potential new region.
			 */

			if (next->vme_start >= end)
				break;

			/*
			 *	Didn't fit -- move to the next entry.
			 */

			entry = next;
			start = entry->vme_end;
		}
		*address = start;
	} else {
		vm_map_entry_t		temp_entry;

		/*
		 *	Verify that:
		 *		the address doesn't itself violate
		 *		the mask requirement.
		 */

#ifdef _AIX
		/*
		 * Don't enforce mask when MAP_FIXED is specified.
		 */
#else	/* _AIX */
		if ((start & mask) != 0)
			return(KERN_NO_SPACE);
#endif	/* _AIX */

		vm_map_lock(map);

		/*
		 *	...	the address is within bounds
		 */

		end = start + size;

		if ((start < map->min_offset) ||
		    (end > map->max_offset) ||
		    (start >= end)) {
			RETURN(KERN_INVALID_ADDRESS);
		}

		/*
		 *	...	the starting address isn't allocated
		 */

		if (vm_map_lookup_entry(map, start, &temp_entry))
			RETURN(KERN_NO_SPACE);

		entry = temp_entry;

		/*
		 *	...	the next region doesn't overlap the
		 *		end point.
		 */

		if ((entry->vme_next != vm_map_to_entry(map)) &&
		    (entry->vme_next->vme_start < end))
			RETURN(KERN_NO_SPACE);
	}

	/*
	 *	At this point,
	 *		"start" and "end" should define the endpoints of the
	 *			available new range, and
	 *		"entry" should refer to the region before the new
	 *			range, and
	 *
	 *		the map should be locked.
	 */

#ifdef _AIX
	/*
	 * Since not all uses of the process address space are recorded by
	 * map entries we must verify that the new range is in fact
	 * available in the process address space.  If not available,
	 * the next suggested starting address is returned.
	 */
	new_start = start;
	if (vm_map_findspace(&new_start, end) != 0) {
		if (anywhere) {
			if (wrapped && new_start >= *address) {
				RETURN(KERN_NO_SPACE);
			} else {
				if (new_start > map->max_offset) {
					*address = map->min_offset;
				} else {
					*address = new_start;
				}
				goto StartAgain;
			}
		} else {
			RETURN(KERN_NO_SPACE);
		}
	}

	/*
	 * The new range is available so ensure that the mapping segments
	 * corresponding to this range exist.
	 */
	vm_map_ldseg(start, end);

#endif	/* _AIX */

#ifndef	_AIX
	/*
	 *	See whether we can avoid creating a new entry (and object) by
	 *	extending one of our neighbors.  [So far, we only attempt to
	 *	extend from below.]
	 */

	if ((object == VM_OBJECT_NULL) &&
	    (entry != vm_map_to_entry(map)) &&
	    (entry->vme_end == start) &&
	    (!entry->is_a_map) &&
	    (!entry->is_sub_map) &&
	    (!entry->keep_on_exec) &&
	    (entry->inheritance == inheritance) &&
	    (entry->protection == cur_protection) &&
	    (entry->max_protection == max_protection) &&
	    (entry->wired_count == 0)) {
		if (vm_object_coalesce(entry->object.vm_object,
				VM_OBJECT_NULL,
				entry->offset,
				(vm_offset_t) 0,
				(vm_size_t)(entry->vme_end - entry->vme_start),
				(vm_size_t)(end - entry->vme_end))) {

			/*
			 *	Coalesced the two objects - can extend
			 *	the previous map entry to include the
			 *	new range.
			 */
			map->size += (end - entry->vme_end);
			entry->vme_end = end;
			RETURN(KERN_SUCCESS);
		}
	}
#endif	/* _AIX */

	/*
	 *	Create a new entry
	 */

	/**/ {
	register vm_map_entry_t	new_entry;

	new_entry = vm_map_entry_create(map);

#ifdef _AIX
	if (new_entry == VM_MAP_ENTRY_NULL)
		RETURN(KERN_NO_SPACE);
#endif	/* _AIX */

	new_entry->vme_start = start;
	new_entry->vme_end = end;

#ifndef _AIX
	new_entry->is_a_map = FALSE;
	new_entry->is_sub_map = FALSE;
	new_entry->keep_on_exec = FALSE;
	new_entry->object.vm_object = object;
#else	/* _AIX */
	new_entry->object = object;
#endif	/* _AIX */

	new_entry->offset = offset;

	new_entry->needs_copy =
	new_entry->copy_on_write = needs_copy;

	new_entry->inheritance = inheritance;
	new_entry->protection = cur_protection;
	new_entry->max_protection = max_protection;
	new_entry->wired_count = 0;

#ifdef _AIX
	/*
	 * Set segment ID of source SCB.
	 */
	if (object != VM_OBJECT_NULL) {
		/*
		 * Mapping a file.
		 */
		struct ucred *crp = crref();

		ssid = VTOGP(new_entry->object)->gn_seg; 
		flags = VNOPFLAGS(new_entry);
		rc = VNOP_MAP(new_entry->object, new_entry->vme_start,
			(new_entry->vme_end - new_entry->vme_start),
			new_entry->offset, flags, crp);
		crfree(crp);

		/*
		 * The only reason VNOP_MAP should fail is because
		 * of enforced locking when the file is locked so
		 * we return a protection failure.
		 */
		if (rc) {
			vm_map_entry_dispose(map, new_entry);
			RETURN(KERN_PROTECTION_FAILURE);
		}

		if (ssid == 0 || scb_refcnt(STOI(ssid)) == 0) {
			ssid = VTOGP(new_entry->object)->gn_seg; 
		}
		assert(ssid != 0);
		new_entry->source_sid = ssid;
		ssidx = STOI(ssid);
		ASSERT(scb_valid(ssidx));
		fetch_and_add(&scb_refcnt(ssidx), 1);
	} else {
		/*
		 * Mapping is for an anonymous object so we need
		 * to create the source SCB.
		 */
		rc = vms_create(&ssid, V_WORKING, 0, SEGSIZE, SEGSIZE, 0);
		assert(rc == 0);
		new_entry->source_sid = ssid;
		ssidx = STOI(ssid);
		scb_refcnt(ssidx) = 1;
	}

	/*
	 * Set segment ID of mapping SCB, update reference count on it,
	 * and if first reference or earliest entry set SCB lookup hint
	 * and address.
	 */
	adsp = getadsp();
	srval = as_getsrval(adsp, start);
	msid = SRTOSID(srval);
	new_entry->mapping_sid = msid;
	msidx = STOI(msid);
	ASSERT(scb_valid(msidx) && scb_mseg(msidx));
	scb_refcnt(msidx) += 1;
	if (scb_refcnt(msidx) == 1 ||
	    new_entry->vme_start < scb_start(msidx)) {
		scb_ame(msidx) = new_entry;
		scb_start(msidx) = new_entry->vme_start;
	}

	/*
	 * If this is a private mapping then we need to create
	 * and assign a paging SCB.
	 */
	if (needs_copy) {
		rc = vms_create(&psid, V_WORKING, 0, SEGSIZE, SEGSIZE, 0);
		assert(rc == 0);
		new_entry->paging_sid = psid;
		psidx = STOI(psid);
		scb_refcnt(psidx) = 1;
		/*
		 * The paging SCB is marked to indicate copy-on-write
		 * to provide for inheritance of pages from the source SCB.
		 */
		scb_cow(psidx) = 1;
	} else {
		new_entry->paging_sid = INVLSID;
	}

	/*
	 * When we perform lookups within a paging SCB we
	 * need an offset that is within one segment.
	 * Knowing the original offset that the paging SCB
	 * maps to in the source object allows us to calculate
	 * corresponding offsets within the paging SCB.
	 * (We set this even when no paging SCB exists).
	 */
	new_entry->orig_offset = offset;

	/*
	 * Initialize cross-memory attach count.
	 */
	new_entry->xmattach_count = 0;
#endif	/* _AIX */

	/*
	 *	Insert the new entry into the list
	 */

	vm_map_entry_link(map, entry, new_entry);
	map->size += size;

	/*
	 *	Update the free space hint and the lookup hint
	 */

#ifndef _AIX
	if ((map->first_free == entry) && (entry->vme_end >= new_entry->vme_start))
#else	/* _AIX */
	/*
	 * Use alignment mask when updating free space hint so we
	 * don't waste time looking at free space which we won't use when
	 * mask is applied to start address.
	 */
	if ((map->first_free == entry) &&
		 (((entry->vme_end + align_mask) & ~align_mask)
			 >= new_entry->vme_start))
#endif	/* _AIX */
		map->first_free = new_entry;

	SAVE_HINT(map, new_entry);

#ifdef _AIX
	/*
	 * We must clip any entry which spans multiple segments since
	 * each entry can reference only one mapping segment and also
	 * because paging SCBs can map at most one segment.
	 */
	clip_start = (new_entry->vme_start + SEGSIZE) & ~SOFFSET;
	while(new_entry->vme_end > clip_start) {
		/*
		 * Clip the entry. 'new_entry' points to what remains.
		 */
		vm_map_clip_start(map, new_entry, clip_start);

		/*
		 * Remove reference to old mapping SCB caused by clipping.
		 */
		old_msidx = STOI(new_entry->mapping_sid);
		ASSERT(scb_valid(old_msidx) && scb_mseg(old_msidx));
		scb_refcnt(old_msidx) -= 1;
		
		/*
		 * Set correct mapping SCB for this entry, reference it
		 * and set lookup hint and address.
		 */
		adsp = getadsp();
		srval = as_getsrval(adsp, new_entry->vme_start);
		new_entry->mapping_sid = SRTOSID(srval);
		new_msidx = STOI(new_entry->mapping_sid);
		ASSERT(scb_valid(new_msidx) && scb_mseg(new_msidx));
		scb_refcnt(new_msidx) += 1;
		scb_ame(new_msidx) = new_entry;
		scb_start(new_msidx) = new_entry->vme_start;

		/*
		 * Fix up paging SCB if entry has one.
		 */
		if (new_entry->paging_sid != INVLSID) {
			/*
			 * Remove reference to old paging SCB.
			 */
			psidx = STOI(new_entry->paging_sid);
			ASSERT(scb_valid(psidx) && scb_wseg(psidx));
			scb_refcnt(psidx) -= 1;

			/*
			 * Allocate and assign new paging SCB and initialize
			 * reference count.
			 */
			rc = vms_create(&psid, V_WORKING, 0, SEGSIZE,
					SEGSIZE, 0);
			assert(rc == 0);
			new_entry->paging_sid = psid;
			psidx = STOI(psid);
			scb_refcnt(psidx) = 1;
			/*
			 * The paging SCB is marked to indicate copy-on-write
			 * to provide for inheritance of pages from the source.
			 */
			scb_cow(psidx) = 1;
		}

		/*
		 * Fix up the source SCB for anonymous mappings.
		 */
		if (object == VM_OBJECT_NULL) {
			/*
			 * Remove reference to old source SCB.
			 */
			ssidx = STOI(new_entry->source_sid);
			ASSERT(scb_valid(ssidx) && scb_wseg(ssidx));
			scb_refcnt(ssidx) -= 1;

			/*
			 * Allocate and assign new source SCB and initialize
			 * reference count.
			 */
			rc = vms_create(&ssid, V_WORKING, 0, SEGSIZE,
					SEGSIZE, 0);
			assert(rc == 0);
			new_entry->source_sid = ssid;
			ssidx = STOI(ssid);
			scb_refcnt(ssidx) = 1;

			/*
			 * This entry refers to the new source SCB
			 * just allocated so reset the offset
			 * to zero (it was modified by clipping).
			 */
			new_entry->offset = 0;
		}

		/*
		 * Reset the original offset that the paging SCB
		 * refers to within the source object.
		 * (We set this even when no paging SCB exists).
		 */
		new_entry->orig_offset = new_entry->offset;

		clip_start += SEGSIZE;
	}
#endif	/* _AIX */

	/**/ }

 BailOut: ;

	vm_map_unlock(map);
	return(result);

#undef	RETURN
}

/*
 *	VM_MAP_RANGE_CHECK:	[ internal use only ]
 *
 *	Asserts that the starting and ending region
 *	addresses fall within the valid range of the map.
 */
#define VM_MAP_RANGE_CHECK(map, start, end)		\
		{					\
		if (start < vm_map_min(map))		\
			start = vm_map_min(map);	\
		if (end > vm_map_max(map))		\
			end = vm_map_max(map);		\
		if (start > end)			\
			start = end;			\
		}

/*
 *	vm_map_submap: REMOVED
 */

/*
 *	vm_map_protect:
 *
 *	Sets the protection of the specified address
 *	region in the target map.  If "set_max" is
 *	specified, the maximum protection is to be set;
 *	otherwise, only the current protection is affected.
 */
kern_return_t vm_map_protect(map, start, end, new_prot, set_max)
	register vm_map_t	map;
	register vm_offset_t	start;
	register vm_offset_t	end;
	register vm_prot_t	new_prot;
	register boolean_t	set_max;
{
	register vm_map_entry_t		current;
	vm_map_entry_t			entry;

#ifdef _AIX
	int			msid, ssid, mfirst, pfirst, npages, key;
	adspace_t		*adsp;
	int			savevmm;
#endif	/* _AIX */

	vm_map_lock(map);

	VM_MAP_RANGE_CHECK(map, start, end);

	if (vm_map_lookup_entry(map, start, &entry)) {
		vm_map_clip_start(map, entry, start);
	}
	 else
		entry = entry->vme_next;

	/*
	 *	Make a first pass to check for protection
	 *	violations.
	 */

	current = entry;
	while ((current != vm_map_to_entry(map)) && (current->vme_start < end)) {

#ifndef _AIX
		if (current->is_sub_map) {
			vm_map_unlock(map);
			return(KERN_INVALID_ARGUMENT);
		}
#endif	/* _AIX */

		if ((new_prot & current->max_protection) != new_prot) {
			vm_map_unlock(map);
			return(KERN_PROTECTION_FAILURE);
		}

		current = current->vme_next;
	}

	/*
	 *	Go back and fix up protections.
	 *	[Note that clipping is not necessary the second time.]
	 */

	current = entry;

	while ((current != vm_map_to_entry(map)) && (current->vme_start < end)) {
		vm_prot_t	old_prot;

		vm_map_clip_end(map, current, end);

		old_prot = current->protection;
		if (set_max)
			current->protection =
				(current->max_protection = new_prot) &
					old_prot;
		else
			current->protection = new_prot;

		/*
		 *	Update physical map if necessary.
		 *	Worry about copy-on-write here -- CHECK THIS XXX
		 */

		if (current->protection != old_prot) {

#ifndef _AIX
#define MASK(entry)	((entry)->copy_on_write ? ~VM_PROT_WRITE : \
							VM_PROT_ALL)
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))

			if (current->is_a_map) {
				vm_map_entry_t	share_entry;
				vm_offset_t	share_end;

				vm_map_lock(current->object.share_map);
				(void) vm_map_lookup_entry(
						current->object.share_map,
						current->offset,
						&share_entry);
				share_end = current->offset +
					(current->vme_end - current->vme_start);
				while ((share_entry !=
					vm_map_to_entry(current->object.share_map)) &&
					(share_entry->vme_start < share_end)) {

					pmap_protect(map->pmap,
						(max(share_entry->vme_start,
							current->offset) -
							current->offset +
							current->vme_start),
						min(share_entry->vme_end,
							share_end) -
						current->offset +
						current->vme_start,
						current->protection &
						    MASK(share_entry));

					share_entry = share_entry->vme_next;
				}
				vm_map_unlock(current->object.share_map);
			}
			else
			 	pmap_protect(map->pmap, current->vme_start,
					current->vme_end,
					current->protection & MASK(entry));
#undef	max
#undef	min
#undef	MASK
#else	/* _AIX */
			/*
			 * Change protection key of any pages aliased for
			 * this map entry.
			 */
			msid = current->mapping_sid;
			mfirst = (current->vme_start & SOFFSET) >> L2PSIZE;

			/*
			 * Determine new page protection key.
			 */
			key = VMPROT2PP(new_prot);

			/*
			 * Determine page range.
			 */
			npages = (current->vme_end - current->vme_start)
					>> L2PSIZE;

			/*
			 * Change protection for any pages aliased at
			 * paging SCB.
			 */
			if (current->paging_sid != INVLSID) {
				/*
				 * Starting page number in paging SCB is
				 * calculated based on difference between
				 * current offset and original offset in
				 * source object.
				 */
				ssid = current->paging_sid;
				pfirst = (current->offset
					  - current->orig_offset) >> L2PSIZE;
				ASSERT(pfirst < NUMPAGES);
				vcs_protectap(msid, mfirst, ssid, pfirst,
						npages, key);
			}

			/*
			 * Change protection for any pages aliased at
			 * source SCB.
			 */
			ssid = current->source_sid;
			ASSERT(ssid != 0);
			pfirst = current->offset >> L2PSIZE;
			vcs_protectap(msid, mfirst, ssid, pfirst, npages, key);
#endif	/* _AIX */

		}
		current = current->vme_next;
	}

	vm_map_unlock(map);
	return(KERN_SUCCESS);
}

/*
 *	vm_map_inherit:
 *
 *	Sets the inheritance of the specified address
 *	range in the target map.  Inheritance
 *	affects how the map will be shared with
 *	child maps at the time of vm_map_fork.
 */
kern_return_t vm_map_inherit(map, start, end, new_inheritance)
	register vm_map_t	map;
	register vm_offset_t	start;
	register vm_offset_t	end;
	register vm_inherit_t	new_inheritance;
{
	register vm_map_entry_t	entry;
	vm_map_entry_t	temp_entry;

#ifdef _AIX
	int			savevmm;
#endif	/* _AIX */

	switch (new_inheritance) {
	case VM_INHERIT_NONE:
	case VM_INHERIT_COPY:
	case VM_INHERIT_SHARE:
		break;
	default:
		return(KERN_INVALID_ARGUMENT);
	}

	vm_map_lock(map);

	VM_MAP_RANGE_CHECK(map, start, end);

	if (vm_map_lookup_entry(map, start, &temp_entry)) {
		entry = temp_entry;
		vm_map_clip_start(map, entry, start);
	}
	else
		entry = temp_entry->vme_next;

	while ((entry != vm_map_to_entry(map)) && (entry->vme_start < end)) {
		vm_map_clip_end(map, entry, end);

		entry->inheritance = new_inheritance;

		entry = entry->vme_next;
	}

	vm_map_unlock(map);
	return(KERN_SUCCESS);
}

/*
 *	vm_map_keep_on_exec: REMOVED
 */

/*
 *	vm_map_pageable: REMOVED
 */

/*
 *	vm_map_entry_unwire: REMOVED
 */

/*
 *	vm_map_entry_delete:	[ internal use only ]
 *
 *	Deallocate the given entry from the target map.
 */		
void vm_map_entry_delete(map, entry)
	register vm_map_t	map;
	register vm_map_entry_t	entry;
{
#ifdef _AIX
	vm_map_entry_t	next_entry;
	int msid, msidx, psid, psidx, ssid, ssidx, sreg;
	int mfirst, pfirst, plast, npages;
	adspace_t *adsp;
	int flags;
	int waslocked;
#endif	/* _AIX */

#ifndef _AIX
	if (entry->wired_count != 0)
		vm_map_entry_unwire(map, entry);
#else	/* _AIX */
	/*
	 * Save pointer to next entry before unlinking.
	 */
	next_entry = entry->vme_next;
#endif	/* _AIX */
		
	vm_map_entry_unlink(map, entry);
	map->size -= entry->vme_end - entry->vme_start;

#ifndef _AIX
	if (entry->is_a_map || entry->is_sub_map)
		vm_map_deallocate(entry->object.share_map);
	else
	 	vm_object_deallocate(entry->object.vm_object);
#else	/* _AIX */
	/* Threads synchronization:
	 *     We must remove addressability to the mapping segment
	 * before destroying any of the underlying data structures.
	 */

	/*
	 * Ping-pong any aliased pages of the source back to 
	 * the source SCB.
	 */
	ssid = entry->source_sid;
	msid = entry->mapping_sid;
	ASSERT(ssid != 0);
	mfirst = (entry->vme_start & SOFFSET) >> L2PSIZE;
	pfirst = entry->offset >> L2PSIZE;
	npages = (entry->vme_end - entry->vme_start) >> L2PSIZE;
	plast = pfirst + npages - 1;
	vcs_relalias(msid, mfirst, ssid, pfirst, plast);

	/*
	 * Handle paging SCB if any.
	 */
	if (entry->paging_sid != INVLSID) {
		/*
		 * Decrement reference count on paging SCB.
		 */
		psid = entry->paging_sid;
		psidx = STOI(psid);
		ASSERT(scb_valid(psidx) && scb_wseg(psidx));
		ASSERT(scb_refcnt(psidx) > 0);
		scb_refcnt(psidx) -= 1;

		/*
		 * If reference count is now zero delete the SCB.
		 */
		if (scb_refcnt(psidx) == 0) {
			vms_delete(psid);
		} else {
			/*
			 * Other entries still referencing this SCB.
			 * Release the resources covered by just
			 * this map entry.
			 */
			pfirst = (entry->offset - entry->orig_offset)
					>> L2PSIZE;
			npages = (entry->vme_end - entry->vme_start)
					>> L2PSIZE;
			vm_releasep(psid, pfirst, npages);
		}
	}

	/*
	 * Decrement reference count on mapping SCB.
	 * If count is now zero, remove mapping segment
	 * otherwise see if mapping SCB hint needs to be changed.
	 */
	msidx = STOI(msid);
	ASSERT(scb_valid(msidx) && scb_mseg(msidx));
	ASSERT(scb_refcnt(msidx) > 0);
	scb_refcnt(msidx) -= 1;

	if (scb_refcnt(msidx) == 0) {
		sreg = entry->vme_start >> L2SSIZE;
		rmmapseg(sreg);
	} else {
		if (scb_start(msidx) == entry->vme_start) {
			scb_ame(msidx) = next_entry;
			scb_start(msidx) = next_entry->vme_start;
		}
	}

	/*
	 * Decrement reference count on source SCB.
	 */
	ssidx = STOI(ssid);
	ASSERT(scb_valid(ssidx));
	ASSERT(scb_refcnt(ssidx) > 0);
	fetch_and_add(&scb_refcnt(ssidx), -1);

	/*
	 * If this is a filesystem object issue unmap.
	 * Otherwise this is an anonymous SCB so delete it
	 * if this was the last reference.
	 */
	if (entry->object != VM_OBJECT_NULL) {
		struct ucred *crp = crref();
		flags = VNOPFLAGS(entry);
		VNOP_UNMAP(entry->object, flags, crp);
		crfree(crp);
	} else if (scb_refcnt(ssidx) == 0) {
		/*
		 * Anonymous SCB.
		 */
		ASSERT(scb_wseg(ssidx));
		vms_delete(ssid);
	}
#endif	/* _AIX */

	vm_map_entry_dispose(map, entry);
}

/*
 *	vm_map_delete:	[ internal use only ]
 *
 *	Deallocates the given address range from the target
 *	map.
 *
 *	When called with a sharing map, removes pages from
 *	that region from all physical maps.
 */
kern_return_t vm_map_delete(map, start, end)
	register vm_map_t	map;
	vm_offset_t		start;
	register vm_offset_t	end;
{
	register vm_map_entry_t	entry;
	vm_map_entry_t		first_entry;

	/*
	 *	Find the start of the region, and clip it
	 */

	if (!vm_map_lookup_entry(map, start, &first_entry))
		entry = first_entry->vme_next;
	else {
		entry = first_entry;
		vm_map_clip_start(map, entry, start);

		/*
		 *	Fix the lookup hint now, rather than each
		 *	time though the loop.
		 */

		SAVE_HINT(map, entry->vme_prev);
	}

	/*
	 *	Save the free space hint
	 */

	if (map->first_free->vme_start >= start)
		map->first_free = entry->vme_prev;

	/*
	 *	Step through all entries in this region
	 */

	while ((entry != vm_map_to_entry(map)) && (entry->vme_start < end)) {
		vm_map_entry_t		next;
		register vm_offset_t	s, e;
		register vm_object_t	object;
		extern vm_object_t	kernel_object;

		vm_map_clip_end(map, entry, end);

		next = entry->vme_next;
		s = entry->vme_start;
		e = entry->vme_end;

#ifndef _AIX
		/*
		 *	Unwire before removing addresses from the pmap;
		 *	otherwise, unwiring will put the entries back in
		 *	the pmap.
		 */

		object = entry->object.vm_object;
		if (entry->wired_count != 0)
			vm_map_entry_unwire(map, entry);

		/*
		 *	If this is a sharing map, we must remove
		 *	*all* references to this data, since we can't
		 *	find all of the physical maps which are sharing
		 *	it.
		 */

		if (object == kernel_object)
			vm_object_page_remove(object, entry->offset,
					entry->offset + (e - s));
		 else if (!map->is_main_map)
			vm_object_pmap_remove(object,
					 entry->offset,
					 entry->offset + (e - s));
		 else
			pmap_remove(map->pmap, s, e);
#endif	/* _AIX */

		/*
		 *	Delete the entry (which may delete the object)
		 *	only after removing all pmap entries pointing
		 *	to its pages.  (Otherwise, its page frames may
		 *	be reallocated, and any modify bits will be
		 *	set in the wrong object!)
		 */

		vm_map_entry_delete(map, entry);
		entry = next;
	}

#ifndef _AIX
	if (map->wait_for_space)
		thread_wakeup((int) map);
#endif	/* _AIX */

	return(KERN_SUCCESS);
}

/*
 *	vm_map_remove:
 *
 *	Remove the given address range from the target map.
 *	This is the exported form of vm_map_delete.
 */
kern_return_t vm_map_remove(map, start, end)
	register vm_map_t	map;
	register vm_offset_t	start;
	register vm_offset_t	end;
{
	register kern_return_t	result;

#ifdef _AIX
	int			savevmm;
#endif	/* _AIX */

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);
	result = vm_map_delete(map, start, end);
	vm_map_unlock(map);

	return(result);
}

/*
 *	vm_map_exec: REMOVED
 */

/*
 *	vm_map_check_protection:
 *
 *	Assert that the target map allows the specified
 *	privilege on the entire address region given.
 *	The entire region must be allocated.
 */
boolean_t vm_map_check_protection(map, start, end, protection)
	register vm_map_t	map;
	register vm_offset_t	start;
	register vm_offset_t	end;
	register vm_prot_t	protection;
{
	register vm_map_entry_t	entry;
	vm_map_entry_t		tmp_entry;

#ifdef _AIX
	int			savevmm;
#endif	/* _AIX */

	vm_map_lock(map);

	if (!vm_map_lookup_entry(map, start, &tmp_entry))
		goto failure;
	entry = tmp_entry;

	while (start < end) {
		if (entry == vm_map_to_entry(map))
			goto failure;

		/* No holes allowed! */
		if (start < entry->vme_start)
			goto failure;

		/* Check protection associated with entry. */
		if ((entry->protection & protection) != protection)
			goto failure;

		/* Proceed to next entry */
		start = entry->vme_end;
		entry = entry->vme_next;
	}
	vm_map_unlock(map);
	return(TRUE);
failure:
	vm_map_unlock(map);
	return(FALSE);
}

#if	!MACH_OLD_VM_COPY
/*
 *	vm_map_copy_discard: REMOVED
 */

/*
 *	vm_map_copy_overwrite: REMOVED
 */

/*
 *	vm_map_copy_insert: REMOVED
 */

/*
 *	vm_map_copyout: REMOVED
 */

/*
 *	vm_map_copyin: REMOVED
 */
#else	/* !MACH_OLD_VM_COPY */

extern void vm_map_copy_discard();
extern kern_return_t vm_map_copy_overwrite();
extern kern_return_t vm_map_copyout();
extern kern_return_t vm_map_copyin();

extern kern_return_t vm_map_copy();
extern void vm_map_copy_entry();
#endif	/* !MACH_OLD_VM_COPY */

/*
 *	vm_map_fork:
 *
 *	Create and return a new map based on the old
 *	map, according to the inheritance values on the
 *	regions in that map.
 *
 *	The source map must not be locked.
 */
#ifdef _AIX
/*
 * NOTE:
 *      The calling process must have the process user address space lock.
 */
#endif	/* _AIX */
#ifndef _AIX
vm_map_t vm_map_fork(old_map)
#else	/* _AIX */
vm_map_t vm_map_fork(old_map, uchild)
	struct user	*uchild;
#endif	/* _AIX */

	vm_map_t	old_map;
{
	vm_map_t	new_map;

#ifndef _AIX
	vm_map_t	new_share_map = VM_MAP_NULL;
#else	/* _AIX */
	unsigned int	srval;
	int		fsid, fsidx, msid, ssid, ssidx;
	int		rc, mfirst, pfirst, plast;
	int		savevmm;
#endif	/* _AIX */

	vm_map_entry_t	old_entry;
	vm_map_entry_t	new_entry;

#ifndef _AIX
	pmap_t		new_pmap = pmap_create((vm_size_t) 0);
#else	/* _AIX */
	pmap_t		new_pmap = (pmap_t) 0;
#endif	/* _AIX */

	vm_size_t	new_size = 0;
	vm_size_t	entry_size;

	vm_map_lock(old_map);

	new_map = vm_map_create(new_pmap,
			old_map->min_offset,
			old_map->max_offset,
			old_map->entries_pageable);

#ifdef _AIX
	if (new_map == VM_MAP_NULL)
		goto forkexit;
#endif	/* _AIX */

	for (
	    old_entry = vm_map_first_entry(old_map);
	    old_entry != vm_map_to_entry(old_map);
	    ) {

#ifndef _AIX
		if (old_entry->is_sub_map)
			panic("vm_map_fork: encountered a submap");
#endif	/* _AIX */

		entry_size = (old_entry->vme_end - old_entry->vme_start);

		switch (old_entry->inheritance) {
		case VM_INHERIT_NONE:
			break;

		case VM_INHERIT_SHARE:

#ifndef _AIX
			/*
			 *	If we don't already have a sharing map:
			 */

			if (!old_entry->is_a_map) {
				vm_map_entry_t	new_share_entry;
				
				/*
				 *	Create exactly one new sharing map
				 *	for use by all VM_INHERIT_SHARE
				 *	entries that don't already reference
				 *	sharing maps.  Each entry needs
				 *	a reference to this map (for pointer
				 *	to it); creation returns an initial
				 *	reference, but subsequent passes
				 *	through this code need an explicit
				 *	reference.
				 */

				if (new_share_map == VM_MAP_NULL) {

					new_share_map = vm_map_create(
						    PMAP_NULL,
			    			    old_map->min_offset,
						    old_map->max_offset,
						    old_map->entries_pageable);

			    		new_share_map->is_main_map = FALSE;
				}
				else {
					vm_map_reference(new_share_map);
				}

				/*
				 *	Create a new sharing entry from the
				 *	old task map entry.
				 */

				new_share_entry =
					vm_map_entry_create(new_share_map);
				*new_share_entry = *old_entry;

				/*
				 *	Insert the entry at end of new sharing
				 *	map.  Has to be at end because
				 *	the old map is being scanned forward.
				 */

				vm_map_entry_link(
					new_share_map,
					vm_map_last_entry(new_share_map),
					new_share_entry);

				/*
				 *	Fix up the task map entry to refer
				 *	to the sharing map now.
				 */

				old_entry->is_a_map = TRUE;
				old_entry->object.share_map = new_share_map;
				old_entry->offset = old_entry->vme_start;
			}
#endif	/* _AIX */

			/*
			 *	Clone the entry, referencing the sharing map.
			 */

			new_entry = vm_map_entry_create(new_map);

#ifdef _AIX
			if (new_entry == VM_MAP_ENTRY_NULL)
			{
				vm_map_deallocate(new_map);
				new_map = VM_MAP_NULL;
				goto forkcleanup;
			}
#endif	/* _AIX */

			*new_entry = *old_entry;

#ifndef _AIX
			vm_map_reference(new_entry->object.share_map);
#else	/* _AIX */
			/*
			 * The child's map entry needs to point to its own
			 * mapping SCB not the parent's. Note that this assumes
			 * that the child's address space has already been
			 * initialized correctly with its own mapping SCBs.
			 */
			srval = as_getsrval(&(uchild->U_adspace),
					new_entry->vme_start);
			new_entry->mapping_sid = SRTOSID(srval);

			/*
			 * Now that new entry is referencing the right objects
			 * we can increment use counts on them.
			 */
			vm_map_entry_reference(new_entry);
#endif	/* _AIX */

			/*
			 *	Insert the entry into the new map -- we
			 *	know we're inserting at the end of the new
			 *	map.
			 */

			vm_map_entry_link(
				new_map,
				vm_map_last_entry(new_map),
				new_entry
				);

			/*
			 *	Update the physical map
			 */

#ifndef _AIX
			pmap_copy(new_map->pmap, old_map->pmap,
				new_entry->vme_start,
				entry_size,
				old_entry->vme_start);
#endif	/* _AIX */

			new_size += entry_size;
			break;

		case VM_INHERIT_COPY:

#ifndef _AIX
#if	!MACH_OLD_VM_COPY
			if (!old_entry->is_a_map) {
				boolean_t	src_needs_copy;
				boolean_t	new_entry_needs_copy;

				new_entry = vm_map_entry_create(new_map);
				*new_entry = *old_entry;

				if (vm_object_copy_temporary(
					&new_entry->object.vm_object,
					&new_entry->offset,
					&src_needs_copy,
					&new_entry_needs_copy)) {

					/*
					 *	Handle copy-on-write obligations
					 */

					if (src_needs_copy && !old_entry->needs_copy) {
						vm_object_pmap_protect(
							old_entry->object.vm_object,
							old_entry->offset,
							entry_size,
							old_map->pmap,
							old_entry->vme_start,
							old_entry->protection &
							    ~VM_PROT_WRITE);

						old_entry->copy_on_write =
						 old_entry->needs_copy = TRUE;
					}

					new_entry->copy_on_write =
					 new_entry->needs_copy = new_entry_needs_copy;

					/*
					 *	Insert the entry at the end
					 *	of the map.
					 */

					vm_map_entry_link(new_map,
						vm_map_last_entry(new_map),
						new_entry);


					new_size += entry_size;
					break;
				}

				vm_map_entry_dispose(new_map, new_entry);
			}

			/* INNER BLOCK (copy cannot be optimized) */ {

			vm_offset_t	start = old_entry->vme_start;
			vm_map_copy_t	copy;
			vm_map_entry_t	last = vm_map_last_entry(new_map);

			vm_map_unlock(old_map);
			if (vm_map_copyin(old_map, start, entry_size, FALSE, &copy) 
			    != KERN_SUCCESS) {
			    	vm_map_lock(old_map);
				if (!vm_map_lookup_entry(old_map, start, &last))
					last = last->vme_next;
				old_entry = last;
				/*
				 *	For some error returns, want to
				 *	skip to the next element.
				 */

				continue;
			}

			/*
			 *	Insert the copy into the new map
			 */

			vm_map_copy_insert(new_map, last, copy);
			new_size += entry_size;

			/*
			 *	Pick up the traversal at the end of
			 *	the copied region.
			 */

			vm_map_lock(old_map);
			start += entry_size;
			if (!vm_map_lookup_entry(old_map, start, &last))
				last = last->vme_next;
			 else
				vm_map_clip_start(old_map, last, start);
			old_entry = last;

			continue;
			/* INNER BLOCK (copy cannot be optimized) */ }
#else	/* !MACH_OLD_VM_COPY */
			/*
			 * This could probably be coalesced into the
			 * new code, but it's not worth the effort.
			 */
			/*
			 *	Clone the entry and link into the map.
			 */

			new_entry = vm_map_entry_create(new_map);
			*new_entry = *old_entry;
			new_entry->wired_count = 0;
			new_entry->object.vm_object = VM_OBJECT_NULL;
			new_entry->is_a_map = FALSE;
			vm_map_entry_link(
				new_map,
				vm_map_last_entry(new_map),
				new_entry);
			if (old_entry->is_a_map) {
				kern_return_t	check;

				check = vm_map_copy(new_map,
						old_entry->object.share_map,
						new_entry->vme_start,
						(vm_size_t)(new_entry->vme_end -
							new_entry->vme_start),
						old_entry->offset,
						FALSE, FALSE);
				if (check != KERN_SUCCESS)
					printf("vm_map_fork: copy in share_map region failed\n");
			}
			else {
				vm_map_copy_entry(old_map, new_map, old_entry,
						new_entry);
			}
			break;
#endif	/* !MACH_OLD_VM_COPY */
#else	/* _AIX */
			/*
			 * Clone the entry.
			 */
			new_entry = vm_map_entry_create(new_map);

			if (new_entry == VM_MAP_ENTRY_NULL)
			{
				vm_map_deallocate(new_map);
				new_map = VM_MAP_NULL;
				goto forkcleanup;
			}

			*new_entry = *old_entry;

			/*
			 * The child's map entry needs to point to its own
			 * mapping SCB not the parent's. Note that this assumes
			 * that the child's address space has already been
			 * initialized correctly with its own mapping SCBs.
			 */
			srval = as_getsrval(&(uchild->U_adspace),
					new_entry->vme_start);
			new_entry->mapping_sid = SRTOSID(srval);


			/* XXX - for lazy evaluation we can just set needs_copy
			 *	 here rather than all the following code and
			 *	 do vm_forkcopy at page-fault time in lookup
			 *	 when needed (that is, if we can make it work!)
			 * lazy evaluation of forked private object
			new_entry->needs_copy = TRUE;
			 */

			/*
			 * Now we need to fork the paging SCB that this entry
			 * refers to. Multiple entries may refer to the same
			 * paging SCB but we only want to fork it once.
			 * We store the sibling sid in the paging scb
			 * to know if we've already forked this paging SCB.
			 */
			fsid = old_entry->paging_sid;
			ASSERT(fsid != INVLSID);
			fsidx = STOI(fsid);
			ASSERT(scb_valid(fsidx) && scb_wseg(fsidx));

			ssid = scb_sibling(fsidx);
			if (ssid)
			{
				/*
				 * Paging SCB already forked.
				 * Set correct paging SCB for new entry
				 * vm_map_entry_reference() will update
				 * its reference count.
				 */
				ASSERT(scb_valid(STOI(ssid)) && scb_wseg(STOI(ssid)));
				new_entry->paging_sid = ssid;
			} else {
				/*
				 * Before we fork the paging SCB we ping-pong 
				 * any aliased pages back to their source
				 * addresses. We must do this since the fork
				 * code transfers pages common between the
				 * forker and sibling to the dummy parent SCB.
				 */
				msid = old_entry->mapping_sid;
				mfirst = (old_entry->vme_start & SOFFSET)
						>> L2PSIZE;

				/*
				 * Ping-pong aliased pages in [0, maxvpn]
				 */
				pfirst = 0;
				plast = scb_maxvpn(fsidx);
				if (plast >= pfirst) {
					vcs_relalias(msid, mfirst, fsid,
							pfirst, plast);
				}

				/*
				 * Ping-pong aliased pages in [minvpn, MAXVPN]
				 */
				pfirst = scb_minvpn(fsidx);
				plast = MAXVPN;
				if (plast >= pfirst) {
					vcs_relalias(msid, mfirst, fsid,
							pfirst, plast);
				}

				/* 
				 * Create copy-on-reference binary tree of dummy
				 * parent, forker (fsid) and sibling (ssid)
				 * Set correct paging SCB for new entry.
				 */
				if ((rc = vm_forkcopy(fsid, &ssid)) != 0)
				{
					/* If forkcopy fails throw out
					 * entire map for forked process.
					 * Have to increment counts on
					 * objects referenced by new entry
					 * so delete works right.
					 */
					vm_map_entry_reference(new_entry);
					vm_map_deallocate(new_map);
					new_map = VM_MAP_NULL;
					goto forkcleanup;
				}

				new_entry->paging_sid = ssid;

				/* 
				 * save sibling sid
				 */
				scb_sibling(fsidx) = ssid;
			}

			/*
			 * Now that new entry is referencing the right objects
			 * we can increment use counts on them.
			 */
			vm_map_entry_reference(new_entry);
				
			/*
			 *	Insert the entry into the new map -- we
			 *	know we're inserting at the end of the new
			 *	map.
			 */

			vm_map_entry_link(
				new_map,
				vm_map_last_entry(new_map),
				new_entry
				);

			new_size += entry_size;
			break;
#endif	/* _AIX */

		}
		old_entry = old_entry->vme_next;
	}

	new_map->size = new_size;

#ifdef _AIX
        /*
         * Now, we need to reset the scb_sibling field on all
         * paging scbs to zero so that successive forks
         * will behave properly.
         */
forkcleanup:
        for (
            old_entry = vm_map_first_entry(old_map);
            old_entry != vm_map_to_entry(old_map);
            ) {
                fsid = old_entry->paging_sid;
                if (fsid && fsid != INVLSID) {
                	fsidx = STOI(fsid);
			ASSERT(scb_valid(fsidx) && scb_wseg(fsidx));
                        scb_sibling(fsidx) = 0;
                }

                old_entry = old_entry->vme_next;
        }

forkexit:
#endif	/* _AIX */

	vm_map_unlock(old_map);

	return(new_map);
}

/*
 *	vm_map_lookup: MOVED TO v_map.c (MUST BE PINNED)
 */

/*
 * Remainder of OSF vm_map.c REMOVED...
 */

#ifdef _AIX
/*
 *	vm_map_sync
 *
 *	Synchronized the data in a mapped region and file by pushing
 *	modified pages back to the object and/or by invalidating pages.
 */
kern_return_t vm_map_sync(map, start, end, flags)
vm_map_t	map;
vm_offset_t	start;
vm_offset_t	end;
int flags;
{
	int		invalidate, wait;
	vm_map_entry_t	entry;
	vm_map_entry_t	tmp_entry;
	int		ssid, ssidx, pfirst, xend, npages, rc;
	kern_return_t	result;
	int		savevmm;

	vm_map_lock(map);

	invalidate = ((flags & MS_INVALIDATE) == MS_INVALIDATE);
	wait = ((flags & MS_SYNC) == MS_SYNC);
	
	if (!vm_map_lookup_entry(map, start, &tmp_entry)) {
		vm_map_unlock(map);
		return(KERN_INVALID_ADDRESS);
	}
	entry = tmp_entry;

	while (start < end) {
		/*
		 * No holes allowed!
		 */
		if ((entry == vm_map_to_entry(map)) ||
		    (start < entry->vme_start)) {
			vm_map_unlock(map);
			return(KERN_INVALID_ADDRESS);
		}

		/*
		 * Determine source SCB and starting page in source.
		 */
		if (entry->inheritance == VM_INHERIT_COPY) {
			/*
			 * Private mapping -- AES says this should fail.
			 */
			vm_map_unlock(map);
			return(KERN_INVALID_ARGUMENT);
		} else {
			/*
			 * Not a private mapping.
			 */
			ssid = entry->source_sid;
			ASSERT(ssid != 0);
			ssidx = STOI(ssid);
			ASSERT(scb_valid(ssidx));

			pfirst = (start - entry->vme_start + entry->offset)
					>> L2PSIZE;
		}

		/*
		 * Determine number of pages in source SCB.
		 */
		xend = (end < entry->vme_end) ? end : entry->vme_end;
		npages = (xend - start) >> L2PSIZE;

		/*
		 * This is like an fsync.  Any modified pages
		 * in the range get written to their home locations.
		 */
		rc = vm_writep(ssid, pfirst, npages);
		ASSERT (rc == 0);

		/*
		 * Invalidate.  For working storage segments all pages
		 * in memory are released, all paging space blocks are
		 * freed and the page states are set to logically zero.
		 * For files, all pages in memory are released except
		 * when the disk block is uninitialized in which case
		 * the page is zeroed and kept.  For deferred-update
		 * files, any extension memory disk block is freed.
		 */
		if (invalidate) {
			if (scb_wseg(ssidx)) {
				rc = vm_releasep(ssid, pfirst, npages);
			} else {
				rc = vm_invalidatep(ssid, pfirst, npages);
			}
			/*
			 * For working storage, vm_releasep() may fail
			 * with EINVAL if pinned pages are encountered
			 * We don't currently support pinning of mmap()
			 * regions but when we do, and vm_releasep() fails
			 * due to a pinned page, we should return an
			 * error such that msync() returns EBUSY.
			 */
			ASSERT (rc == 0);
		}

		/*
		 * If synchronous, wait for I/O on SCB to complete
		 * (except for working storage segments for which
		 * waiting on an I/O level isn't supported).
		 */
		if (wait && !scb_wseg(ssidx)) {
			if ((result = vms_iowait(ssid)) != 0) {
				/*
				 * I/O error.
				 */
				return(KERN_MEMORY_ERROR);
			}
		}

		/*
		 * Proceed to next entry.
		 */
		start = entry->vme_end;
		entry = entry->vme_next;
	}

	vm_map_unlock(map);

	return(KERN_SUCCESS);
}

/*
 *	vm_map_incore
 *
 *	Determine if an address is within a map and if it is in memory.
 */
kern_return_t vm_map_incore(map, addr, incore)
vm_map_t	map;
vm_offset_t	addr;
char		*incore;
{
	vm_map_entry_t	entry;
	int		savevmm;
	kern_return_t	exitrc;
	int		pgoff, pno;

	vm_map_lock(map);

	/*
	 * Ensure address is contained within a map entry.
	 */
	if (vm_map_lookup_entry(map, addr, &entry)) {
		if (entry->paging_sid != INVLSID) {
			/*
			 * Paging SCB exists, see if private page is in memory.
			 */
			pgoff = (addr - entry->vme_start)
				+ (entry->offset - entry->orig_offset);
			pno = pgoff >> L2PSIZE;
			if (v_lookup(entry->paging_sid, pno) >= 0) {
				*incore = 1;
			} else {
				*incore = 0;
			}
		} else {
			/*
			 * See if source address is in memory.
			 */
			pgoff = (addr - entry->vme_start) + entry->offset;
			pno = pgoff >> L2PSIZE;
			if (v_lookup(entry->source_sid, pno) >= 0) {
				*incore = 1;
			} else {
				*incore = 0;
			}
		}
		exitrc = KERN_SUCCESS;
	} else {
		exitrc = KERN_INVALID_ADDRESS;
	}

	vm_map_unlock(map);
	return(exitrc);
}

/*
 * vm_msleep
 *
 * Called to wait for a time when the semaphore might be free.  This routine
 * locks a global semaphore lock, sets the wanted flag, checks the state of
 * the semaphore and if it is not free sleeps.
 */
kern_return_t vm_msleep(map, usem)
vm_map_t	map;
volatile msemaphore	*usem;
{
	volatile msemaphore sem;
	kern_return_t	result;
	vm_map_entry_t	entry;
	int		rc, sidx, sreg;
	struct segstate *sp;
	adspace_t	*adsp;
	unsigned int	srval;
	int		savevmm;

	rc = lockl(&vm_msem_lock, LOCK_SHORT); /* Lock global semaphore lock. */
	ASSERT(rc == LOCK_SUCC);

	/* check if the semaphore has been removed. No point in putting
	 * this thread to sleep if the semaphore has been removed. 
	 * And if we don't make this check then the thread may go to
  	 * sleep and never get wakened up
	 */
        if (copyin((caddr_t) &usem->msem_wanted, (caddr_t) &sem.msem_wanted,
                    sizeof(sem.msem_wanted)))
        {
                unlockl(&vm_msem_lock);
                return(KERN_INVALID_ADDRESS);
        }
        if (sem.msem_wanted == -1)
        {
                unlockl(&vm_msem_lock);
                return(KERN_INVALID_ADDRESS);
        }

	sem.msem_wanted = TRUE;	/* Set wanted flag to true. */
	if (copyout((caddr_t) &sem.msem_wanted, (caddr_t) &usem->msem_wanted,
		    sizeof(sem.msem_wanted)))
	{
		unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}

				/* Need to synchronzie caches here. */

				/* Check the state of the semaphore */
	if (copyin((caddr_t) &usem->msem_state, (caddr_t) &sem.msem_state,
		    sizeof(sem.msem_state)))	
	{
		unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}
	
	if (sem.msem_state == 0)
	{
		unlockl(&vm_msem_lock);
		return(KERN_SUCCESS);
	}
	
	/* Find the object backing the semaphore. */

	vm_map_lock(map);

	if (map != VM_MAP_NULL &&
	    vm_map_lookup_entry(map, (vm_offset_t) usem, &entry)) {

		/* Wait for the semaphore to free up. */

		/*
		 * We use sidx to determine what event list to use.
		 * Note that this means that ALL semaphores in the
		 * same object end up on the same event list (and
		 * thus all get wakened up together). This is the
		 * same behavior as in the OSF code.
		 */
		sidx = STOI(entry->source_sid);
		
		rc = e_sleepl(&vm_msem_lock, msemqhash(sidx), EVENT_SIGRET);

		if (rc == EVENT_SIG) {
			/*
			 * Indicate sleep interrupted by a signal.
			 */
			result = KERN_INTERRUPTED;
		} else {
			result = KERN_SUCCESS;
		}
	} else {
		/*
		 * The region isn't an mmap'd region, however we allow
		 * semaphores in shmat'd regions also so see if it is one.
		 */
		sreg = (uint_t) usem >> SEGSHIFT;
		sp = &U.U_segst[sreg];

		if (sp->segflag & SEG_SHARED || sp->segflag & SEG_MAPPED) {
			/*
			 * It is a shmat'd region.
			 */
			adsp = getadsp();
			srval = as_getsrval(adsp, usem);
			sidx = STOI(SRTOSID(srval));
			
			rc = e_sleepl(&vm_msem_lock, msemqhash(sidx),
				      EVENT_SIGRET);

			if (rc == EVENT_SIG) {
				/*
				 * Indicate sleep interrupted by a signal.
				 */
				result = KERN_INTERRUPTED;
			} else {
				result = KERN_SUCCESS;
			}
		} else {
			result = KERN_INVALID_ADDRESS;
		}
	}

	vm_map_unlock(map);
	unlockl(&vm_msem_lock);
	return(result);
}

/*
 * vm_mwakeup
 *
 * Called to wake up any processes that might be waiting for a semaphore.
 * This routine locks a global semaphore lock, checks to see if the
 * semaphore has been deleted and wakes up any processes waiting on
 * the semaphore.
 * The return code lock_ret from lockl() indicates whether the lock,
 * vm_msem_lock, has already been obtained. If lock_ret equals to 
 * LOCK_SUCC, vm_mwakeup was called by mwakeup() and vm_msem_lock was 
 * not locked. If lock_ret equals to LOCK_NEST, vm_mwakeup was called 
 * by vm_msem_remove() and vm_msem_lock was locked by vm_msem_remove() 
 * already. 
 */
kern_return_t vm_mwakeup(map, usem)
vm_map_t	map;
volatile msemaphore	*usem;
{
	volatile msemaphore	sem;
	kern_return_t	result;
	vm_map_entry_t	entry;
	int		lock_ret, sidx, sreg;
	struct segstate *sp;
	adspace_t	*adsp;
	unsigned int	srval;
	int		savevmm;

	/* Lock global semaphore lock. */
	lock_ret = lockl(&vm_msem_lock, LOCK_SHORT); 
	ASSERT(lock_ret == LOCK_SUCC || lock_ret == LOCK_NEST);

	if (copyin((caddr_t) &usem->msem_wanted, 
		(caddr_t) &sem.msem_wanted, sizeof(sem.msem_wanted)))
	{
		if (lock_ret != LOCK_NEST)
			unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}
	
	/*
	 * Check we are not waking up sleepers when the semaphore has
	 * been deleted
	 */
	if (sem.msem_wanted != -1) {
		sem.msem_wanted = FALSE;	/* Set wanted flag to false. */
		if (copyout((caddr_t) &sem.msem_wanted,
			    (caddr_t) &usem->msem_wanted,
			     sizeof(sem.msem_wanted)))
		{
			if (lock_ret != LOCK_NEST)
				unlockl(&vm_msem_lock);
			return(KERN_INVALID_ADDRESS);
		}
	}

	/* Find the object backing the semaphore. */

	vm_map_lock(map);

	if (map != VM_MAP_NULL &&
	    vm_map_lookup_entry(map, (vm_offset_t) usem, &entry)) {

		/* Wake up everyone waiting on the semaphore. */

		sidx = STOI(entry->source_sid);
		
		/*
		 * Use e_wakeupx to avoid dispatching one of the
		 * waiting processes while we still hold the msemaphore
		 * lock which it would need.
		 */
		e_wakeupx(msemqhash(sidx), E_WKX_NO_PREEMPT);

		result = KERN_SUCCESS;
	} else {
		/*
		 * The region isn't an mmap'd region, however we allow
		 * semaphores in shmat'd regions also so see if it is one.
		 */
		sreg = (uint_t) usem >> SEGSHIFT;
		sp = &U.U_segst[sreg];

		if (sp->segflag & SEG_SHARED || sp->segflag & SEG_MAPPED) {
			/*
			 * It is a shmat'd region.
			 */
			adsp = getadsp();
			srval = as_getsrval(adsp, usem);
			sidx = STOI(SRTOSID(srval));
			
			e_wakeupx(msemqhash(sidx), E_WKX_NO_PREEMPT);

			result = KERN_SUCCESS;
		} else {
			result = KERN_INVALID_ADDRESS;
		}
	}

	vm_map_unlock(map);
	if (lock_ret != LOCK_NEST)
		unlockl(&vm_msem_lock);
	return(result);
}
/*
 * vm_msem_remove
 *
 * Removes a semaphore.
 * If any processes are waiting on this semaphore, they are
 * woken up. Calls vm_wakeupx with "vm_msem_lock" locked.
 * 
 */
int 
vm_msem_remove(map, usem)
vm_map_t map; 
volatile msemaphore *usem;
{
	volatile msemaphore sem;
	int	wanted, rc;

        rc = lockl(&vm_msem_lock, LOCK_SHORT); /* Lock global semaphore lock. */
        ASSERT(rc == LOCK_SUCC);

        if (copyin((caddr_t) &usem->msem_wanted, (caddr_t) &sem.msem_wanted,
                    sizeof(sem.msem_wanted)))
        {
                unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
        }

	if (sem.msem_wanted == -1) {
                unlockl(&vm_msem_lock);
		return(KERN_INVALID_ARGUMENT);
	}
	/*
	 * The wanted count is also used to show if the semaphore is removed.
	 * Save away whether there are waiters. We must mark the semaphore
	 * as removed before we wake up the waiters in case their priority is
	 * higher than ours
	 */
	wanted = sem.msem_wanted;
	sem.msem_wanted = -1;	
	if (copyout((caddr_t) &sem.msem_wanted, (caddr_t) &usem->msem_wanted,
                             sizeof(sem.msem_wanted)))
	{
		unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}

	if (wanted > 0)
		rc = vm_mwakeup(map, usem);

	sem.msem_state = -1;

	if (copyout((caddr_t) &sem.msem_state, (caddr_t) &usem->msem_state,
                             sizeof(sem.msem_state)))
	{
		unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}
	
	if (rc)
	{	
		unlockl(&vm_msem_lock);
		return(KERN_INVALID_ADDRESS);
	}	
	
	unlockl(&vm_msem_lock);
	return(0);
}

/*
 * vm_map_core
 *
 * Called to return the list of anonymous mmap regions so they may be
 * written out as part of core() processing.  This is done to provide
 * the thread stacks as part of the core image (under the assumption
 * that these stacks are created as anonymous mmap regions).  We don't
 * perform any special checks to limit the set of regions to just those
 * being used for thread stacks or even just those that are writable.
 * This routine is initially called with the input argument equal to NULL
 * in order to determine the total number of regions.  In the subsequent
 * call we fill out the appropriate fields in the vm_info array which
 * has been allocated to the necessary size.
 */
int
vm_map_core(vm_map_t map, struct vm_info *vmcore)
{
	vm_map_entry_t	entry;
	int		anon_cnt = 0;
	int		savevmm;

	vm_map_lock(map);

	for (entry  = vm_map_first_entry(map);
	     entry != vm_map_to_entry(map);
	     entry  = entry->vme_next) {
		/*
		 * Keep a count of each anonymous map entry and fill
		 * out a vm_info structure for each one if the input
		 * pointer is non-NULL.
		 */
		if (entry->object == VM_OBJECT_NULL) {
			anon_cnt++;
			if (vmcore != NULL) {
				vmcore->vminfo_addr = (void *)entry->vme_start;
				vmcore->vminfo_size = entry->vme_end -
							entry->vme_start;
				vmcore++;
			}
		}

	}

	vm_map_unlock(map);
	return(anon_cnt);
}
#endif	/* _AIX */
