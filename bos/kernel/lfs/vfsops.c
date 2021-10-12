static char sccsid[] = "@(#)66	1.4  src/bos/kernel/lfs/vfsops.c, syslfs, bos411, 9428A410j 2/24/94 09:28:37";
/*
 *   COMPONENT_NAME:	SYSLFS
 *
 *   FUNCTIONS: vfs_cntl
 *		vfs_mount
 *		vfs_quotactl
 *		vfs_root
 *		vfs_statfs
 *		vfs_sync
 *		vfs_unmount
 *		vfs_vget
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/gfs.h>
#include <sys/vfs.h>
#include <sys/vnode.h>

#ifdef	_FSDEBUG
/*
 * This structure contains counts of the number of times
 * each vfs operation is called.
 */
struct {
	int cntl;	/* 0 */
	int mount;	/* 1 */
	int quotactl;	/* 2 */
	int root;	/* 3 */
	int statfs;	/* 4 */
	int sync;	/* 5 */
	int unmount;	/* 6 */
	int vget;	/* 7 */
} vfsop_cnt;

#define VFSOP_CNT(OP)	fetch_and_add(&vfsop_cnt.OP, 1)
#else	/* _FSDEBUG */
#define VFSOP_CNT(OP)
#endif	/* _FSDEBUG */

/*
 * NAME:	vfs_mountvfsp
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_mount(struct vfs *vfsp,
	  struct ucred *crp)
{
	VFSOP_CNT(mount);
	return (*vfsp->vfs_ops->vfs_mount)(vfsp,crp);
}

/*
 * NAME:	vfs_unmount
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_unmount(struct vfs *vfsp,
	    int	flg,
	    struct ucred *crp)
{
	VFSOP_CNT(unmount);
	return (*vfsp->vfs_ops->vfs_unmount)(vfsp,flg,crp);
}

/*
 * NAME:	vfs_root
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_root(struct vfs *vfsp,	/* Ptr to virtual file system */
         struct vnode **vpp,	/* Ptr to vnode ptr,ret value */
	 struct ucred *crp)
{
	VFSOP_CNT(root);
	return (*vfsp->vfs_ops->vfs_root)(vfsp,vpp,crp);
}

/*
 * NAME:	vfs_statfs
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_statfs(register struct vfs *vfsp,	/* Ptr to virtual file system */
	   register struct statfs *sbp,
	   struct ucred *crp)
{
	VFSOP_CNT(statfs);
	return (*vfsp->vfs_ops->vfs_statfs)(vfsp,sbp,crp);
}

/*
 * NAME:	vfs_sync
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_sync(struct	gfs *gfsp)
{
	VFSOP_CNT(sync);
	return (*(gfsp)->gfs_ops->vfs_sync)();
}

/*
 * NAME:	vfs_vget
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_vget(struct vfs *vfsp,              /* Pointer to virtual file system */
	 struct vnode **vpp,            /* Pointer to vnode pointer,ret val */
	 struct fileid *fidp,           /* Pointer to file handle */
	 struct ucred *crp)
{
	VFSOP_CNT(vget);
	return (*vfsp->vfs_ops->vfs_vget)(vfsp,vpp,fidp,crp);
}

/*
 * NAME:	vfs_cntl
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int
vfs_cntl(struct vfs *vfsp,
	 int cmd,
	 caddr_t arg,
	 size_t argsiz,
	 struct ucred *crp)
{
	VFSOP_CNT(cntl);
	return (*vfsp->vfs_ops->vfs_cntl)(vfsp,cmd,arg,argsiz,crp);
}

/*
 * NAME:	vfs_quotactl
 *
 * RETURNS:	The return code from the underlying file system operation
 */
int 
vfs_quotactl(struct vfs *vfsp,
	     int cmd,
	     uid_t id,
	     caddr_t arg,
	     struct ucred *crp)
{
	VFSOP_CNT(quotactl);
	return (*vfsp->vfs_ops->vfs_quotactl)(vfsp,cmd,id,arg,crp);
}
