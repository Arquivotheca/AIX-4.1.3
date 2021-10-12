static char sccsid[] = "@(#)41	1.9.1.6  src/bos/kernel/pfs/xix_vget.c, syspfs, bos41J, 9507C 2/15/95 09:35:47";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_vget
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/user.h"
#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/errno.h"
#include "sys/syspest.h"

/*
 * NAME:	jfs_vget(vfsp, vpp, fidp, crp)
 *
 * FUNCTION:	return a pointer to an existing or newly created vnode. 
 * 		File id is decoded to yield _iget() arguments.  
 *		Resulting inode is used to create a vnode.
 *
 * PARAMETERS:	vfsp	- vfs where requested vnode should exist
 *		vpp	- returned vnode
 *		fidp	- file identifier
 *		crp	- credential
 *
 * RETURN :	errors from subroutines
 *			
 */

jfs_vget(vfsp, vpp, fidp, crp)
struct vfs	*vfsp;		/* Pointer to virtual file system */
struct vnode	**vpp;		/* Pointer to vnode pointer,ret val */
struct fileid	*fidp;		/* Pointer to file handle */
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;	
	struct inode *ip;
	struct vfs *nvfsp = NULL;
	struct hinode *hip;

	*vpp = NULL;

	if (vfsp->vfs_flag & VFS_DEVMOUNT)
		nvfsp = vfsp;

	IHASH(fidp->fid_ino, vfsp->vfs_fsid.fsid_dev, hip);

	/* get the inode.
	 */
	ICACHE_LOCK();

	/* increment the vget count on the hash so dev_ialloc()s on this
	 * hash will call _iget() with the doscan flag set.
	 */
	hip->hi_vget++;

	if (rc = _iget(vfsp->vfs_fsid.fsid_dev,fidp->fid_ino,hip,&ip,1,nvfsp))
	{
		hip->hi_vget--;
		ICACHE_UNLOCK();
		return rc;
	}

	/* Check for request for old (deleted) file with different
	 * gen number or unlinked file with same gen number, which
	 * is possible after gen is reset in re-install.
	 */
	if (fidp->fid_gen != ip->i_gen)
	{
		iput(ip, nvfsp);
		hip->hi_vget--;
		ICACHE_UNLOCK();

		return ESTALE;
	}
	
	hip->hi_vget--;
	ICACHE_UNLOCK();

	/* return the vnode with a reference.
	 */
	if (nvfsp)
		*vpp = ip->i_gnode.gn_vnode;
	else
		rc = iptovp(vfsp, ip, vpp);

	return rc;
}
