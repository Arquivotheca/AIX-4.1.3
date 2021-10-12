static char sccsid[] = "@(#)71	1.19  src/bos/kernel/lfs/vfs.c, syslfs, bos41J, 9521A_all 5/23/95 08:01:11";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: statfs, fstatfs, fidtovp, vfs_search, vn_search
 *	      vfs_hold, vfs_unhold, ftsearch
 *
 * ORIGINS: 24, 27
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

/* (#)vfs.c	1.5 87/05/19 NFSSRC */
/* (#)vfs.c 1.1 86/09/25 SMI	*/

#include <sys/systm.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/fs_locks.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/var.h>
#include <sys/syspest.h>

/*
 * vfs global data
 */
extern struct vnode *rootdir;		/* pointer to root vnode */
extern struct vfs *rootvfs;		/* pointer to root vfs. This is */
extern Simple_lock vfs_list_lock;

/*
 * System calls
 */

/*
 * get filesystem statistics
 */
statfs(path, buf)
char		*path;
struct statfs	*buf;
{
	struct statfs sbuf;
	struct vnode *vp;
	struct vfs   *vfsp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get current user credentials */
	crp = crref();

	rc = lookupname(path, USR, L_SEARCH, NULL, &vp, crp);
	if (rc == 0)
	{
		bzero(&sbuf,sizeof(sbuf));
		sbuf.f_name_max = 255;		/* XXX - TEMP */

		vfsp = vp->v_vfsp;
		rc = VFS_STATFS(vfsp, &sbuf, crp);
		VNOP_RELE(vp);
		if ( (rc == 0)
		    && copyout(&sbuf,buf,sizeof(sbuf)) )
			rc = EFAULT;
	}

	crfree(crp);
	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);
	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}

fstatfs(fd, buf)
int		fd;
struct statfs	*buf;
{
	struct statfs sbuf;
	struct file *fp;
	struct vnode *vp;
	struct vfs   *vfsp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get the current user credentials */
	crp = crref();

	/* trace file descriptor */
	TRCHKL1T(HKWD_SYSC_FSTATFS, fd);

	rc = getft(fd, &fp, DTYPE_VNODE);
	if (rc == 0)
	{
		vp = (struct vnode *)fp->f_vnode, 
		bzero(&sbuf,sizeof(sbuf));
		sbuf.f_name_max = 255;		/* XXX - TEMP */

		vfsp = vp->v_vfsp;

		/* We have to check if a gfs exists for pipes */
		if (!vp->v_vfsp->vfs_gfs)
			rc = EINVAL;     /* no statfs vfs op */
		else if (!(rc = VFS_STATFS(vp->v_vfsp, &sbuf, crp)) &&
				copyout(&sbuf, buf, sizeof(sbuf)))
			rc = EFAULT;

		ufdrele(fd);
	}

	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}

/*
 * NAME: fidtovp
 *
 * FUNCTION: Used by the server side of distributed file systems. It maps
 *	     fsid and fid argument to an object and sets a pointer to a 
 *	     vnode for that object.  The use count on the vnode is incremented 
 *	     so that the vnode will not be removed.
 *
 * RETURNS:  Possible ERRNO values:
 *	    	ESTALE	The requested file or file system has been removed
 * 			or recreated since the last access with the given
 *			fid/fsid.
 *
 */

int
fidtovp(fsid_t	*fsid,			/* pointer to file system id struct */
	struct	fileid	*fid,		/* pointer to the file id struct    */
	struct	vnode	**vpp,		/* pointer to vnode pointer         */
	struct  ucred   *crp)		/* pointer to credentials           */
{
        struct vfs *vfsp;
        int error=0;
	void	vfs_hold(), vfs_unhold();

	VFS_LIST_LOCK();
        for (vfsp = rootvfs; vfsp; vfsp = vfsp->vfs_next)
	{
                if (vfsp->vfs_flag & VFS_UNMOUNTING)
			continue;
                if (vfsp->vfs_fsid.val[0] == fsid->val[0] &&
                    vfsp->vfs_fsid.val[1] == fsid->val[1])
		{
			vfs_hold(vfsp);
			break;
		}
        }
	VFS_LIST_UNLOCK();

        if (vfsp == NULL)
		error = ESTALE;
	else
	{
        	error = VFS_VGET(vfsp, vpp, fid, crp);
		vfs_unhold(vfsp);
	}
        return error;
}

