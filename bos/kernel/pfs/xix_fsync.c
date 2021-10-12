static char sccsid[] = "@(#)11	1.12  src/bos/kernel/pfs/xix_fsync.c, syspfs, bos411, 9428A410j 7/7/94 16:53:32";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_fsync
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
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

/*
 * NAME:	jfs_fsync(vp, flags, vinfo, crp)
 *
 * FUNCTION:	Sync local file.
 *
 * PARAMETERS:	vp	- is the pointer to the vnode that represents the file
 *			  to be written to disk.
 *		flags	- open flags 
 *		vinfo	- file descriptor for mapped file
 *		crp	- credential
 *
 *
 * RETURN:	Zero is returned if fsync completes sucessfully.
 *		if an error occurrs an error code from errno.h is
 *		returned.
 *			
 */

jfs_fsync(vp, flags, vinfo, crp)
struct vnode	*vp;      	/* vnode pointer                */
int		flags;		/* open flags			*/
int		vinfo;		/* fd used for mapped files     */
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;			/* return code			*/
	struct inode *ip;

	ip = VTOIP(vp);                 

	IWRITE_LOCK(ip);

	ip->i_flag |= IFSYNC;
	rc = commit(1, ip);		

	IWRITE_UNLOCK(ip);

	RETURNX (rc, reg_elist);
}
