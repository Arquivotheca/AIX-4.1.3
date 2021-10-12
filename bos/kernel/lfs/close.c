static char sccsid[] = "@(#)03	1.13.1.7  src/bos/kernel/lfs/close.c, syslfs, bos41J, 9514A_all 3/29/95 11:10:20";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: close, closefd
 *
 * ORIGINS: 3, 27
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

#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/lockf.h"
#include "sys/syspest.h"
#include "sys/proc.h"
#include "sys/trchkid.h"
#include "sys/audit.h"
#include "sys/fs_hooks.h"
#include "sys/sleep.h"

/*
 * NAME:	close()		( system call entry point)
 *
 * FUNCTION:	Do a little preliminary work and then call the
 *		common code for closing a file descriptor.
 *
 * PARAMETERS:	fdes. Fdes is the user file descriptor provided by
 *		open or create.
 *
 * RETURNS:	really shouldn't fail. (we hope)
 *
 * SERIALIZATION: Take the U_FD_LOCK when checking the file descriptor
 *		  table and setting the UF_CLOSING flag. If another
 *		  thread is still using the file descriptor, we go to
 *		  sleep, and e_sleep_thread places us on the u_fdevent
 *		  sleep list and releases the U_FD_LOCK before putting
 *		  us to sleep.
 *
 */
close(fdes)
int	fdes;
{
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int tlock;		/* is multi-thread locking required?	*/
	int rc;
	static int svcnum = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Close", &svcnum, 1, fdes)))
		audit_svcfinis();

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();
	if ((fdes < 0 || fdes >= U.U_maxofile)
	   || (U.U_ufd[fdes].flags & UF_CLOSING)
	   || ((fp = U.U_ufd[fdes].fp) == NULL))
	{
		if (tlock)
			U_FD_UNLOCK();
		rc = EBADF;
	}
	else
	{
		U.U_ufd[fdes].flags |= UF_CLOSING;
		while (U.U_ufd[fdes].count != 0)
		{
			assert(curproc->p_threadcount > 1);
			(void) e_sleep_thread(&U.U_fdevent,
					&U.U_fd_lock, LOCK_SIMPLE);
		}
		if (tlock)
			U_FD_UNLOCK();

		/* trace file descriptor and vnode pointer */
		TRCHKL2T(HKWD_SYSC_CLOSE, fdes, fp->f_vnode);

		/* call common code for closing a file descriptor */
		rc = closefd(fdes, 1);
	}

	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}


/*
 * NAME:	closefd()
 *
 * FUNCTION:	Call registered close hooks for this file descriptor.
 *		Clean up any locks this process may have done using
 *		this file descriptor. Clean up any mapping done with
 *		this file descriptor. Release any resources held by
 *		this file by calling closef().  Finally, free the
 *		file descriptor by calling ufdfree() if we're updating
 *		the file descriptor table.
 *
 *		Note that we require that the caller check that the
 *		file pointer is ok.  An alternative would be to
 *		return noiselessly if it's NULL, for example.
 *
 * PARAMETERS:	fdes:  Fdes is the user file descriptor provided by
 *		open or create.
 *		maintain_fdtable:  Do we want to keep the user's
 *		file descriptor table up to date?  We do unless
 *		we're exiting.
 *
 * RETURNS:	result from closef() (which is U.Ually ignored
 *		by the caller.)
 */
int
closefd(int fdes, int maintain_fdtable)
{
	struct file    *fp = U.U_ufd[fdes].fp;
	struct fs_hook *closeh;
	int rc;

	ASSERT(fp);

	/*
	 * Call all registered close hooks.
	 */
	for (closeh = closeh_anchor; closeh; closeh = closeh->next)
		(closeh->hook)(fdes, fp);
	
	if (U.U_lockflag)
		lockrelease(fp);

	if (U.U_ufd[fdes].flags & UF_MAPPED)
		rmseg(fdes);

	rc = closef(fp);

	if (maintain_fdtable)
		ufdfree(fdes);

	return rc;
}
