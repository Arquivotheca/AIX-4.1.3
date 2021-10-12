static char sccsid[] = "@(#)27	1.12  src/bos/kernel/pfs/xix_rele.c, syspfs, bos41J, 9514A_all 3/31/95 12:52:42";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_rele
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/vfs.h"
#include "sys/syspest.h"
#include "sys/atomic_op.h"

/* 
 * NAME:	jfs_rele(vp)
 *
 * FUNCTION:	release a reference on a vnode. 
 *		if it was the last reference to the vnode, 
 *		release a reference of the associated inode.
 *
 * PARAMETERS:	vp	- The doomed vnode pointer.
 *
 * RETURNS:	Returns any errors that occur below this layer.
 *
 * SERIALIZATION:
 *	ICACHE_LOCK serializes v_count of device mount fs vnodes.
 *	IWRITE_LOCK serializes v_count of softmount fs vnodes.
 *	gnode vnodelist is serialized by IWRITE_LOCK AFTER
 *	it has been initialized by iget()).
 */
jfs_rele(vp)
struct vnode *vp;
{
	struct inode	*ip = VTOIP(vp);
	struct vfs	*vfsp = vp->v_vfsp;

	/* device mount fs vnode
	 */
	if (vfsp->vfs_flag & VFS_DEVMOUNT)
	{
		int old;
		int new;
		
		old = vp->v_count;
		do
		{
			if (old <= 1)
			{
				assert(old == 1);
				ICACHE_LOCK();
				iput(ip, vfsp);
				ICACHE_UNLOCK();
				break;
			}
			new = old - 1;
		} while(!compare_and_swap((atomic_p)&vp->v_count, &old, new));
		return 0;
	}

	/*
	 * softmount fs object
	 */
	IWRITE_LOCK(ip);
	if (--vp->v_count > 0)
	{
		IWRITE_UNLOCK(ip);
		return 0;
	}

	/*
	 * last reference release (v_count = 0) of softmount fs vnode
	 *
	 * free vnode and release reference of inode of this vnode.
	 */
	vn_free(vp);
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, NULL);
	ICACHE_UNLOCK();

	return 0;
}
