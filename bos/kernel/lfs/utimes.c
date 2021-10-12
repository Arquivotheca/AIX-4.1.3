static char sccsid[] = "@(#)13	1.6  src/bos/kernel/lfs/utimes.c, syslfs, bos411, 9428A410j 4/19/94 16:15:42";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: utimes
 *
 * ORIGINS: 26, 27
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
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/vattr.h"

/*
 * NAME: utimes()    (system call entry point)
 *
 * FUNCTION: this call changes the modification and acess times on a file
 *
 * PARAMETERS: fname, tptr.  fname is the file name and tptr is the pointer
 *             to the array of timeval structures containing the new times.  If
 *             tptr is NULL we use the current system time; otherwise we use the
 *             times in the timeval structures.
 *
 * RETURN VALUES: explicitly none, implicitly sets u.u_error.
 */
utimes(fname, times)
char		*fname;
struct timeval	*times;
{
	int timeflg = 0; 
	struct timeval tv[2];
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	struct timestruc_t etv[2];

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* set the actime and modtime if we have them */
	if (times == (struct timeval *)NULL)
	{
		timeflg |= T_SETTIME;
		tv[0].tv_sec = 0;
		tv[1].tv_sec = 0;
	}
	else if( copyin(times,tv,sizeof(tv)) )
	{
		rc = EFAULT;
		goto out;
	}

	etv[0].tv_sec = tv[0].tv_sec;
	etv[1].tv_sec = tv[1].tv_sec;

	etv[0].tv_nsec = 0;
	etv[1].tv_nsec = 0;

	/* reset the time */
	rc = setnameattr(fname, V_UTIME, timeflg,
		&etv[0], &etv[1], 0);

out:
	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}
