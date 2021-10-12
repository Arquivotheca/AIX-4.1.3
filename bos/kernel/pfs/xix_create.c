static char sccsid[] = "@(#)06	1.11.1.6  src/bos/kernel/pfs/xix_create.c, syspfs, bos411, 9433B411a 8/18/94 09:21:49";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_create
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
#include "sys/file.h"
#include "sys/syspest.h"

/*
 * NAME:	jfs_create (dvp, vpp, flag, pname, mode, vinfop, crp)
 *
 * FUNCTION:	Create and open a file of the given name, in the given
 *		directory.
 *
 * PARAMETERS:	dvp	- Parent vnode where "pname" is to be created.
 *		vpp	- Returned vnode
 *		flag	- open flags from the file pointer.
 *		pname	- Name of new file
 *		mode	- rwx permission for the new file
 *		vinfo	- network information for this file 
 *			  (from the file pointer).
 *		crp	- credential
 *
 * RETURNS:	Returns any error codes, and passes back through vpp the
 *		new vnode for the file.
 */

jfs_create (dvp, vpp, flag, pname, mode, vinfop, crp)
struct vnode	*dvp;		/* parent directory vnode pointer	*/
struct vnode	**vpp;		/* new vnode 				*/
int		flag;		/* flag from open file pointer		*/
caddr_t		pname;		/* contains the name of the new file	*/
int		mode;		/* contains the permission modes for the file */
caddr_t		*vinfop;	/* the vinfo from file pointer		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;			/* return code				*/
	struct inode *dip;	/* Parent inode				*/
	struct inode *ip;	/* inode returned from dir_create	*/
	dname_t nmp;		/* Name arument				*/
	struct vfs	*vfsp;

	*vpp = NULL;

	vfsp = dvp->v_vfsp;
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfsp = NULL;

	dip = VTOIP(dvp);

	IWRITE_LOCK(dip);

	/* create and commit
	 */
	nmp.nm = pname;
	nmp.nmlen = strlen(pname);
	rc = dir_create(dip, &nmp, IFREG|mode, &ip, vfsp, crp);
	switch (rc) {
		case 0:
			/* dir_create() returns the new locked, referenced inode
			 */
			rc = commit(2, dip, ip);

			IWRITE_UNLOCK(dip);

			if (rc)
				goto bad;

			flag &= ~FTRUNC;
			break;

		case EEXIST:
			/* dir_create() returns the existing, referenced inode
			 * WITHOUT being locked to prevent deadlock as multiple 
			 * inode locking should follow lock odering based on
			 * i_number order. (* i.e. jfs_remove *)
			 */
			IWRITE_UNLOCK(dip);

			IWRITE_LOCK(ip);

			if ((flag & FEXCL) || (rc = ip_access(ip, flag, crp)))
				goto bad;

			flag &= ~FCREAT;
			break;

		default:
			IWRITE_UNLOCK(dip);

			assert(ip == NULL);
			goto bad;
	}

	/* open the inode 
	 */
	if (rc = ip_open(ip, flag, 0, NULL, crp))
		goto bad;

	IWRITE_UNLOCK(ip);

	/* get vnode for the inode. 
	 */
	if (vfsp)
		*vpp = ip->i_gnode.gn_vnode;
	else
		rc = iptovp(dvp->v_vfsp, ip, vpp);

	RETURNX(rc, reg_elist);

bad:
	if (ip)
	{
		IWRITE_UNLOCK(ip);

		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
	}

	RETURNX(rc, reg_elist);
}
