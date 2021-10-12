static char sccsid[] = "@(#)44	1.13  src/bos/kernel/lfs/fsync.c, syslfs, bos411, 9428A410j 10/22/93 17:29:35";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fsync
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

#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/trchkid.h"

BUGVDEF(fsdebug, 0);          /* define fsync debug variable */

/*
 * Synch an open file.
 */

/*
 * NAME:	fsync()		(system call entry point)
 *
 * FUNCTION:	causes all modified data in an open file to be saved to
 *		permanent storage.  IF the file is mapped onto a segment
 *		in read/write mode, then it is save to permanenet storage
 *		If the file is mapped copy-on-write, then the pages of the file
 *		that have been changed are saved to permanent storage.
 *
 * PARMETERS:	filedes.  filedes is an integer file descriptor of a regular
 *		open file.
 *
 * RETURN VALUE:	Zero is returned if fsync completes sucessfully.
 *			a -1 is returned if it fails and errno is set to
 *			one of the following:
 *			
 *			EIO	an I/O error occurred.
 *			
 *			EBADF   Fildes is not a valid file descriptor for
 *				writing
 *
 *			EINVAL	The file is a FIFO file, directory, or
 *				special file.
 */
fsync(fd)
int	fd;
{
	struct vnode *vp;
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((rc = getft(fd, &fp, DTYPE_VNODE)) == 0)
	{
		vp = fp->f_vnode;

		/* trace file descriptor and vnode pointer */
		TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_FSYNC, fd, vp);

		if ((fp->f_flag & FWRITE) == 0) 
			rc = EBADF;
		else if (vp->v_vntype != VREG)
			rc = EINVAL;
		else
			/* looks good, sync it */
			rc = VNOP_FSYNC(vp, fp->f_flag, fd, fp->f_cred);

		ufdrele(fd);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
