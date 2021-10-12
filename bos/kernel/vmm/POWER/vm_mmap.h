/* @(#)34	1.5  src/bos/kernel/vmm/POWER/vm_mmap.h, sysvmm, bos411, 9428A410j 7/27/93 19:15:16 */
#ifndef	_H_VM_MMAP
#define _H_VM_MMAP

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 18 27 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * vm_mmap.h
 *
 * This header file collects all required definitions from various OSF/1
 * header files to compile source files ported from OSF/1.
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
 *	File:	vm_object.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory object module definitions.
 */
/*
 *	File:	memory_object.h
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface definition.
 */
/*
 *	File:	mach/vm_param.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Machine independent virtual memory parameters.
 *
 */
/*
 *	File:	mach/vm_prot.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory protection definitions.
 *
 */
/*
 *	File:	mach/vm_inherit.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory map inheritance definitions.
 *
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
 *	File:	kern/lock.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Locking primitives definitions
 *
 */
/*
 *	File:	kern/macro_help.h
 *
 *	Provide help in making lint-free macro routines
 *
 */
/*
 *	File:	h/kern_return.h
 *	Author:	Avadis Tevanian, Jr.
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	Kernel return codes.
 *
 */

/*---------- mach/machine/vm_types.h ----------*/

typedef	unsigned int	vm_offset_t;
typedef	unsigned int	vm_size_t;

#define VM_OFFSET_NULL	((vm_offset_t) 0)

/*---------- vm/vm_object.h ----------*/

typedef struct vnode	*vm_object_t;
#define VM_OBJECT_NULL	((vm_object_t) 0)

/*---------- mach/memory_object.h ----------*/

typedef struct vnode 	*memory_object_t;
#define MEMORY_OBJECT_NULL	((memory_object_t) 0)

typedef unsigned int	pmap_t;

/*---------- mach/vm_param.h ----------*/

#define PAGE_SIZE	PSIZE	/* size of page in addressible units */
#define page_shift	L2PSIZE	/* number of bits to shift for pages */
#define	page_mask	(PAGE_SIZE-1)	/* page_size - 1; mask for
						   offset within page */

/*
 *	Convert addresses to pages and vice versa.
 *	No rounding is used.
 */

#define atop(x)		(((unsigned)(x)) >> page_shift)
#define ptoa(x)		((vm_offset_t)((x) << page_shift))

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define round_page(x)	((vm_offset_t)((((vm_offset_t)(x)) + page_mask) & ~page_mask))
#define trunc_page(x)	((vm_offset_t)(((vm_offset_t)(x)) & ~page_mask))

/*
 *	Determine whether an address is page-aligned, or a count is
 *	an exact page multiple.
 */

#define	page_aligned(x)	((((vm_offset_t) (x)) & page_mask) == 0)


/*---------- mach/vm_prot.h ----------*/

/*
 *	Types defined:
 *
 *	vm_prot_t		VM protection values.
 */

typedef int		vm_prot_t;

/*
 *	Protection values, defined as bits within the vm_prot_t type
 */

#define VM_PROT_NONE	((vm_prot_t) 0x00)

#define VM_PROT_READ	((vm_prot_t) 0x01)	/* read permission */
#define VM_PROT_WRITE	((vm_prot_t) 0x02)	/* write permission */
#define VM_PROT_EXECUTE	((vm_prot_t) 0x04)	/* execute permission */

/*
 *	The default protection for newly-created virtual memory
 */

#define VM_PROT_DEFAULT	(VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE)

/*
 *	The maximum privileges possible, for parameter checking.
 */

#define VM_PROT_ALL	(VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE)


/*---------- mach/vm_inherit.h ----------*/
/*
 *	Types defined:
 *
 *	vm_inherit_t	inheritance codes.
 */

typedef int		vm_inherit_t;	/* might want to change this */

/*
 *	Enumeration of valid values for vm_inherit_t.
 */

#define VM_INHERIT_SHARE	((vm_inherit_t) 0)	/* share with child */
#define VM_INHERIT_COPY		((vm_inherit_t) 1)	/* copy into child */
#define VM_INHERIT_NONE		((vm_inherit_t) 2)	/* absent from child */
#define VM_INHERIT_DONATE_COPY	((vm_inherit_t) 3)	/* copy and delete */

#define VM_INHERIT_DEFAULT	VM_INHERIT_COPY


/*---------- kern/lock.h ----------*/
/*
 *	No multiprocessor locking is necessary.
 */

#ifndef _AIX
#define simple_lock_init(l)
#define simple_lock(l)
#define simple_unlock(l)
#endif	/* _AIX */

#define decl_simple_lock_data(class,name)


/*---------- mach/machine/boolean.h ----------*/
#include <sys/types.h>

/*---------- kern/macro_help.h ----------*/

/*
 *	Provide help in making lint-free macro routines
 */

#ifdef	lint
boolean_t	NEVER;
boolean_t	ALWAYS;
#else	/* lint */
#define		NEVER		FALSE
#define		ALWAYS		TRUE
#endif	/* lint */

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (NEVER)

#define		MACRO_RETURN	if (ALWAYS) return


/*---------- mach/machine/kern_return.h, mach/kern_return.h ----------*/
typedef	int		kern_return_t;

#define KERN_SUCCESS			0

#define KERN_INVALID_ADDRESS		1
		/* Specified address is not currently valid.
		 */

#define KERN_PROTECTION_FAILURE		2
		/* Specified memory is valid, but does not permit the
		 * required forms of access.
		 */

#define KERN_NO_SPACE			3
		/* The address range specified is already in use, or
		 * no address range of the size specified could be
		 * found.
		 */

#define KERN_INVALID_ARGUMENT		4
		/* The function requested was not applicable to this
		 * type of argument, or an argument
		 */

#define KERN_FAILURE			5
		/* The function could not be performed.  A catch-all.
		 */

#define KERN_RESOURCE_SHORTAGE		6
		/* A system resource could not be allocated to fulfill
		 * this request.  This failure may not be permanent.
		 */

#define KERN_NOT_RECEIVER		7
		/* The task in question does not hold receive rights
		 * for the port argument.
		 */

#define KERN_NO_ACCESS			8
		/* Bogus access restriction.
		 */

#define KERN_MEMORY_FAILURE		9
		/* During a page fault, the target address refers to a
		 * memory object that has been destroyed.  This
		 * failure is permanent.
		 */

#define KERN_MEMORY_ERROR		10
		/* During a page fault, the memory object indicated
		 * that the data could not be returned.  This failure
		 * may be temporary; future attempts to access this
		 * same data may succeed, as defined by the memory
		 * object.
		 */

#define KERN_ALREADY_IN_SET		11
		/* The port argument is already a member of a set.
		 */

#define KERN_NOT_IN_SET			12
		/* The port argument is not a member of a set.
		 */

#define KERN_NAME_EXISTS		13
		/* The task already has a translation for the name.
		 */

#define KERN_ABORTED			14
		/* The operation was aborted.  Ipc code will
		 * catch this and reflect it as a message error.
		 */

#define KERN_INTERRUPTED		15
		/* The process was interrupted out of a sleep
		 * by a signal.
		 */


#endif	/* _H_VM_MMAP */
