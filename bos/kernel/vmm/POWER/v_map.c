static char sccsid[] = "@(#)30	1.8  src/bos/kernel/vmm/POWER/v_map.c, sysvmm, bos411, 9438C411a 9/22/94 16:08:14";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: vm_map_lookup
 *
 * ORIGINS: 18 27 83
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
#endif	multimax

#else	/* _AIX */

#include <sys/types.h>
#include <sys/vnode.h>
#include "vmsys.h"
#include "vm_mmap.h"
#include "vm_map.h"
#include <jfs/inode.h>
#include <sys/syspest.h>
#include <sys/except.h>

#endif	/* _AIX */

boolean_t	vm_map_lookup_entry();	/* forward declaration */

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
#else /* _AIX */
#define SAVE_HINT(map,value) \
		(map)->hint = (value);
#endif /* _AIX */

/*
 *	vm_map_lookup: (A SIGNIFICANTLY ALTERED VERSION FOR AIX)
 *
 *	Finds the VM object, offset, and
 *	protection for a given virtual address in the
 *	specified map, assuming a page fault of the
 *	type specified.
 *
 */

kern_return_t vm_map_lookup(var_map, msid, vaddr, fault_type,
				object, offset, out_prot, pgobj, pgoff)

	vm_map_t		*var_map;	/* IN/OUT */

	int			msid;		/* IN */
	register vm_offset_t	vaddr;		/* IN */
	register vm_prot_t	fault_type;	/* IN */

	int			*object;	/* OUT */
	vm_offset_t		*offset;	/* OUT */
	int			*pgobj;		/* OUT */
	vm_offset_t		*pgoff;		/* OUT */
	vm_prot_t		*out_prot;	/* OUT */

{
	register vm_map_entry_t		entry;
	register vm_map_t		map = *var_map;
	register vm_prot_t		prot;
	int			msidx, found, sidx;
	vm_offset_t		segoff;
	struct inode *ip;
	int			validmap = 1;

	/*
	 * We must perform lookup using (sid,pno) rather than by
	 * effective address since we need to handle kernel faults
	 * on mapping regions for which a different segment register
	 * may have been used (e.g. kernel does a vm_att() to region).
	 */
	msidx = STOI(msid);
	segoff = vaddr & SOFFSET;

	/*
	 * If the map has an interesting hint, try it before searching.
         * The mapping segment may not exist in this process' address map
         * or the map could be NULL.
	 */
        if (map == NULL)
                validmap = 0;
        else
                entry = map->hint;

	if (!validmap || (entry == vm_map_to_entry(map)) ||
	    (entry->mapping_sid != msid) ||
	    (segoff < (entry->vme_start & SOFFSET)) || 
	    (segoff > ((entry->vme_end - 1) & SOFFSET))) {
		/*
		 * Entry was either not a valid hint, or 
		 * was not the right entry so we need to
		 * search. The SCB search hint points to
		 * the first map entry which corresponds
		 * to the segment we faulted in.
		 */
		entry = scb_ame(msidx);
		ASSERT(entry != VM_MAP_ENTRY_NULL);
		found = FALSE;
		while (entry->mapping_sid == msid) {
			if (((entry->vme_end - 1) & SOFFSET) >= segoff) {
				if (segoff >= (entry->vme_start & SOFFSET)) {
			    		/*
				 	 * Save this lookup for future hints
				 	 */
					if (validmap)
						SAVE_HINT(map, entry);
					found = TRUE;
				}
				break;
			}
			entry = entry->vme_next;
		}
		if (!found)
			return(KERN_INVALID_ADDRESS);
		
	}
		
	/*
	 * Check whether this process is allowed to have
	 * this page.
	 */

	prot = entry->protection;
	if ((fault_type & (prot)) != fault_type)
		return(KERN_PROTECTION_FAILURE);

	*out_prot = prot;

	/*
	 * Determine source SCB and offset.
	 */
	*object = entry->source_sid;
	ASSERT(*object != 0);
	*offset = (segoff - (entry->vme_start & SOFFSET)) + entry->offset;

	/*
	 * If the source object is a file then determine if the
	 * reference is beyond end-of-file.  Note that this check is
	 * made here for the local file system only.  Client segments
	 * are handled by the strategy routine indicating EOF condition.
	 * Note also that a reference in the partial page beyond EOF is
	 * legal (SIGBUS only occurs for references to full pages beyond EOF).
	 */
	sidx = STOI(entry->source_sid);
	if (scb_pseg(sidx)) {
		ip = GTOIP(scb_gnptr(sidx));
		if ((*offset >> L2PSIZE) > BTOPN(ip->i_size))
			return(EXCEPT_EOF);
	}

	/*
	 * Determine paging SCB and offset if any.
	 */
	if (entry->paging_sid != INVLSID) {
		/*
		 * Since the first offset in the source SCB corresponds
		 * to offset 0 in the paging SCB, we subtract the original
		 * source offset to get the offset within the paging SCB.
		 */
		*pgobj = entry->paging_sid;
		*pgoff = (segoff - (entry->vme_start & SOFFSET))
			+ (entry->offset - entry->orig_offset);
		ASSERT(*pgoff < SEGSIZE);
	} else {
		*pgobj = INVLSID;
		*pgoff = 0;
	}

	return(KERN_SUCCESS);
}

