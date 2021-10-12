static char sccsid[] = "@(#)49	1.20  src/bos/kernel/lfs/lockf.c, syslfs, bos41B, 412_41B_sync 1/4/95 10:25:48";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: lockf
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/flock.h>
#include <sys/user.h>
#include "sys/fs_locks.h"
#include <sys/errno.h>
#include <sys/lockf.h>
#include <sys/trchkid.h>

/*
 * NAME: 	lockf()		(system call entry point)
 *
 * FUNCTION:	Used to lock and unlock regions of an open file.
 *
 * PARAMETERS:	Fdes, function, size.  Fdes is an integer open file
 *		descriptor, request is an integer constant defined in
 *		<sys/lockf.h>, and size is the number of bytes to be
 *		locked or unlocked.
 *
 * RETURN VALUES: 	Zero is returned if there was successfull
 *			completion, otherwise a -1 is returned and
 *			u.u_error is set.
 */
lockf(fdes, function, size)
int fdes;
int function;
long size;
{
	register struct vnode *vp;
	register int locking = 0;
	struct eflock lockbuf;
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	TRCHKL4T(HKWD_SYSC_LOCKF, fdes, function, (size >= 0? size : -size),
		 (size >= 0? 0 : size));

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	rc = getft(fdes, &fp, DTYPE_VNODE);

	if (rc)
		goto getft_out;

	vp = fp->f_vnode;

	lockbuf.l_whence = 1;

	if (size < 0) {
		lockbuf.l_start = size;
		lockbuf.l_len = -size;
	} 
	else {
		lockbuf.l_start = 0;
		lockbuf.l_len = size;
	}

	switch (function) {
	case F_ULOCK:
		lockbuf.l_type = F_UNLCK;
		locking = SETFLCK;
		break;

	case F_LOCK:
	case F_TLOCK:
		/* check write permission on file */
		if (!(fp->f_flag & FWRITE)) {
			rc = EBADF;
			goto trace_out;
		}

		lockbuf.l_type = F_WRLCK;
		locking = SETFLCK;
		/* The only difference between F_LOCK and F_TLOCK */
		if (function == F_LOCK)
			locking |= SLPFLCK;
		break;

	case F_TEST:
		lockbuf.l_type = F_WRLCK;
		locking = 0;
		break;

	default:
		rc = EINVAL;
		goto trace_out;
	}
	lockbuf.l_vfs = vp->v_vfsp->vfs_type;
	lockbuf.l_pid = U.U_procp->p_pid;
	lockbuf.l_sysid = 0;

	rc = VNOP_LOCKCTL(vp, fp->f_offset, &lockbuf, locking,
			  0, 0, fp->f_cred);

	if (rc != 0) {
		if (rc == EAGAIN)
			rc = EACCES;
	}
	else {
		if ((function == F_TEST) && (lockbuf.l_type != F_UNLCK))
			rc = EACCES;

		if (locking & SETFLCK) {
			int tlock;  /* is multi-thread locking required? */

			if (tlock = (U.U_procp->p_active > 1))
				U_FD_LOCK();
			U.U_lockflag = 1;
			U.U_pofile(fdes) |= UF_FDLOCK;
			if (tlock)
				U_FD_UNLOCK();
		}
	}

trace_out:

	ufdrele(fdes);

getft_out:

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
