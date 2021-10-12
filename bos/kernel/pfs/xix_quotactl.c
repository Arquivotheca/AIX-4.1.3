static char sccsid[] = "@(#)92	1.2.1.5  src/bos/kernel/pfs/xix_quotactl.c, syspfs, bos411, 9428A410j 7/7/94 16:54:16";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_quotactl
 *
 * ORIGINS: 3, 26, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/types.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"

/* Fs specific includes
 */
#include "jfs/jfsmount.h"
#include "jfs/quota.h"

/*
 * NAME:	jfs_quotactl(vfsp, cmds, uid, arg, crp)
 *
 * FUNCTION:	implement operations associated with disk quotas
 *
 * PARAMETERS:  vfsp	- virtual file system pointer
 *		cmds	- quota operation to be performed and quota type
 *		id	- id associated with operation (ignored by some ops)
 *		arg	- interpreted by operation (ignored by some ops)
 *		crp	- credential
 *
 * RETURN:	0	- success
 *		EINVAL	- invalid quota operation
 *		errors returned by subroutines.
 *			
 */

int
jfs_quotactl(vfsp, cmds, uid, arg, crp)
struct vfs	*vfsp;
int		cmds;
uid_t		uid;
caddr_t		arg;
struct ucred	*crp;		/* pointer to credential structure */
{
	int operation, type, rc;
	struct vnode *vp = NULL;
	struct jfsmount *jmp;
	uid_t ruid;

	/* must be a device mount. 
	 */
	if (vfsp->vfs_data == NULL)
		return (ENOTBLK);

	/* determine quota type.
	 */
	type = cmds & SUBCMDMASK;
	if ((u_int)type >= MAXQUOTAS)
		return (EINVAL);

	if (type == USRQUOTA)
		ruid = crp->cr_ruid;
	else
		ruid = (uid_t)crp->cr_rgid;

	/* BSD compatability.
	 */
	if (uid == -1)
		uid = ruid;

	/* get quota operation.
	 */
	operation = cmds >> SUBCMDSHIFT;

	/* check privilages.
	 */
	switch (operation) {

		/* no privilage required for qsync.
		 */
		case Q_SYNC:
			break;

		/* no privilage required for getquota if
		 * asking about current id.
		 */
		case Q_GETQUOTA:
			if (uid == ruid)
				break;
			/* fall through */

		default:
			/* check for super user.
		 	 */
			if (privcheck_cr(FS_CONFIG, crp))
		 		return(EPERM);
		}

	/* get mount structure for the file system.
	 */
	vp = vfsp->vfs_mntd;
        jmp = VTOIP(vp)->i_ipmnt->i_jmpmnt;
	assert(jmp);

	/* perform quota operation.
	 */
	switch (operation) {

		case Q_QUOTAON:
			rc = quotaon(jmp, type, arg, crp);
			break;

		case Q_QUOTAOFF:
			rc = quotaoff(jmp, type);
			break;

		case Q_SETQUOTA:
			rc = setquota(jmp, uid, type, arg);
			break;

		case Q_SETUSE:
			rc = setuse(jmp, uid, type, arg);
			break;

		case Q_GETQUOTA:
			rc = getquota(jmp, uid, type, arg);
			break;

		case Q_SYNC:
			rc = qsync(jmp);
			break;

		default:
			rc = EINVAL;
		}

	return(rc);
}
