static char sccsid[] = "@(#)37	1.14.1.6  src/bos/kernel/pfs/xix_symlnk.c, syspfs, bos411, 9433B411a 8/18/94 09:22:17";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_symlink
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "jfs/commit.h"
#include "sys/vfs.h"
#include "sys/dir.h"
#include "sys/uio.h"
#include "sys/file.h"

/*
 * NAME:	jfs_symlink(dvp, lnm, tnm, crp)
 *
 * FUNCTION:	creates a symbolic link (type IFLNK) in the file system.
 *		The data blocks of the inode contain the path name.
 *
 * PARAMETERS:	Dvp	- directory vnode pointer where symlink is to
 *			  be created.
 *		lnm 	- the name of the new link
 *		tnm 	- the name of the existing object that will be 
 *			  the target of the link
 *		crp	- credential
 *
 * RETURN :	errors from subroutines
 */

jfs_symlink(dvp, lnm, tnm, crp)
struct vnode	*dvp;
char		*lnm;
char		*tnm;
struct ucred	*crp;		/* pointer to credential structure */
{
	int		rc;	/* Return code				*/
	struct inode 	*dip;	/* Directory inode			*/
	struct inode 	*ip;	/* Returned inode			*/
	struct iovec iov;
	struct uio   uio;
	dname_t nmp;		/* Name arument				*/
	int		tsz;	/* Target name size			*/
	struct vfs	*vfsp;

	vfsp = dvp->v_vfsp;
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfsp = NULL;

	dip = VTOIP(dvp);

	IWRITE_LOCK(dip);

	/* create and commit
	 */
	nmp.nm = lnm;
	nmp.nmlen = strlen(lnm);
	if ((rc = dir_create(dip, &nmp, IFLNK | 0777, &ip, vfsp, crp)) == 0)
	{

		/* dir_create() returns the new locked, referenced inode
		 */

		/* dir_create ANDs in the u.u_cmask, but sym links
		 * really need to be 777 access. Fix that here.
		 */
		ip->i_mode |= 0777;
		imark(ip, IFSYNC|ICHG|IUPD);

		/* If symbolic link will fit in disk inode,
		 * put it there else put it in its own disk block
		 */
		tsz = strlen(tnm);
		if (tsz <= sizeof(ip->i_symlink))
		{	
			/* Copy name to disk inode and set both
			 * disk inode size and incore inode size.
			 */
			bzero(ip->i_symlink, sizeof(ip->i_symlink));
			bcopy(tnm, ip->i_symlink, tsz);
			ip->i_size = tsz;
		}
		else
		{
			/* set up iovector */
			iov.iov_base = tnm;
			iov.iov_len = tsz;

			/* set up uio struct */
			uio.uio_iov = &iov;
			uio.uio_iovcnt = 1;
			uio.uio_offset = 0;
			uio.uio_segflg = UIO_SYSSPACE;
			uio.uio_resid = tsz;

			if (rc = writei(ip, FWRITE, 0, &uio, crp))
				(void) dir_delete(dip, &nmp, ip, crp);
		}

		/* if writei failed (EIO or ENOSPC) then don't commit
		 * the symlink inode. It has a link count of zero (by
		 * dir_delete()) and no permanant resources.
		 * iput() at exit will free its working resource and
		 * the inode.
		 */
		if (rc == 0)
			rc = commit(2, ip, dip);
		else
			(void) commit(1, dip);

		IWRITE_UNLOCK(ip);
	}

	IWRITE_UNLOCK(dip);

	if (ip)
	{
		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
	}

	RETURNX(rc, reg_elist);
}
