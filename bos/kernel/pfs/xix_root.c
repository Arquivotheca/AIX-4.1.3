static char sccsid[] = "@(#)31	1.14  src/bos/kernel/pfs/xix_root.c, syspfs, bos411, 9428A410j 7/7/94 16:54:44";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_root
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

#include "sys/vfs.h"
#include "sys/errno.h"
#include "sys/syspest.h"

/*
 * NAME:	jfs_root (vfsp, vpp, crp)
 *
 * FUNCTION:	Return root vnode for vfsp
 *
 * PARAMETERS:	vfsp	- virtual file system for which we want the root
 *		vpp	- address to return root vnode to
 *		crp	- credential
 *
 * RETURN :	ESTALE if force unmount in progress,
 *		otherwise zero is returned
 *			
 */

jfs_root (vfsp, vpp, crp)
struct vfs	*vfsp;		/* Ptr to virtual file system */
struct vnode	**vpp;		/* Ptr to vnode ptr,ret value */
struct ucred	*crp;		/* pointer to credential structure */
{
	/*
	 * error return if vfs has already been (or, at least, started) 
	 * a force unmount.
	 */
	if (vfsp->vfs_flag & VFS_SHUTDOWN)
		return ESTALE;

	/*
	 * get the root vnode
	 */
	*vpp = vfsp->vfs_mntd;

	assert(*vpp != NULL);

	/*
	 * acquire a reference of the root vnode
	 */
	jfs_hold(*vpp);

	return 0;
}
