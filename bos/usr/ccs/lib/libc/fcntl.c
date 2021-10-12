static char sccsid[] = "@(#)09	1.2  src/bos/usr/ccs/lib/libc/fcntl.c, libcfs, bos411, 9428A410j 6/16/90 01:06:48";

/*
 * COMPONENT_NAME: LIBCFS - File System interfaces in the C library
 *
 * FUNCTIONS: fcntl, lockfx
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
* POSIX declares fcntl as fcntl(int fd, int cmd, ...);  Strictly
* speaking, this routine must use varargs to handle a variable
* number of arguments in a portable fashion.
*/

#define	_NO_PROTO

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/lockf.h>

fcntl(fd,cmd,arg)
int fd, cmd, arg;
{
	return __fcntl(fd,cmd,arg);
}


lockfx(fd,cmd,arg)
int     fd;
int     cmd;
int     arg;
{
	if( cmd != F_SETLK && cmd != F_SETLKW && cmd != F_GETLK )
	{
		errno = EINVAL;
		return -1;
	}
	return __fcntl(fd,cmd,arg);
} 

/* lockf should be here too! */

/* restartable fcntl */
__fcntl(fd,cmd,arg)
int fd, cmd, arg;
{
	int save_errno, rc;
	pid_t owner;

	if (cmd == F_GETOWN)
	{
		if ((rc = __fgetown(fd,&owner,0)) == -1)
			return rc;
		else
			return (int) owner;
	}
	if (cmd == F_SETOWN)
		return __fsetown(fd,&arg,0);

	/*
	* Loop making the fcntl call so long as kfcntl returns
	* ERESTART.  Avoid bashing errno when ERESTART is returned.
	*/

	save_errno = errno;
	while ((rc=kfcntl(fd,cmd,arg)) == -1)
		if (errno != ERESTART)
			break;
		else
			errno = save_errno;

	return rc;
}
