static char sccsid[] = "@(#)38	1.15  src/bos/kernel/lfs/closef.c, syslfs, bos411, 9428A410j 10/22/93 17:28:55";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: closef, fp_close
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
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include "sys/fs_locks.h"
#include <sys/file.h>
#include <sys/syspest.h>
#include <sys/fp_io.h>

/*
 * DEBUG !!!
 */
BUGVDEF(clsdbg,0);

/*
 * NAME: closef()
 *
 * FUNCTION: Internal form of close. Decrement count of fp and call
 *	     VNOP_CLOSE. Will also release the open vnode.
 *
 * PARAMETERS:	fp. Fp is the file pointer we wish to close.
 *
 * RETURN VALUES: Returns any error codes that may occur.
 *
 * SERIALIZATION: The file pointer count is serialized by the FP_LOCK
 *		  for that file structure.
 *
 */
int
closef(fp)
register struct file *fp;
{
	int error, count;

	BUGLPR(clsdbg, BUGACT, ("closef(fp: %x)\n",fp));

	ASSERT(fp != NULL);
	ASSERT(fp->f_count > 0);


	/* Take the fp_lock when decrementing the reference count */
	FP_LOCK(fp);
	count = --fp->f_count;
	FP_UNLOCK(fp);
	if (count > 0)
		return 0;

	/* reference count now zero, call fileop layer, then free fp */
	if (fp->f_ops)
		error = (*fp->f_ops->fo_close)(fp);

	if (fp->f_cred)
		crfree(fp->f_cred);
	fpfree(fp);

	return error;
}

/*
 * NAME: fp_close()
 *
 * FUNCTION: Call back routine mainly used by device drivers.
 *
 * PARAMETERS:	fp. Fp is the file pointer we wish to close.
 *
 * RETURN VALUES: Returns any error codes that may occur.
 *
 * SERIALIZATION: The file pointer count is serialized by the FP_LOCK
 *		  for that file structure. If the caller is calling back
 *		  into the LFS, we must be sure that we release the
 *		  kernel lock before we call closef().
 *
 */
int
fp_close(struct file *fp)
{
	int		waslocked;	/* kernel lock status		*/
	int		rc;		/* closef() return code		*/
	int klock;	                /* save kernel_lock state       */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	rc = closef(fp);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
