/* @(#)33       1.5  src/bos/kernel/vmm/POWER/vm_map.h, sysvmm, bos411, 9428A410j 2/24/94 11:20:05 */
#ifndef	_H_VM_MAP
#define _H_VM_MAP

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: vm_map.h
 *
 * ORIGINS: 65 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_map.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory map module definitions.
 *
 * Contributors:
 *	avie, dlb, mwyoung
 */

#ifndef _AIX

#include <mach_old_vm_copy.h>

#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/pmap.h>
#include <vm/vm_object.h>
#include <kern/lock.h>
#include <kern/macro_help.h>

#endif	/* _AIX */

/*
 *	Types defined:
 *
 *	vm_map_t		the high-level address map data structure.
 *	vm_map_entry_t		an entry in an address map.
 *	vm_map_version_t	a timestamp of a map, for use with vm_map_lookup
 *	vm_map_copy_t		represents memory copied from an address map,
 *				 used for inter-map copy operations
 */

#ifndef _AIX
/*
 *	Type:		vm_map_object_t [internal use only]
 *
 *	Description:
 *		The target of an address mapping, either a virtual
 *		memory object or a sharing map that denotes sharing
 *		with other address maps.
 */
typedef union vm_map_object {
	struct vm_object	*vm_object;	/* object object */
	struct vm_map		*share_map;	/* share map */
	struct vm_map		*sub_map;	/* belongs to another map */
} vm_map_object_t;
#endif	/* _AIX */

/*
 *	Type:		vm_map_entry_t [internal use only]
 *
 *	Description:
 *		A single mapping within an address map.
 *
 *	Implementation:
 *		Address map entries consist of start and end addresses,
 *		a VM object (or sharing map) and offset into that object,
 *		and user-exported inheritance and protection information.
 *		Control information for virtual copy operations is also
 *		stored in the address map entry.
 */
struct vm_map_links {
	struct vm_map_entry	*prev;		/* previous entry */
	struct vm_map_entry	*next;		/* next entry */
	vm_offset_t		start;		/* start address */
	vm_offset_t		end;		/* end address */
};

#ifdef _AIX
/* NOTE:
 * sizeof(vm_map_entry) cannot exceed sizeof(struct ame) defined in vmscb.h.
 */
#endif	/* _AIX */

struct vm_map_entry {
	struct vm_map_links	links;		/* links to other entries */
#define vme_prev		links.prev
#define vme_next		links.next
#define vme_start		links.start
#define vme_end			links.end

#ifndef _AIX
	union vm_map_object	object;		/* object I point to */
#else	/* _AIX */
	vm_object_t	 	object;		/* object I point to */
						/* (pointer to vnode) */
#endif	/* _AIX */

	vm_offset_t		offset;		/* offset into object */
	unsigned int

#ifndef _AIX
	/* boolean_t */		is_a_map:1,	/* Is "object" a map? */
	/* boolean_t */		is_sub_map:1,	/* Is "object" a submap? */
	/* boolean_t */		keep_on_exec: 1, /* preserve mapping on exec */
#endif	/* _AIX */

		/* Only used when object is a vm_object: */
	/* boolean_t */		copy_on_write:1,/* is data copy-on-write */
	/* boolean_t */		needs_copy:1;	/* does object need to be copied */
		/* Only in task maps: */
	vm_prot_t		protection;	/* protection code */
	vm_prot_t		max_protection;	/* maximum protection */
	vm_inherit_t		inheritance;	/* inheritance */
	int			wired_count;	/* can be paged if = 0 */

#ifdef _AIX
	int			source_sid;	/* segment ID of source SCB  */
	int			mapping_sid;	/* segment ID of mapping SCB */
	int			paging_sid;	/* segment ID of paging SCB  */
	vm_offset_t		orig_offset;	/* original offset in object */
	unsigned int		xmattach_count; /* cross-memory attach count */
#endif	/* _AIX */

};

typedef struct vm_map_entry	*vm_map_entry_t;

#define VM_MAP_ENTRY_NULL	((vm_map_entry_t) 0)

/*
 *	Type:		vm_map_t [exported; contents invisible]
 *
 *	Description:
 *		An address map -- a directory relating valid
 *		regions of a task's address space to the corresponding
 *		virtual memory objects.
 *
 *	Implementation:
 *		Maps are doubly-linked lists of map entries, sorted
 *		by address.  One hint is used to start
 *		searches again from the last successful search,
 *		insertion, or removal.  Another hint is used to
 *		quickly find free space.
 *
 *		The same address map structure is also used to
 *		represent the contents of memory regions shared
 *		by more than one task.  When used this way, an
 *		address map is called a "sharing map".
 */

