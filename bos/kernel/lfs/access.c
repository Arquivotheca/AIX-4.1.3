static char sccsid[] = "@(#)35	1.17  src/bos/kernel/lfs/access.c, syslfs, bos411, 9428A410j 4/6/94 13:00:17";
/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: access, fp_access
 *
 * ORIGINS: 27, 3, 26
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
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/access.h"
#include "sys/fp_io.h"
#include "sys/trchkid.h"

BUGVDEF(accdbg,0);

/*
 * NAME:  access() 	(system call entry point)
 *
 * FUNCTION:  access checks the access permissions on the named file, based
 *		on the mode parameter.
 *
 * PARAMETERS:	fname - file name to check
 *		fmode - mode to check file against
 *
 * RETURNS:	explicity none, implicity sets u.u_error.
 */

access(fname,fmode)
char	*fname;
int	fmode;
{

	TRCHKL1T(HKWD_SYSC_LFS | hkwd_SYSC_ACCESS, fmode);

	return(accessx(fname, fmode, ACC_INVOKER));
}


/*
 * NAME: fp_access
 *
 * FUNCTION: Check access permissions on an open file.
 *
 * NOTE:
 *	Code must be added here to call a file system access service,
 *	because things get somewhat complicated with with access
 *	control lists and privilege vectors.
 *
 * RETURNS:	0 if permissions are valid;
 *		1 if access denied (u.u_error is set).
 */

fp_access(fp, perm)

register struct file	*fp;		/* file descriptor pointer */
register int		perm;		/* permissions to check */
{
	struct stat	stst;		/* stat structure */
	int rc;				/* return code */
	int klock;	                /* save kernel_lock state */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	if ((rc = fp_fstat(fp, &stst, STATSIZE, FP_SYS)) == 0)
		if ((stst.st_mode & (perm|(perm>>3)|(perm>>6))) == 0)
			rc = fp_accessx(fp, perm >> 6, ACC_ANY);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
