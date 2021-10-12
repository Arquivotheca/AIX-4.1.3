static char sccsid[] = "@(#)25	1.10.1.4  src/bos/kernel/pfs/xix_rdlnk.c, syspfs, bos411, 9428A410j 7/7/94 16:54:19";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_readlink
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
#include "sys/errno.h"
#include "sys/file.h"

/*
 * NAME:	jfs_readlink (vp, uiop, crp)
 *
 * FUNCTION:	Read a symbolic link
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  symlink we want to read
 *		uiop	- How much to read and where it goes
 *		crp	- credential
 *
 * RETURN :	EINVAL	- if not a symbolic link
 *		errors from subroutines
 *			
 */

jfs_readlink(vp, uiop, crp)
struct vnode	*vp;		/* VLNK vnode			*/
struct uio	*uiop;		/* Uio iformation		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int	rc;			/* Return code			*/
	struct inode *ip;		/* inode for vp			*/

	if (vp->v_vntype != VLNK)
		return EINVAL;

	ip = VTOIP(vp);
	IREAD_LOCK(ip);

	/* AES requires ERANGE if the link name won't fit */
	if (ip->i_size > uiop->uio_resid)
	{
		rc = ERANGE;
		goto out; 
	}

	/* If name resides in the disk inode copy it from there
	 */

	if (ip->i_size <= sizeof(ip->i_symlink))
	{
		int cnt;

		cnt = MIN(ip->i_size, uiop->uio_iov->iov_len);
		rc = uiomove(ip->i_symlink, cnt, UIO_READ, uiop);
		if (rc == 0)
		{
			INODE_LOCK(ip);
			imark(ip, IACC);
			INODE_UNLOCK(ip);
		}
	}
	else
		rc = readi(ip, FREAD, 0, uiop);

out:
	IREAD_UNLOCK(ip);

	return rc;
}
