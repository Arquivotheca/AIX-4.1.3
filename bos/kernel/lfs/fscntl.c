static char sccsid[] = "@(#)03	1.6.1.5  src/bos/kernel/lfs/fscntl.c, syslfs, bos411, 9428A410j 5/16/94 13:25:28";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fscntl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/audit.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/vfs.h"

BUGVDEF(fscntldbg, 0);

extern struct vfs *rootvfs;

/*
 *	fscntl system call
 */
int
fscntl(vfs_no, cmd, arg, argsize)
int		vfs_no;
int		cmd;
caddr_t		arg;
size_t		argsize;
{
	register struct vfs *vfsp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	static int svcnum=0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag)
	    && (audit_svcstart("FS_Extend", &svcnum, 2, vfs_no, cmd)))
		audit_svcfinis();

	/*
	 * Look up the thing to be fscntl'd
	 */

	/* 
	 * Take the vfs_list_lock to search the vfs list to
	 * find the object to be fscntl'd.
	 */
	VFS_LIST_LOCK();
	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next)
		if ( !(vfsp->vfs_flag &
			(VFS_DISCONNECTED | VFS_SOFT_MOUNT | VFS_UNMOUNTING))
		     && (vfsp->vfs_number == vfs_no) )
		{
			vfs_hold(vfsp);
			break;
		}
	VFS_LIST_UNLOCK();

	/*
	 * call the vfs op if we found the vfs
	 */
	if (vfsp == NULL)
		rc = EINVAL;
	else
	{
		struct ucred *crp = crref();
		rc = VFS_CNTL(vfsp, cmd, arg, argsize, crp);
		vfs_unhold(vfsp);
		crfree(crp);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
