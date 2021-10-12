static char sccsid[] = "@(#)20	1.10.1.4  src/bos/kernel/pfs/xix_mkdir.c, syspfs, bos411, 9433B411a 8/18/94 09:22:02";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_mkdir
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
 * NAME:	jfs_mkdir(dvp, pname, mode, crp)
 *
 * FUNCTION:	Make a new directory in dvp with mode == mode and name "pname"
 *
 * PARAMETERS:	dvp 	- is the pointer to the vnode that represents the 
 *			  directory where the new directory will reside.  
 *		pname	- name of new directory
 *		mode	- create mode (rwxrwxrwx).
 *		crp	- credential
 *
 * RETURN :	Errors from subroutines
 *			
 */

jfs_mkdir(dvp, pname, mode, crp)
struct vnode	*dvp;
char		*pname;
int		mode;
struct ucred	*crp;		/* pointer to credential structure */
{
	int 	rc;		/* Return code				*/
	struct inode *dip;	/* Directory inode			*/
	struct inode *ip;	/* Returned inode			*/
	dname_t nmp;		/* Name arument				*/
	struct vfs	*vfsp;
	
	dip = VTOIP(dvp);
	vfsp = dvp->v_vfsp;
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfsp = NULL;

	IWRITE_LOCK(dip);

	/* create directory and commit
	 * (dir_mkdir() returns the new locked, referenced inode)
	 */
	nmp.nm = pname;
	nmp.nmlen = strlen(pname);
	if ((rc = dir_mkdir(dip, &nmp, mode|IFDIR, &ip, vfsp, crp)) == 0)
		rc = commit(2, dip, ip);

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
