static char sccsid[] = "@(#)12	1.18  src/bos/kernel/lfs/ustat.c, syslfs, bos41B, 412_41B_sync 12/6/94 16:47:33";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: ustat
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
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/var.h"
#include "sys/stat.h"
#include "ustat.h"
#include "sys/syspest.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/statfs.h"

BUGVDEF(ustatdbg, 0);
extern struct vfs *rootvfs;

ustat(uapdev, ustp)
dev_t		uapdev;
struct ustat *	ustp;
{
	register remote;		/* flag: !=0 ==> VFS_REMOTE	*/
	register dev_t dev = uapdev;
	register struct vfs *vfsp;
	struct ustat	ust;
	struct statfs	fsbuf;
	dev_t vfsdev;
	struct vnode *rootv;		/* root vnode			*/
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred    *crp;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get current user credentials */
	crp = crref();

	BUGLPR(ustatdbg, BUGACT, ("Entry, devno x%x\n", dev));

	/*
	 * remote vs local:
	 *	if the uapdev is marked as remote, we only look at
	 *	the remote VFS structs.
	 *	otherwise, we only look at the local ones.
	 */
	dev ^= (remote = (dev & SDEV_REMOTE));

	/*
	 * browse through the mounted filesystems, looking for this
	 * dev...
	 */
	VFS_LIST_LOCK();
	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next) {
		/*
		 * Make sure the vfs isn't disconnected or unmounting
		 */
		if (vfsp->vfs_flag &
			    (VFS_DISCONNECTED|VFS_UNMOUNTING))
			continue;

		/*
		 * If it's a local dev and the vfs in question is
		 * a device mount, then fetch the root vnode and compare
		 * the rdev for the vfs with the dev requested.  Ignore
		 * file systems that are unresponsive.
		 */
		if (!remote && vfsp->vfs_flag & VFS_DEVMOUNT)
		{
			vfs_hold(vfsp);
			if (VFS_ROOT(vfsp, &rootv, crp))
			{
				vfs_unhold(vfsp);
				continue;
			}
			vfsdev = brdev((VTOGP(rootv))->gn_rdev);
			VNOP_RELE(rootv);

			if (vfsdev == dev)
				break;
			else
				vfs_unhold(vfsp);
		}

		/*
		 * If it's a remote dev and the vfs is remote, then
		 * compare the vfs number with the dev requested.
		 */
		if (remote && vfsp->vfs_flag & VFS_REMOTE 
		    && dev == vfsp->vfs_number)
		{
			vfs_hold(vfsp);
			break;
		}
	}
	VFS_LIST_UNLOCK();

	if (vfsp)
	{
		/* attempt to get the stats from the vfs */

		rc = VFS_STATFS(vfsp, &fsbuf, crp);

		/* decrement the reference count on the vfs */ 
		vfs_unhold(vfsp);

		if (rc == 0)
		{
			/*
			* Construct the ustat information based on
			* the returned statfs structure.  Note that
			* f_tfree is scaled in UBSIZE units instead
			* of the actual block size of the file system.
			*/

			ust.f_tfree = (fsbuf.f_bfree * fsbuf.f_bsize)/UBSIZE;
			ust.f_tinode = fsbuf.f_ffree;
			bcopy(fsbuf.f_fname,ust.f_fname,sizeof(ust.f_fname));
			bcopy(fsbuf.f_fpack,ust.f_fpack,sizeof(ust.f_fpack));

			/* now copy it back to the user's space */
			if( copyout(&ust,ustp,sizeof(ust)) )
				rc = EFAULT;
		}
	}
	else
	{
		BUGLPR(ustatdbg, BUGACT, ("No match!\n"));
		rc = EINVAL;
	}

	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
