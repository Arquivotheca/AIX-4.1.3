static char sccsid[] = "@(#)13	1.10  src/bos/kernel/pfs/xix_hold.c, syspfs, bos41J, 9514A_all 3/31/95 12:52:21";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_hold
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

/*
 * NAME:	jfs_hold(vp)
 *
 * FUNCTION:	Increment the hold count on a vnode.
 *
 * PARAMETERS:	vp	- vnode to hold.
 *
 * RETURNS:	always returns 0.
 *
 * SERIALIZATION:
 *	fetch_and_add() serializes v_count of device mount fs vnodes.
 *	IWRITE_LOCK serializes v_count of softmount fs vnodes.
 */
jfs_hold (vp)
struct vnode *vp;
{
	struct inode *ip = VTOIP(vp);
	struct vfs	*vfsp = vp->v_vfsp;

	if (vfsp->vfs_flag & VFS_DEVMOUNT)
	{
		/* device mount fs object
		 */
		fetch_and_add(&vp->v_count, 1);
	}
	else
	{
		/* soft mount fs object
		 */
		IWRITE_LOCK(ip);
		vp->v_count++;
		IWRITE_UNLOCK(ip);
	}

	return 0;
}
