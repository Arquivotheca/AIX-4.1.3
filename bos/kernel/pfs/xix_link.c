static char sccsid[] = "@(#)17	1.14.1.5  src/bos/kernel/pfs/xix_link.c, syspfs, bos411, 9428A410j 7/7/94 16:53:50";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_link
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
 * NAME:	jfs_link (vp, dvp, name, crp)
 *
 * FUNCTION:	Create link to vp in directory dvp by the name "name".
 *
 * PARAMETERS:	vp 	- is the pointer to the vnode that represents the file
 *			  to link to
 *		dvp	- directory in which name is to be created
 *		name	- name of new link
 *		crp	- credential
 *
 * RETURN :	Errors from subroutines
 *			
 */

jfs_link(vp, dvp, name, crp)
struct vnode	*vp;
struct vnode	*dvp;
char		*name;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;			/* Return code			*/
	struct inode *dip;	/* Directory inode for dir vnode	*/
	struct inode *ip;	/* inode for vnode		*/
	dname_t nmp;		/* Name argument */
	int	dos;		/* ip != dip ? */

	dip = VTOIP(dvp);
	ip = VTOIP(vp);

	/* Cross device link */
	if (ip->i_dev != dip->i_dev)
		return EXDEV;

	/* must be super user to link(2) to directories */
	if ((ip->i_mode & IFMT) == IFDIR && privcheck_cr(FS_CONFIG, crp) != 0)
		return EPERM;

	/* Lock directory and file inodes.  
	 * watch for dip == ip when linking directories "by hand".
	 */
	dos = (dip != ip) ? 2 : 1;
	iwritelocklist(dos, dip, ip);

	/* create link and commit
	 */
	nmp.nm = name;
	nmp.nmlen = strlen(name);
	rc = dir_link(dip, &nmp, ip, crp);
	if (rc == 0)
		rc = commit(dos, dip, ip);

	IWRITE_UNLOCK(ip);
	if (dos == 2)
		IWRITE_UNLOCK(dip);

	RETURNX(rc, reg_elist);
}