#ifdef _AIX
/* NOTE: sizeof(vm_map) cannot exceed sizeof(struct ame) defined in vmscb.h.
 */
#endif	/* _AIX */

typedef struct vm_map {

#ifndef _AIX
	lock_data_t		lock;		/* Lock for map data */
#endif	/* _AIX */

	struct vm_map_links	links;		/* links to the entries */
#define min_offset		links.start	/* start of range */
#define max_offset		links.end	/* end of range */
	int			nentries;	/* Number of entries */

#ifndef _AIX
	pmap_t			pmap;		/* Physical map */
#endif	/* _AIX */

	vm_size_t		size;		/* virtual size */

#ifndef _AIX
	boolean_t		is_main_map;	/* Am I a main map? */
#endif	/* _AIX */

	int			ref_count;	/* Reference count */
	decl_simple_lock_data(,	ref_lock)	/* Lock for ref_count field */
	vm_map_entry_t		hint;		/* hint for quick lookups */
	decl_simple_lock_data(,	hint_lock)	/* lock for hint storage */
	vm_map_entry_t		first_free;	/* First free space hint */
	boolean_t		entries_pageable; /* map entries pageable?? */

#ifndef _AIX
	boolean_t		wait_for_space;	/* Should callers wait for space? */
	unsigned int		timestamp;	/* Version number */
#endif	/* _AIX */

} *vm_map_t;

#define		VM_MAP_NULL	((vm_map_t) 0)

#define vm_map_to_entry(map)	((struct vm_map_entry *) &(map)->links)
#define vm_map_first_entry(map)	((map)->links.next)
#define vm_map_last_entry(map)	((map)->links.prev)

#ifndef _AIX
/*
 *	Type:		vm_map_version_t [exported; contents invisible]
 *
 *	Description:
 *		Map versions may be used to quickly validate a previous
 *		lookup operation.
 *
 *	Usage note:
 *		Because they are bulky objects, map versions are usually
 *		passed by reference.
 *
 *	Implementation:
 *		Since lookup operations may involve both a main map and
 *		a sharing map, it is necessary to have a timestamp from each.
 *		[If the main map timestamp has changed, the share_map and
 *		associated timestamp are no longer valid; therefore, the
 *		map version does not include a reference for the embedded
 *		share_map.]
 */
typedef struct {
	unsigned int	main_timestamp;
	vm_map_t	share_map;
	unsigned int	share_timestamp;
} vm_map_version_t;

/*
 *	Type:		vm_map_copy_t [exported; contents invisible]
 *
 *	Description:
 *		A map copy object represents a region of virtual memory
 *		that has been copied from an address map but is still
 *		in transit.
 *
 *		A map copy object may only be used by a single thread
 *		at a time.
 *
 *	Implementation:
 *		The map copy object is very similar to the main
 *		address map in structure, and as a result, some
 *		of the internal maintenance functions/macros can
 *		be used with either address maps or map copy objects.
 *
 *		The map copy object contains a header links
 *		entry onto which the other entries that represent
 *		the region are chained.  The endpoints in the
 *		header entry are used to record the original page
 *		alignment of the source region.
 */
typedef struct vm_map_copy {
#if	!MACH_OLD_VM_COPY
	struct vm_map_links	links;
	unsigned int		nentries;
	boolean_t		entries_pageable;
#else	/* !MACH_OLD_VM_COPY */
	vm_map_t map; vm_offset_t addr; vm_size_t size;
#endif	/* !MACH_OLD_VM_COPY */
} *vm_map_copy_t;

#define	VM_MAP_COPY_NULL	((vm_map_copy_t) 0)

#define vm_map_copy_to_entry(map)		\
		((struct vm_map_entry *) &(map)->links)
#define vm_map_copy_first_entry(map)		\
		((map)->links.next)
#define vm_map_copy_last_entry(map)		\
		((map)->links.prev)

/*
 *	Macros:		vm_map_lock, etc. [internal use only]
 *	Description:
 *		Perform locking on the data portion of a map.
 */

#define vm_map_lock_init(map)			\
MACRO_BEGIN					\
	lock_init(&(map)->lock, TRUE);		\
	(map)->timestamp = 0;			\
MACRO_END

#define vm_map_lock(map)			\
MACRO_BEGIN					\
	lock_write(&(map)->lock);		\
	(map)->timestamp++;			\
