static char sccsid[] = "@(#)72	1.1  src/bos/usr/ccs/lib/libbsd/fcntl.c, libbsd, bos411, 9428A410j 3/25/90 22:38:43";

/*
 * COMPONENT_NAME: LIBBSD - Berkeley compatiblity library
 *
 * FUNCTIONS: fcntl
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

#ifndef	_NO_PROTO
#define	_NO_PROTO
#endif

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*
* In BSD fcntl F_SETFL results in ioctl FIONBIO and FIOASYNC.  In
* AIX, F_SETFL only sets the file flags.  This code provides BSD
* compatibility.
*
* Note:  __fcntl is an internal interface to re-startable fcntl in
* libc.
*/

fcntl(fd,cmd,arg)
int fd, cmd, arg;
{
	int save_errno, rc, tmp;

	if ((rc = __fcntl(fd,cmd,arg)) == -1)
		return rc;

	if (cmd == F_SETFL)
	{
		/*
		* Drive FIOASYNC and FIONBIO based on the new
		* file flags ala BSD. We ignore errors from the 
		* ioctl since some devices may not support them.
		* Also, save the previous value of errno to
		* preserve system call semanitics.
		*/
		save_errno = errno;
		tmp = arg & FNDELAY;
		(void) __ioctl(fd,FIONBIO,&tmp,0);
		tmp = arg & FASYNC;
		(void) __ioctl(fd,FIOASYNC,&tmp,0);
		errno = save_errno;
	}

	return rc;
}