/*
 * NAME: vfs_search
 *
 * FUNCTION: This routine searchs the entire VFS list by simply following
 *	     the links through the list. We guarantee the integrity of the
 *	     list by holding the vfs_list_lock for the duration of the 
 * 	     search.
 *     
 * RETURNS:  Returns the value returned by the last call to the vfs_srchfcn
 *	     routine.
 *
 */

int
vfs_search(int (*vfs_srchfnc)(), caddr_t vfs_srchargs)
{
	struct	vfs * vfsp;
	int	rc;		/* return of last vfs searched */

	VFS_LIST_LOCK();
	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next)
	{
		if (vfsp->vfs_flag & VFS_UNMOUNTING)
			continue;
		else
		{
			vfs_hold(vfsp);
			rc = (*vfs_srchfnc)(vfsp, vfs_srchargs);
			vfs_unhold(vfsp);
			if (rc)
			{
				break;
			}
		}
	}
	VFS_LIST_UNLOCK();
	return rc;
}

/*
 * NAME: vn_search
 *
 * FUNCTION: Provides a method of scanning the list of vnodes currently
 *	     in use in a vfs. It is assumed that the calling routine has
 *	     a lock on the vfsp passed in. This routine iterates through 
 *	     the vnode list, calling the vn_srchfcn routine for each entry.
 *     
 * RETURNS:  If there are no vnodes found, this routine returns -1.
 *	     Otherwise, it returns the value returned by the last call to 
 *	     vn_srchfcn routine.
 *
 */

int
vn_search(struct vfs *vfsp,
	  int	(*vn_srchfcn)(),
	  caddr_t vn_srchargs)
{
	int	rc;
	struct vnode *vp;
	extern struct galloc gpa_vnode;

	/* 
	 * we assume the vfsp passed is valid, and we should have a
	 * reference count on the vfs.
	 */

	if (vfsp->vfs_vnodes)
	{
		for (vp = vfsp->vfs_vnodes; vp; vp = vp->v_vfsnext)
		{
			rc = (*vn_srchfcn)(vp, vn_srchargs);
			if (rc)
				break;
		}
	}
	else
		rc = gpai_srch(&gpa_vnode, vfsp,
			       (int)&((struct vnode *)0)->v_vfsp,
			       vn_srchfcn, vn_srchargs);
	return rc;
}

/*
 * NAME: vfs_hold
 *
 * FUNCTION: Provides synchronization for incrementing of the 
 *	     use count on the specified vfs.
 *     
 * RETURNS:  NONE
 *
 *
 */

void
vfs_hold(struct vfs *vfsp)	/* pointer to the vfs structure */
{
	fetch_and_add(&vfsp->vfs_count, 1);
}

/*
 * NAME: vfs_unhold
 *
 * FUNCTION: Provides synchronization for decrementing of the 
 *	     use count on the specified vfs.
 *     
 * RETURNS: NONE
 *
 */

void
vfs_unhold(struct vfs *vfsp)	/* pointer to the vfs structure */
{
	assert(vfsp->vfs_count > 0);
	fetch_and_add(&vfsp->vfs_count, -1);
}

/*
 * NAME: ftsearch
 *
 * FUNCTION: This routine searches the file table. This routine will
 *	     hold the FFREE_LOCK for the duration of the file table
 *	     search.  Calls to this function will incur severe
 *	     performance penalty since no files can be opened or closed
 *	     for the duration of the file table search.
 *
 * RETURN VALUES: Returns the value returned by the last call to the 
 *		  ftecheck routine.
 *
 */

int
ftsearch(int (*ftecheck) (struct file *fp, void *checkarg),
	 int   type,
	 void *checkarg)
{
	int		rc = 0;		/* return code from function call */
	struct file	*fp;		/* file pointer for file table entry */
	
	FFREE_LOCK();
	for (fp = &file[0]; fp < &file[v.v_file-1]; fp++)
	{
		if (fp->f_count != 0 && (type == 0 || fp->f_type == type))
		{
			FP_LOCK(fp);
			if (fp->f_count != 0 &&
			    (type == 0 || fp->f_type == type))
				rc = (*ftecheck)(fp, checkarg);
			FP_UNLOCK(fp);
			if (rc != 0)
				break;
		}
	}
	FFREE_UNLOCK();
	return rc;
}
