static char sccsid[] = "@(#)89	1.5  src/bos/kernel/lfs/quotactl.c, syslfs, bos411, 9428A410j 10/22/93 17:31:17";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: quotactl
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "sys/systm.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"

/*
 * System call
 */

/*
 * perform disk quota operations.
 */
quotactl(path, cmd, id, addr)
char		*path;
int		cmd;
int		id;
caddr_t		addr;
{
	struct vnode *vp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;      /* current credentials                  */
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	crp = crref();

	if (rc = lookupname(path, USR, L_SEARCH, NULL, &vp, crp) == 0) {
		rc = VFS_QUOTACTL(vp->v_vfsp, cmd, id, addr, crp);
		VNOP_RELE(vp);
	}

	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
