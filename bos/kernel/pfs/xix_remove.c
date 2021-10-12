static char sccsid[] = "@(#)28	1.14.1.5  src/bos/kernel/pfs/xix_remove.c, syspfs, bos411, 9428A410j 7/7/94 16:54:29";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_remove
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
#include "sys/dir.h"

/*
 * NAME:	jfs_remove (vp, dvp, pname, crp)
 *
 * FUNCTION:	remove object vp (named by "pname") from directory dvp
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  object we want to remove
 *		dvp	- parent directory of vp
 *		pname 	- name for vp
 *		crp	- credential
 *
 * RETURN :	errors from subroutines
 *			
 */

jfs_remove(vp, dvp, pname, crp)
struct vnode	*vp;		/* To be removed 	*/
struct vnode	*dvp;		/* Its parent		*/
char		*pname;		/* Its name		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int	rc;		/* return code			*/
	struct inode *dip;	/* directory inode		*/
	struct inode *ip;	/* vnode's inode		*/
	dname_t nmp;		/* Name argument		*/
	int	dos;		/* ip != dip ?			*/
	struct inode *ixip = NULL;
	extern struct inode	*sec_delete();
	
	dip = VTOIP(dvp);
	ip = VTOIP(vp);

	/* must be super user to unlink(2) to directories */
	if ((ip->i_mode & IFMT) == IFDIR && privcheck_cr(FS_CONFIG, crp) != 0)
		return EPERM;

	/* Lock directory and file inodes.  
	 * watch for dip == ip when removing directories "by hand".
	 */
	dos = (dip != ip) ? 2 : 1;
	iwritelocklist(dos, dip, ip);

	/* remove and commit
	 */
	nmp.nm = pname;
	nmp.nmlen = strlen(pname);
	if ((rc = dir_delete(dip, &nmp, ip, crp)) == 0)
	{
		ixip = sec_delete(ip); /* return locked .inodex inode */
		if (ixip)
		 	rc = commit(dos+1, ixip, dip, ip);
		else
			rc = commit(dos, dip, ip);
	}

	if (ixip)
		IWRITE_UNLOCK(ixip);

	IWRITE_UNLOCK(ip);

	if (dos == 2)
		IWRITE_UNLOCK(dip);

	RETURNX(rc, reg_elist);
}
