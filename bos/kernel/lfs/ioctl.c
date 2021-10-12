static char sccsid[] = "@(#)05	1.13  src/bos/kernel/lfs/ioctl.c, syslfs, bos411, 9428A410j 5/12/94 11:01:35";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: kioctl, fp_ioctl
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

#include "sys/types.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include <sys/trchkid.h>

#define	FOP_IOCTL(f,c,a,x,y)	(*f->f_ops->fo_ioctl)(f,c,a,x,y);

/*
 * NAME:	kioctl()	(system call entry point)
 *
 * FUNCTION:	Get the file pointer associated with the user's file
 *		descriptor and call the corresponding file op.
 *
 * PARAMETERS:	Fdes, cmd, arg. Fdes is the file descriptor we wish
 *		to do the ioctl on. Cmd is the ioctl cmd type. Arg is
 *		any arguments that command might need.
 *
 * RETURNS:	Returns any errors. If no errors the cmd or arg may
 *		contain resultant values.
 */
kioctl(fdes, cmd, arg, ext)
int	fdes;
int	cmd;
int	arg;
caddr_t	ext;
{
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	TRCHKL3T(HKWD_SYSC_IOCTL, fdes, cmd, arg);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((rc = getf(fdes, &fp)) == 0)
	{
		u.u_ioctlrv = 0;		/* default ioctl return value */

		rc = FOP_IOCTL(fp, cmd, arg, ext, 0);

		if (rc == EINTR)		/* interrupted? */
			rc = ERESTART;		/* restart possibly */

		ufdrele(fdes);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	/* driver can change u.u_ioctlrv to specify a non-zero return value */
	if (rc)
		u.u_error = rc;
	return rc ? -1 : u.u_ioctlrv;
}

fp_ioctl(fp,cmd,arg,ext)
struct file *	fp;
unsigned int	cmd;
caddr_t		arg;
int		ext;
{
	int rc;
	int klock;	                /* save kernel_lock state */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	rc = FOP_IOCTL(fp, cmd, arg, ext, FKERNEL);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}

