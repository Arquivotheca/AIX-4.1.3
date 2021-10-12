static char sccsid[] = "@(#)07	1.9.1.2  src/bos/kernel/lfs/setattr.c, syslfs, bos411, 9428A410j 12/15/93 14:45:39";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: setnameattr, vsetattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/chownx.h"
#include "sys/vattr.h"
#include "sys/audit.h"

/*
 * NAME: setnameattr(pathname, cmd, arg1, arg2, arg3)
 *
 * FUNCTION: Code shared by all programs that set attributes given a file name
 *
 * PARAMETERS: pathname, cmd, arg1, arg2and arg3.  Pathname is the name of the
 *   	       file, cmd is the desired operation, and arg1, arg2 and arg3 are
 *	       the arguments to the command.  All three arguments must be passed
 *	       and those that are not used will be ignored.  All arguments are
 *	       guaranteed to fit into an integer on this machine.
 *
 * RETURN VALUES: returns any error codes
 */
int
setnameattr(pathname, cmd, arg1, arg2, arg3, lflag)
char *pathname;
int cmd, arg1, arg2, arg3, lflag;
{
	register int rc;
	struct vnode *vp;
	static int tcbmod = 0;
	struct ucred *crp;

	/* get current credentials */
	crp = crref();

	/* let's look up the name and get the vnode for it */
	rc = lookupname(pathname, USR, L_SEARCH|L_EROFS|lflag, NULL, &vp, crp);

	if (rc == 0)
	{
		/* we have a vnode, let's set the attributes */
		rc = vsetattr(vp, cmd, arg1, arg2, arg3, crp);

		/* release the vnode and return any code */
		VNOP_RELE(vp);
	}

	crfree(crp);
	return (rc);
}

/*
 * NAME:	vsetattr()
 *
 * FUNCTION:	To check for a read only file system and call
 *		the SETATTR vnode op (isolate it to one call).
 *
 * PARAMETERS:	Same as the VNOP_SETATTR.
 *
 * RETURNS:	Same as VNOP_SETATTR.
 */
vsetattr(vp, cmd, arg1, arg2, arg3, crp)
register struct vnode *vp;
register int cmd;
register int arg1, arg2, arg3;
struct ucred *crp;
{
	static int tcbmod = 0;
	int rc;

	if (vp->v_vfsp->vfs_flag & VFS_READONLY)
		return(EROFS);

	rc = VNOP_SETATTR(vp, cmd, arg1, arg2, arg3, crp);

	/* 
	 * Success TCB_Mod has occured
	 */

	if ((rc == 0)
	    && (audit_flag)
	    && (vp->v_gnode->gn_flags & GNF_TCB))
	{
                if (audit_svcstart("TCB_Mod", &tcbmod, 0))
                        audit_svcfinis();
	}

	return(rc);
}
