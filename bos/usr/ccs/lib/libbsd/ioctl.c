static char sccsid[] = "@(#)73	1.2  src/bos/usr/ccs/lib/libbsd/ioctl.c, libbsd, bos411, 9428A410j 6/16/90 00:59:55";

/*
 * COMPONENT_NAME: LIBBSD - Berkeley compatiblity library
 *
 * FUNCTIONS: ioctl
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* define _BSD to ensure proper value of FNDELAY from <fcntl.h> */

#ifndef	_BSD
#define	_BSD
#endif

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*
* A number of BSD ioctls are not supported directly.  The code
* here implements some of them via fcntl.  They are:
*
* FIOCLEX - close on exec
* FIONCLEX - no close on exec
* FIONBIO - non-blocking I/O
* FIOASYNC - async I/O
*
* Note: kfcntl is the name of the (non-restartable) fcntl system
* call.  It is used here because the fcntl commands being used
* won't need to be restarted because they cannot block.  __ioctl
* is the name of a routine in libc that implements restartable ioctl.
*/

ioctl(fd,cmd,arg,x)
int fd, cmd, arg, x;
{
	int flags, rc, save_errno, newbit;

	/* convert to appropriate fcntl */
	if (cmd == FIOCLEX)
		return kfcntl(fd,F_SETFD,FD_CLOEXEC);

	if (cmd == FIONCLEX)
		return kfcntl(fd,F_SETFD,0);

	if (cmd == FIONBIO || cmd == FIOASYNC)
	{
		/* set the file flags via F_SETFL ala BSD */

		/* get the current file flags */
		if ((flags = kfcntl(fd,F_GETFL,0)) == -1)
			return -1;	/* probably EBADF */

		/* compute the new file flags */
		newbit = cmd == FIONBIO ? FNDELAY : FASYNC;

		if (*(int *)arg)
			flags |= newbit;
		else
			flags &= ~newbit;

		if ((rc = kfcntl(fd,F_SETFL,flags)) == -1)
			return rc;	/* can't fail */
		
		save_errno = errno;
		(void) __ioctl(fd,cmd,arg,x);
		errno = save_errno;
		return rc;
	}

	return __ioctl(fd,cmd,arg,x);
}
