static char sccsid[] = "@(#)30	1.26  src/bos/kernel/pfs/xix_rmdir.c, syspfs, bos411, 9434A411a 8/19/94 09:12:03";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_rmdir
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
 * i know this is somewhere else, but it's a major hassle to add trivial
 * stuff like this to header files...  major re-makes, etc...
 */
#define	ISDOTDOT(name)		((name)[0] == '.' && (name)[1] == '.' \
				&& (name)[2] == '\0')
/*
 * NAME:	jfs_rmdir (vp, dvp, pname, crp)
 *
 * FUNCTION:	Remove directory vp (named "pname") from dvp
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  object we want to remove
 *		dvp	- parent directory of vp
 *		pname	- name of vp within dvp
 *		crp	- credential
 *
 * RETURN :	EINVAL	- if name is . or ..
 *		EINVAL  - if . or .. exist but are invalid.
 *		errors from subroutines
 */

jfs_rmdir(vp, dvp, pname, crp)
struct vnode	*vp;
struct vnode	*dvp;
char		*pname;
struct ucred	*crp;		/* pointer to credential structure */
{
	int 	rc;		/* Return code				*/
	struct inode *dip;	/* Directory inode			*/
	struct inode *ip;	/* Returned inode			*/
	dname_t nmp;		/* Name argument			*/
	int	same;		/* Is dip == ip ? The same inode        */
	int	dots = 0;	/* status of dot and dot dot entries	*/
	
	/*
	 * fail EEXIST if ".." ...
	 */
	if (ISDOTDOT(pname))
		return EEXIST;

	/*
	 * sorry, but we don't allow rmdir (".") either...
	 */
	else if (ISDOTS(pname))
		return EINVAL;

	dip = VTOIP(dvp);
	ip = VTOIP(vp);

	/* Lock directory and file inodes.  When removing directories
	 * "by hand" dip may == ip, watch for it
	 */
	same = (dip != ip) ? 2 : 1;

	/* Lock inodes */
	iwritelocklist(same, dip, ip);

	/*
	 * make sure the directory to rmdir is already empty.
	 */
    	if ((rc = dir_valid(ip, dip->i_number, 1, &dots)) == 0)
	{
		/*
		 * If the destination has '.' or '..' entries they
		 * must be valid.
		 */
		if (((dots & DOT_EXIST) &&  !(dots & DOT_VALID)) ||
		    ((dots & DDOT_EXIST) && !(dots & DDOT_VALID)))
		{
			rc = EINVAL;
			goto unlock_out;
		}

		nmp.nm = pname;
		nmp.nmlen = strlen(pname);

		/*
		 *	delete the directory entry
		 *	set link count of target to 0
		 *	decrement parent's link count
		 *	commit and then truncate target
		 *      so that operation is atomic and 
		 *      directory is completely empty
		 *      (i.e. "." and ".." are gone.)
		 */
		if ((rc = dir_delete(dip, &nmp, ip, crp)) == 0) {
			ip->i_nlink = 0;
			if (DDOT_STAT(dots) & DDOT_EXIST)
				dip->i_nlink--;
			if ((rc = commit(2, dip, ip)) == 0)
				itrunc(ip,0,crp);
		}
	}

unlock_out:
	IWRITE_UNLOCK(ip);
	IWRITE_UNLOCK(dip);

	RETURNX(rc, reg_elist);
}
