static char sccsid[] = "@(#)01	1.7.2.5  src/bos/kernel/lfs/chmod.c, syslfs, bos411, 9428A410j 12/15/93 14:47:58";

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: chmod, fchmod
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

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/audit.h"
#include "sys/trchkid.h"
#include "sys/malloc.h"

/*
 * NAME: chmod()    (system call entry point)
 * 
 * FUNCTION: Changes the mode on the named file.
 *
 * PARAMETERS: fname and fmode. Fname is the file name and fmode is an integer
 *             with the new permissions.
 *
 * RETURN VALUES: Explicitly none, implicitly sets u.u_error
 */
chmod(fname, fmode)
char	*fname;
int	fmode;
{
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	static int svcnum = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Mode", &svcnum, 1, fmode)))
	{
		if(fname){
			char *ptr;
			int len;

			if((ptr = malloc(MAXPATHLEN)) == NULL)
				rc = ENOMEM;
			else if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
				rc = EFAULT;
				free(ptr);
			}
			else {
				audit_svcbcopy(ptr, len);
				free(ptr);
			}
		}
		audit_svcfinis();
	}

	/* get the parameters and call setnameattr */
	rc = setnameattr(fname, V_MODE, fmode, 0, 0, 0);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}

/*
 * NAME fchmod()    (system call entry point)
 *
 * FUNCTION: changes the mode on an open file associated with the given
 *           file descriptor.
 *
 * PARAMETERS: fdes, mode. Fdes is the open file descriptor and mode is
 *             an integer, the new mode.
 *
 * RETURN VALUES: explicitly none, implicitly u.u_error is set
 */
fchmod(fdes, mode)
int	fdes;
int	mode;
{
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;
	int rc;
	static int svcnum = 0;

	TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_FCHMOD, fdes, mode);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Mode", &svcnum, 1, mode)))
		audit_svcfinis();

	/* get the file pointer from the file descriptor */
	if ((rc = getft(fdes, &fp, DTYPE_VNODE)) == 0)
	{
		/* get creds for the operation */
		crp = crref();

		/* we have a valid file pointer so let's set the mode */
		rc = vsetattr(fp->f_vnode, V_MODE, mode, 0, 0, crp);

		crfree(crp);
		ufdrele(fdes);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}
