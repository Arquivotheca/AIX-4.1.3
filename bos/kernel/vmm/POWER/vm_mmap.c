static char sccsid[] = "@(#)32	1.5.3.2  src/bos/kernel/vmm/POWER/vm_mmap.c, sysvmm, bos41J, 9513A_all 3/24/95 13:27:58";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: mmap, munmap, mprotect, msync, madvise, mincore, mvalid
 *	      msleep, mwakeup, msem_remove
 *
 * ORIGINS: 65 27 83
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_mman.c	7.1 (Berkeley) 6/5/86
 */

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mstsave.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/vnode.h>
#include <jfs/inode.h>
#include <sys/fp_io.h>
#include <sys/stat.h>
#include <sys/syspest.h>

#include "vmdefs.h"
#include "vmmacs.h"
#include "vm_mmap.h"
#include "vm_map.h"
#include <sys/vmuser.h>
#include <sys/mman.h>
#include <sys/vmker.h>
#include <sys/atomic_op.h>

static vm_prot_t vm_prots[] = {
	VM_PROT_NONE,
	VM_PROT_READ,
	VM_PROT_READ|VM_PROT_WRITE,
	VM_PROT_READ|VM_PROT_WRITE,
	VM_PROT_READ|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE
};

#define MAX_PROT (PROT_EXEC|PROT_WRITE|PROT_READ)
 
/*
 * Lower and upper bounds for mmap() regions in process address space.
 */
#define VM_MIN_ADDRESS	((unsigned) SHMLOSEG*SEGSIZE)
#define VM_MAX_ADDRESS	((unsigned) (SHMHISEG+1)*SEGSIZE)

/* Ideally, these define should be shared with the driver for /dev/zero  */
#define DEV_ZERO_MAJOR   2 
#define DEV_ZERO_MINOR   3


/*
 * mmap()
 */
