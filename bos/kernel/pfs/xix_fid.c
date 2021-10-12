static char sccsid[] = "@(#)10	1.9  src/bos/kernel/pfs/xix_fid.c, syspfs, bos411, 9428A410j 7/7/94 16:53:29";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_fid
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/vfs.h"
#include "jfs/inode.h"

/*
 * NAME:	jfs_fid (vp, fidp, crp)
 *
 * FUNCTION: 	jfs_fid() modifies the file identifier pointed to by the fhp.
 * 		Information pointed to by the vnode private data area(v_pdata)
 * 		(really the inode) is used to construct a unique file identifier
 * 		for the aix file system.
 *
 * PARAMETERS:	vp	- Pointer to the vnode that represents the object
 *			  for which a file handle is to be constructed.
 *		fidp	- Returned file handle
 *		crp	- credential
 *
 * RETURN:	Always zero
 *			
 */

jfs_fid(vp, fidp, crp)
struct vnode	*vp;
struct fileid	*fidp;
struct ucred	*crp;		/* pointer to credential structure */
{
	struct inode *ip;		/* Inode for vp			*/
	
	ip = VTOIP(vp);

	bzero(fidp, sizeof (*fidp));
	fidp->fid_len  = MAXFIDSZ;
	fidp->fid_ino = ip->i_number;
	fidp->fid_gen = ip->i_gen;

	return 0;
}
