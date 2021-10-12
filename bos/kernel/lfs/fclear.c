static char sccsid[] = "@(#)40	1.14.1.6  src/bos/kernel/lfs/fclear.c, syslfs, bos411, 9428A410j 4/19/94 16:15:23";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fclear
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *
 *    System Call to Clear a region of a file.
 *
 *    syntax:
 *        fclear()            (system call entry point)
 *
 *    function:   To zero out a portion of a file, any full blocks
 *                   zeroed out are freed.
 *
 *    description:   Get the file structure asssociated with the
 *                   file descriptor. Verify the file is opened with
 *                   WRITE access and that it is a regular file.
 *                   Call the filesystem dependent routine to do
 *                   the actual "clearing" of the file region.
 *                   The region is cleared starting from the file
 *                   offset set in the file structure.
 *
 *    input:
 *       fd     = the file descriptor as returned by the open call,
 *       length = number of bytes to zero out,
 *       file offset = set in the file structure.
 *
 *
 *    output:
 *           area cleared in file.
 *           inode updated with new time and size.
 *           file offset incremented by number of bytes cleared.
 *           An error code of -1 is returned if the call fails
 *
 */

#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/trchkid.h"
#include "sys/audit.h"


BUGVDEF(fcdebug, 0);          /* define fclear debug variable */

/*
 * make a length byte hole in a file starting at f_offset
 */
fclear(fd, length)
int		fd;
unsigned long	length;
{
	register struct vnode *vp;
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
#ifdef OFFSET_LOCK
	int offset_locked = 0;  /* f_offset field locked                */
#endif
	static int svcnum = 0;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Write", &svcnum, 1, fd)))
		audit_svcfinis();

	if ((rc = getft(fd, &fp, DTYPE_VNODE)) != 0)
	{
		u.u_error = rc;
		if( lockt != LOCK_NEST )
			FS_KUNLOCK(&kernel_lock);
		return -1;
	}

	/* trace file descriptor, vnode pointer, and length */
	TRCHKL3T(HKWD_SYSC_LFS | hkwd_SYSC_FCLEAR, fd,
	        fp->f_vnode, length);

	if ((fp->f_flag & FWRITE) == 0) {         /* file opened for write*/
		rc = EBADF;
		goto errout;
	}
	vp = (struct vnode *) fp->f_vnode;      /* get vnode */
	if (vp->v_vntype != VREG) {
		rc = EINVAL;
		goto errout;
	}
	if (length > 0) {

		/*
		 * call filesystem dependent fclear routine.
		 */

#ifdef OFFSET_LOCK
		/*
		 * Holding the offset lock across the VNOP can cause deadlock
		 * if the underlying filesystem supports enforced mode 
		 * record locking.  Another thread can block this thread
		 * with a record lock and be blocked by this threads offset 
		 * lock.  Version 3 did not attempt to protect the offset
		 * between processes which shared common file table entries, 
		 * so we will continue that behaviour.  Several suggestions
		 * such as performing common_reclock() in the LFS were 
		 * suggested, but these medicines were deemed too bitter
		 * to solve this problem.  This code is not to be compiled
		 * into the LFS, and is here for historical purposes.
	 	 */ 
		if ((U.U_procp->p_threadcount != 1) || (fp->f_count != 1))
		{
			offset_locked = 1;
			FP_OFFSET_LOCK(fp);
		}
#endif

		rc = VNOP_FCLEAR(vp, fp->f_flag, fp->f_offset, 
				 length, fp->f_vinfo, fp->f_cred);
		if (rc == 0)
			fp->f_offset += length;    /* adjust file offset */

#ifdef OFFSET_LOCK
		if (offset_locked)
			FP_OFFSET_UNLOCK(fp);
#endif
	}
errout:

	ufdrele(fd);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : length;     /* set return value */
}