caddr_t
mmap(addr, len, prot, flags, fd, pos)
caddr_t	addr;
size_t	len;
int	prot;
int	flags;
int	fd;
off_t	pos;
{
	vm_map_t	user_map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	struct file *fp;
	struct stat	stst;
	register struct vnode *vp;
	enum vtype	vtype;
	off_t		file_pos = pos;
	memory_object_t	pager;
	vm_offset_t	pager_offset;
	vm_prot_t	user_prot;
	vm_prot_t	max_prot;
	boolean_t	anywhere;
	int		flag;
	kern_return_t	result;
	int		error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * We create address map on first mmap() call.
	 */
	if (user_map == VM_MAP_NULL) {
		user_map = vm_map_create((pmap_t) 0, VM_MIN_ADDRESS,
					VM_MAX_ADDRESS, FALSE);
		if (user_map == VM_MAP_NULL) {
			error = ENOMEM;
			goto out;
		}
		u.u_map = (caddr_t) user_map;
	}

	/*
	 * Check the prot argument.
	 */
	if ((prot < 0) || (prot > MAX_PROT)) {
		error = EINVAL;
		goto out;
	}
	user_prot = vm_prots[prot];

	/*
	 * Check the flags argument.
	 */
	if ((flags & MAP_SHARED) && (flags & MAP_PRIVATE)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Both MAP_FILE and MAP_VARIABLE are defined 0x00 so:
	 * Impossible to check both MAP_FIXED and MAP_VARIABLE being defined.
	 * Impossible to check both MAP_FILE and MAP_ANONYMOUS being defined. 
	 */

	anywhere = (flags & MAP_FIXED) ? FALSE : TRUE;

	/*
	 * Start addr must be page aligned for MAP_FIXED.
	 */
	if (!anywhere && !page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Offset must be page aligned.
	 */
	if (!page_aligned(file_pos)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Round the start address to the nearest page boundary
	 * (need to ensure this for non-MAP_FIXED case as well).
	 */
	user_addr = round_page(user_addr);

	/*
	 * Round the end address to the nearest page boundary.
	 * If requested size is zero, just return success
	 * (don't bother to set up a zero length mapping).
	 */
	user_size = round_page(user_size);
	if (user_size == (vm_offset_t) 0)
		goto out;

	if ((flags & MAP_TYPE) == MAP_ANON) {
		/* 
		 * Map an anonymous object.
		 *
		 * Offset is 0.
		 * Maximum protection will be ALL.
		 */
		if (fd != -1) {
			error = EBADF;
			goto out;
		}

		vp = NULL;
		pager = MEMORY_OBJECT_NULL;
		pager_offset = (vm_offset_t) 0;
		max_prot = VM_PROT_ALL;
	} else  {
		/*
		 * Map a file system object.
		 */


                if (error = getft(fd,&fp,DTYPE_VNODE)) {
                       /*
                        * Return ENODEV for wrong VNODE type.
                        */
                       if (error == EINVAL)
                           error = ENODEV;
		       goto out;
                      }
		vp = (struct vnode *)fp->f_data;

		/*
		 * Maximum protections are based on file descriptor protections
		 * for shared mapping; for private mapping, all access allowed.
		 * However, file must be opened for reading even for private
		 * mapping.
		 */
		flag = fp->f_flag;
		if ((flags & (MAP_SHARED|MAP_PRIVATE)) == MAP_SHARED) {
			max_prot = VM_PROT_NONE;
			if (flag & FREAD)
				max_prot |= VM_PROT_READ | VM_PROT_EXECUTE;
			if (flag & FWRITE)
				max_prot |= VM_PROT_WRITE;
		} else {
			if (!(flag & FREAD)) {
				error = EACCES;
				goto out1;
			}
			max_prot = VM_PROT_ALL;
		}
				
		/*
		 * Requested protections must be within maximums.
		 */
		if ((user_prot & ~max_prot) != VM_PROT_NONE) {
			error = EACCES;
			goto out1;
		}

		vtype = vp->v_type;

		/*
		 * Determine type of file.
		 */
		if (vtype == VREG) {
			/*
		 	 * Regular file.
			 */
			
			/*
			 * Range specified by [off,off+len) must be legitimate
			 * for the possible size of the file.
			 */
			if (file_pos > MAXFSIZE ||
			    user_size > MAXFSIZE ||
			    file_pos + user_size > MAXFSIZE) {
				error = ENXIO;
				goto out1;
			}

			/*
			 * Pass the vnode pointer and file offset.
			 */
			pager = vp;
			pager_offset = (vm_offset_t) file_pos;

		} else if (vtype == VCHR) {
			/*
			 * Character special file.
			 */

			/*
			 * Support anonymous mapping through /dev/zero.
			 */
			if (VTOGP(vp)->gn_rdev == 
                             makedev(DEV_ZERO_MAJOR,DEV_ZERO_MINOR)) {
				vp = NULL;
				pager = MEMORY_OBJECT_NULL;
				pager_offset = (vm_offset_t) 0;
				flags |= MAP_ANONYMOUS;
				max_prot = VM_PROT_ALL;
			} else {
				/* XXX -
				 * Map in special device memory
				 * (e.g., frame buffer).
				 */
				error = ENODEV;
				goto out1;
			}
		} else {
			error = ENODEV;
			goto out1;
		}
	}

	/* 
	 * Map the object.
	 */
	result = vm_map_enter(user_map, &user_addr, user_size,
			vmker.cachealign - 1, anywhere,
			pager, pager_offset,
			((flags & (MAP_SHARED|MAP_PRIVATE)) != MAP_SHARED),
			user_prot,
			max_prot,
			((flags & (MAP_SHARED|MAP_PRIVATE)) == MAP_SHARED ?
			 VM_INHERIT_SHARE : VM_INHERIT_COPY)
			);

	if (result != KERN_SUCCESS)
	      switch (result) {

	      case KERN_NO_SPACE:
	      case KERN_INVALID_ADDRESS:
		error = ENOMEM;
		break;

	      case KERN_MEMORY_FAILURE:
	      case KERN_MEMORY_ERROR:
		error = EIO;
		break;

	      case KERN_PROTECTION_FAILURE:
		error = EACCES;
		break;

	      default:
		error = EINVAL;
		break;
	}

out1:
	/* Decrement the file descriptor count */
	if (fd != -1)
		ufdrele(fd);

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? (caddr_t)-1 : (caddr_t)user_addr);
}


/*
 * munmap()
 */
int
munmap(addr, len)
caddr_t	addr;
size_t	len;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * If specified region [addr,addr+len) includes addresses
	 * outside valid mapping region then fail request.
	 */
	if (user_addr < VM_MIN_ADDRESS ||
	    user_addr + user_size > VM_MAX_ADDRESS) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Start addr must be page aligned.
	 */
	if (!page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * If address map doesn't exist, just return success.
	 * (We don't require that specified region corresponds exactly
	 * to a region created with mmap() or that it even contain one).
	 */
	if (map == VM_MAP_NULL) {
		goto out;
	}

	/*
	 * Round the end address to the nearest page boundary.
	 */
	if (user_size == (vm_offset_t) 0)
		goto out;
	user_size = round_page(user_size);

	/*
	 * Remove any mappings in the specified region.
	 */
	result = vm_map_remove(map, user_addr, user_addr + user_size);
	if (result != KERN_SUCCESS)
		error = EINVAL;

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}


/*
 * mprotect()
 */
int
mprotect(addr, len, prot)
caddr_t	addr;
size_t	len;
int	prot;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	vm_prot_t	user_prot;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Special support for setting red-zones in malloc'd thread stacks.
	 * The specified range must be within the break value in the process
	 * private segment and the specified protection cannot be PROT_NONE.
	 */
	if (((uint)addr >> SEGSHIFT) == PRIVSEG &&
	    ((uint)(addr + len - 1) >> SEGSHIFT) == PRIVSEG &&
	    prot != PROT_NONE)
	{
		if (((uint)(addr + len) & SOFFSET) <= U.U_dsize)
			return(vm_protect(addr, len, VMPROT2PP(prot)));
	}
	    
	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * If specified region [addr,addr+len) includes addresses
	 * outside valid mapping region then fail request.
	 */
	if (user_addr < VM_MIN_ADDRESS ||
	    user_addr + user_size > VM_MAX_ADDRESS) {
		error = ENOMEM;
		goto out;
	}

	/*
	 * Check the prot argument.
	 */
	if ((prot < 0) || (prot > MAX_PROT)) {
		error = EINVAL;
		goto out;
	}
	user_prot = vm_prots[prot];

	/*
	 * Start addr must be page aligned.
	 */
	if (!page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * If address map doesn't exist, just return success.
	 * (We don't require that specified region corresponds exactly
	 * to a region created with mmap() or that it even contain one).
	 */
	if (map == VM_MAP_NULL) {
		goto out;
	}

	/*
	 * Round the end address to the nearest page boundary.
	 */
	user_size = round_page(user_size);

	/*
	 * Change protections of any mappings in the specified region.
	 */
	result = vm_map_protect(map, user_addr, user_addr + user_size,
				user_prot, FALSE);

	if (result != KERN_SUCCESS) switch (result) {
		case KERN_PROTECTION_FAILURE:
			error = EACCES;
			break;

		case KERN_NO_SPACE:
		case KERN_INVALID_ADDRESS:
			error = ENOMEM;
			break;

		default:
			error = EINVAL;
	}

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}


/*
 * msync()
 */
int
msync(addr, len, flags)
caddr_t	addr;
size_t	len;
int	flags;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	kern_return_t result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * If specified region [addr,addr+len) includes addresses
	 * outside valid mapping region then fail request.
	 */
	if (user_addr < VM_MIN_ADDRESS ||
	    user_addr + user_size > VM_MAX_ADDRESS) {
		error = ENOMEM;
		goto out;
	}

	/*
	 * Start addr must be page aligned.
	 */
	if (!page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Flags can only be one of the following values:
	 */
	switch (flags) {
	case MS_SYNC:
	case MS_ASYNC:
	case MS_INVALIDATE:
		break;

	default:
		error = EINVAL;
		goto out;
	}
	
	/*
	 * If address map doesn't exist, return failure.
	 * (All pages within specified region must be mapped).
	 */
	if (map == VM_MAP_NULL) {
		error = ENOMEM;
		goto out;
	}

	/*
	 * Round the end address to the nearest page boundary.
	 */
	user_size = round_page(user_size);
	
	/*
	 * Sync any mappings in the specified region.
	 */
	result = vm_map_sync(map, user_addr, user_addr + user_size, flags);

	if (result) switch(result) {
	case KERN_INVALID_ADDRESS:
		error = ENOMEM;
		break;

	case KERN_MEMORY_ERROR:
		error = EIO;
		break;

	default:
		error = EINVAL;
	}

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}


/*
 * madvise()
 */
int
madvise(addr, len, behav)
caddr_t	addr;
size_t	len;
int	behav;
{
        int error = 0;

	/* just check for invalid behav argument */
        switch (behav) {
        case MADV_NORMAL:
        case MADV_RANDOM:
        case MADV_SEQUENTIAL:
        case MADV_WILLNEED:
        case MADV_DONTNEED:
        case MADV_SPACEAVAIL:
          break;

        default:
          error = EINVAL;
        }

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}

/*
 * mincore()
 */
int
mincore(addr, len, vec)
caddr_t	addr;
size_t	len;
char	*vec;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	kern_return_t result;
	int error = 0;
	int npages, p;
	vm_offset_t thisaddr;
	char *thisvec, incore;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * If specified region [addr,addr+len) includes addresses
	 * outside valid mapping region then fail request.
	 */
	if (user_addr < VM_MIN_ADDRESS ||
	    user_addr + user_size > VM_MAX_ADDRESS) {
		error = ENOMEM;
		goto out;
	}

	/*
	 * Start addr must be page aligned.
	 */
	if (!page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * If address map doesn't exist, then fail.
	 */
	if (map == VM_MAP_NULL) {
		error = ENOMEM;
		goto out;
	}
	/*
	 * For each page covered by the range [addr, addr + len)
	 * determine its memory residency.
	 */
	npages = user_size >> L2PSIZE;
	thisaddr = user_addr;
	thisvec = vec;
	for (p = 0; p < npages; p++)
	{
		/*
		 * See if page is in memory at this alias or
		 * at source address or at some other alias.
		 */
		result = vm_map_incore(map, thisaddr, &incore);
		if (result != KERN_SUCCESS) {
			error = ENOMEM;
			goto out;
		}

		if (subyte(thisvec, incore) != 0) {
			error = EFAULT;
			break;
		}

		thisaddr += PAGE_SIZE;
		thisvec++;
	}

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}

/*
 * mvalid()
 */
int
mvalid(addr, len, prot)
caddr_t	addr;
size_t	len;
int	prot;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	vm_offset_t	user_addr = (vm_offset_t) addr;
	vm_size_t	user_size = (vm_size_t) len;
	vm_prot_t	user_prot;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/*
	 * If specified region [addr,addr+len) includes addresses
	 * outside valid mapping region then fail request.
	 */
	if (user_addr < VM_MIN_ADDRESS ||
	    user_addr + user_size > VM_MAX_ADDRESS) {
		error = ENOMEM;
		goto out;
	}

	/*
	 * Check the prot argument.
	 */
	if ((prot < 0) || (prot > MAX_PROT)) {
		error = EINVAL;
		goto out;
	}
	user_prot = vm_prots[prot];

	/*
	 * Start addr must be page aligned.
	 */
	if (!page_aligned(user_addr)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * If address map doesn't exist, then fail.
	 * (We do require that specified region contain regions
	 * created with mmap() and that it not contain any holes).
	 */
	if (map == VM_MAP_NULL) {
		error = EACCES;
		goto out;
	}

	/*
	 * Round the end address to the nearest page boundary.
	 */
	user_size = round_page(user_size);

	/*
	 * Check protections of all pages in the specified region.
	 */
	result = vm_map_check_protection(map, user_addr, user_addr + user_size,
					user_prot);

	if (!result)
		error = EACCES;

out:
	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}

/*
 * msleep - If semaphore is still locked when checked put requestor
 * 	    to sleep, waiting for an mwakeup on the semaphore.
 */
msleep(sem)
msemaphore *sem;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	result = vm_msleep(map, sem);

	if (result) switch(result) {
	case KERN_INTERRUPTED:
		error = EINTR;
		break;

	default:
		error = EFAULT;
	}

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}

/*
 * mwakeup - Wakeup any processes waiting on the semaphore.
 *
 * Note the semaphore may still be locked.
 */
mwakeup(sem)
msemaphore *sem;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	result = vm_mwakeup(map, sem);

	if (result) {
		error = EFAULT;
	}

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}
/*
 * msem_remove - remove a semaphore
 *
 */
int
msem_remove(sem)
msemaphore *sem;
{
	vm_map_t	map = (vm_map_t) u.u_map;
	kern_return_t	result;
	int error = 0;
	int		mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	result = vm_msem_remove(map, sem);

	if (result == KERN_INVALID_ARGUMENT) {
		error = EINVAL;
	}
	else
	if (result) {
		error = EFAULT;
	}

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	u.u_error = error;
	return (u.u_error ? -1 : 0);
}
