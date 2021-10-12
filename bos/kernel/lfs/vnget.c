static char sccsid[] = "@(#)74	1.7.1.8  src/bos/kernel/lfs/vnget.c, syslfs, bos411, 9439C411d 9/30/94 17:07:17";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: vn_get, vn_free
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
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/inode.h"
#include "sys/gpai.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/lock_def.h"
#include "sys/fs_locks.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"

extern struct galloc gpa_vnode;

/*
 *	vn_get()
 *
 * allocates vnode structure.
 *
 * The gnode is assumed to be serialized by the caller.
 *
 */
vn_get (vfsp, gp, vpp)
struct vfs	*vfsp;
struct gnode *gp;
struct vnode **vpp;
{
	struct vnode	*vp;
	int rc;

	/* allocate a vnode and link to gnode vnode list
	 */
	if (vp = (struct vnode *)gpai_alloc(&gpa_vnode))
	{	
		vp->v_flag = 0;
		vp->v_count = 1;
		vp->v_audit = NULL;
		vp->v_vfsp = vfsp;		/* Assign vfs pointer */
		vp->v_vfsgen = 0;
		vp->v_mvfsp = NULL;
		vp->v_gnode = gp;		/* Assign gnode pointer */

		/* if there is a vnode on the gnode vnodelist, then
		 * chances are its a JFS softmount base vnode, so insert 
		 * the vnode into gnode vnodelist AFTER the base vnode;
		 * otherwise, insert at head
		 */
		if (gp->gn_vnode)
		{
			vp->v_next = gp->gn_vnode->v_next;
			gp->gn_vnode->v_next = vp;
		}
		else
		{
			vp->v_next = gp->gn_vnode;
			gp->gn_vnode = vp;
		}

		*vpp = vp;
		/*
		 * for soft mount vfs types the vfs_count
		 * represents the number of active vnodes on
		 * this vfs (* including the root vnode *). 
		 * Therefore, whenever we add another soft mount
		 * vnode, we must increment the vfs_count on the
		 * vfs.
		 */
		if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
			vfs_hold(vfsp);
		rc = 0;
	}
	else
	{
		*vpp = NULL;
		rc = ENOMEM;
	}

	return rc;
}

/*
 *	vn_free()
 *
 * frees previously allocated vnode structure.  
 * No known errors to return.
 *
 * The gnode is assumed to be serialized by the caller.
 */
vn_free(vp)
struct vnode *vp;
{
	struct gnode	*gp = VTOGP(vp);
	struct vnode	*tvp;
	struct vfs	*vfsp = vp->v_vfsp;

	tvp = gp->gn_vnode;		/* Delink vp vnode list */
	if (tvp == vp)
		gp->gn_vnode = vp->v_next;
	else
	{
		for (; tvp != NULL; tvp = tvp->v_next)
			if (tvp->v_next == vp)
				break;
		if (tvp == NULL)
			panic("vn_free: vp not in gnode list");
		tvp->v_next = vp->v_next;
	}

	/*
	 * the vfs_count field in the vfs specifies the
	 * number of active vnode (including the root vnode)
	 * in the vfs. When we free a vnode, we must decrement
	 * the vfs_count.
	 */
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfs_unhold(vfsp);

	/* Put the vnode back in the free pool. */
	gpai_free (&gpa_vnode, vp);

	return(0);
}
