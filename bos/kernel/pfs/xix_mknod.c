static char sccsid[] = "@(#)21	1.15.1.5  src/bos/kernel/pfs/xix_mknod.c, syspfs, bos411, 9433B411a 8/18/94 09:22:08";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_mknod
 *
 * ORIGINS: 3, 27
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
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/dir.h"

/*
 * NAME:	jfs_mknod(dvp, pname, mode, dev, crp)
 *
 * FUNCTION:	Make a new object in dvp with mode == mode and name "pname"
 *		and rdev == dev for special files.
 *
 * PARAMETERS:	dvp 	- is the pointer to the vnode that represents the 
 *			  directory where the new directory will reside.  
 *		pname	- name of new directory
 *		mode	- create mode (rwxrwxrwx).
 *		dev	- new device number if special file
 *		crp	- credential
 *
 * RETURN :	Errors from subroutines
 *			
 */

jfs_mknod(dvp, pname, mode, dev, crp)
struct vnode	*dvp;
caddr_t		pname;
int		mode;
dev_t		dev;
struct ucred	*crp;		/* pointer to credential structure */
{
	int 	rc;		/* Return code				*/
	struct inode *dip;	/* Directory inode			*/
	struct inode *ip;	/* Returned inode			*/
	dname_t nmp;		/* Name argument			*/
	struct vfs	*vfsp;

	/* Can I do this? */
	if ((mode & IFMT) == IFDIR)
		return EISDIR;
		
	if ((mode & IFMT) != IFIFO && (mode & IFMT) != IFSOCK)
		if (rc = privcheck_cr(DEV_CONFIG, crp))
			return rc;

	vfsp = dvp->v_vfsp;
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfsp = NULL;

	dip = VTOIP(dvp);

	IWRITE_LOCK(dip);

	/* create node and commit
	 * (dir_mknod() returns the new locked, referenced inode)
	 */
	nmp.nm = pname;
	nmp.nmlen = strlen(pname);
	if ((rc = dir_mknod(dip, &nmp, mode, &ip, vfsp, crp)) == 0)
	{
		switch (mode & IFMT)
		{
			case IFCHR:
			case IFBLK:
				ip->i_rdev = dev;
				ITOGP(ip)->gn_rdev = dev;
				imark(ip, ICHG);
		}
		rc = commit(2, dip, ip);
	}

	IWRITE_UNLOCK(dip);

	if (ip)
	{
		IWRITE_UNLOCK(ip);

		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
	}

	RETURNX(rc, reg_elist);
}
