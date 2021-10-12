static char sccsid[] = "@(#)20	1.18.1.9  src/bos/kernel/pfs/xix_gattr.c, syspfs, bos411, 9428A410j 7/7/94 16:53:37";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_getattr
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
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/vattr.h"
#include "sys/utsname.h"

void	get_vattr(struct inode *, struct vattr *);

/*
 * NAME:	jfs_getattr (vp, vattrp, crp)
 *
 * FUNCTION:	Return object attributes in fs independent format.
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  to stat
 *		vattrp	- returned info
 *		crp	- credential
 *
 * RETURN :	Always zero 
 */

jfs_getattr(vp, vattrp, crp)
struct vnode	*vp;
struct vattr	*vattrp;
struct ucred	*crp;		/* pointer to credential structure */
{
	struct inode *ip;

	ip = VTOIP(vp);
	IREAD_LOCK(ip);

	get_vattr(ip, vattrp);

	IREAD_UNLOCK(ip);
	return 0;
}


/*
 * NAME:	get_vattr(ip, vattrp)
 *
 * FUNCTION:	Return object attributes 
 *
 * PARAMETERS:	ip	- pointer to inode of the file to stat.
 *
 *		vattrp	- returned info
 *
 * RETURN :	void
 *
 */

void
get_vattr(struct inode		*ip,
	 struct vattr		*vattrp)
{
	INODE_LOCK(ip);
	vattrp->va_atime = ip->i_atime_ts;
	vattrp->va_mtime = ip->i_mtime_ts;
	vattrp->va_ctime = ip->i_ctime_ts;
	INODE_UNLOCK(ip);

	vattrp->va_dev = brdev(ip->i_dev);
	vattrp->va_serialno = ip->i_number;
	vattrp->va_size = ip->i_size;
	vattrp->va_type = IFTOVT(ip->i_mode);
	vattrp->va_blocks = ip->i_nblocks *
			(PAGESIZE / ip->i_ipmnt->i_fperpage) / DEV_BSIZE;
	vattrp->va_blocksize = PAGESIZE;	/* XXX. FsBSIZE() */

	vattrp->va_mode = ip->i_mode;
	vattrp->va_nlink = ip->i_nlink;
	vattrp->va_rdev = (dev_t)ip->i_rdev;
	vattrp->va_nid = xutsname.nid;	
	vattrp->va_uid = ip->i_uid;
	vattrp->va_gid = ip->i_gid;
	vattrp->va_chan = ip->i_gnode.gn_chan;
	vattrp->va_gen = ip->i_gen;
}