MACRO_END

#define vm_map_unlock(map)	lock_write_done(&(map)->lock)
#define vm_map_lock_read(map)	lock_read(&(map)->lock)
#define vm_map_unlock_read(map)	lock_read_done(&(map)->lock)
#define vm_map_lock_write_to_read(map) \
		lock_write_to_read(&(map)->lock)
#define vm_map_lock_read_to_write(map) \
		(lock_read_to_write(&(map)->lock) || (((map)->timestamp++), 0))
#define vm_map_lock_set_recursive(map) \
		lock_set_recursive(&(map)->lock)
#define vm_map_lock_clear_recursive(map) \
		lock_clear_recursive(&(map)->lock)
#else	/* _AIX */

#ifdef  _KERNSYS
#include <sys/inline.h>
#endif /* _KERNSYS */
/*
 * Make locks NULL macros except for map lock and unlock
 * which we use to gain addressibility to address map
 * and map entry structures (contained in VMM data segment
 * which is not normally addressible).
 */
#define vm_map_lock(map)			\
MACRO_BEGIN					\
	savevmm = chgsr(VMMSR, vmker.vmmsrval);	\
MACRO_END

#define vm_map_unlock(map)			\
MACRO_BEGIN					\
	(void)chgsr(VMMSR, savevmm);		\
MACRO_END

#define vm_map_lock_init(map)
#define vm_map_lock_read(map)
#define vm_map_unlock_read(map)
#define vm_map_lock_write_to_read(map)
#define vm_map_lock_read_to_write(map)
#define vm_map_lock_set_recursive(map)
#define vm_map_lock_clear_recursive(map)

#endif	/* _AIX */

/*
 *	Exported procedures that operate on vm_map_t.
 */

#ifndef _AIX
extern void		vm_map_init();		/* Initialize the module */
#endif	/* _AIX */

extern vm_map_t		vm_map_create();	/* Create an empty map */
extern vm_map_t		vm_map_fork();		/* Create a map in the image
						 * of an existing map */

#ifndef _AIX
extern void		vm_map_reference();	/* Gain a reference to
						 * an existing map */
#endif	/* _AIX */

extern void		vm_map_deallocate();	/* Lose a reference */
extern kern_return_t	vm_map_enter();		/* Enter a mapping */
extern kern_return_t	vm_map_remove();	/* Deallocate a region */

#ifndef _AIX
extern kern_return_t	vm_map_exec();		/* remove all but keep-on-exec
						 * mappings */
#endif	/* _AIX */

extern kern_return_t	vm_map_protect();	/* Change protection */

#ifndef _AIX
extern kern_return_t	vm_map_inherit();	/* Change inheritance */
extern kern_return_t	vm_map_keep_on_exec();  /* Change keep-on-exec state */

extern kern_return_t	vm_map_find();		/* Old allocation primitive */
extern void		vm_map_print();		/* Debugging: print a map */
#endif	/* _AIX */

extern kern_return_t	vm_map_lookup();	/* Look up an address */

#ifndef _AIX
extern boolean_t	vm_map_verify();	/* Verify that a previous
						 * lookup is still valid */
extern void		vm_map_verify_done();	/* Indicate that the operation
						 * requiring a verified lookup
						 * is complete. */

extern kern_return_t	vm_map_copyin();	/* Make a copy of a region */
extern kern_return_t	vm_map_copyout();	/* Place a copy into a map */
extern kern_return_t	vm_map_copy_overwrite();/* Overwrite existing memory
						 * with a copy */
extern void		vm_map_copy_discard();	/* Discard a copy without
						 * using it */
/* Functions for msemaphore  */
extern kern_return_t	vm_msleep(); /* Wait for a semphore to be freed */
extern kern_return_t	vm_mwakeup(); /* Wakeup sleepers on semphore */
extern void		vm_msem_init();	/* Initialization for msem code */
#endif	/* _AIX */


/*
 *	Functions implemented as macros
 */
#define		vm_map_min(map)		((map)->min_offset)
						/* Lowest valid address in
						 * a map */

#define		vm_map_max(map)		((map)->max_offset)
						/* Highest valid address */

#define		vm_map_pmap(map)	((map)->pmap)
						/* Physical map associated
						 * with this address map */
#ifndef _AIX
/*
 *	Submap object.  Must be used to create memory to be put
 *	in a submap by vm_map_submap.
 */
extern vm_object_t	vm_submap_object;
#endif	/* _AIX */

#endif	/* _H_VM_MAP */
