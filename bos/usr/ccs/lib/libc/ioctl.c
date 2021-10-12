static char sccsid[] = "@(#)98	1.5  src/bos/usr/ccs/lib/libc/ioctl.c, libcfs, bos411, 9428A410j 6/16/90 01:06:56";

/*
 * COMPONENT_NAME: LIBCFS - File System interfaces in the C library
 *
 * FUNCTIONS: ioctl, ioctlx, __ioctl, __fgetown, __fsetown
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

#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

ioctl(fd,cmd,arg)
int	fd;
int	cmd;
int	arg;
{
	return __ioctl(fd,cmd,arg,0);
}

ioctlx(fd,cmd,arg,x)
int	fd;
int	cmd;
int	arg;
long	x;
{
	return __ioctl(fd,cmd,arg,x);
}

/* restartable ioctl */

__ioctl(fd,cmd,arg,x)
int	fd;
int	cmd;
int	arg;
long	x;
{
	int save_errno, rc;

	if (cmd == FIOGETOWN)
		return __fgetown(fd,arg,0);
	if (cmd == FIOSETOWN)
		return __fsetown(fd,arg,0);

	save_errno = errno;

	while ((rc=kioctl(fd,cmd,arg,x)) == -1)
		if (errno != ERESTART)
			break;
		else
			errno = save_errno;

	return rc;
}

/*
* ioctl FIOGETOWN and fcntl F_GETOWN both funnel through
* here to get special handling that normally happens in
* the kernel in 4.3 BSD.  Sockets support a negative owner
* id implying that the owner is a process group while a
* positive owner id implies that the owner is just a process.
* The tty driver assumes that the owner is always a process
* group and must be positive.  The code below converts the
* ioctl or fcntl to the correspoding ioctl command and adjusts
* the returned value to compensate for the tty driver.
*/

__fgetown(fd,ownp,x)
int	fd;
pid_t *	ownp;
long	x;
{
	struct stat s;
	int rc, cmd;

	/* fetch file type */

	if ((rc = fstat(fd,&s)) == -1)
		return rc;	/* probably EBADF */

	/* determine appropriate ioctl command */

	cmd = S_ISSOCK(s.st_mode) ? SIOCGPGRP : TIOCGPGRP;

	if ((rc = __ioctl(fd,cmd,ownp,x)) == -1)
		return rc;

	/*
	* Since tty only supports the process group owner,
	* adjust the returned value to be negative.  This
	* isn't quite BSD compatible.
	*/

	if (cmd == TIOCGPGRP && *ownp > 0)
		*ownp = -*ownp;
	return 0;
}

__fsetown(fd,ownp,x)
int	fd;
pid_t *	ownp;
long	x;
{
	struct stat s;
	int rc, cmd;
	pid_t owner;
	extern pid_t kgetpgrp();	/* BSD getpgrp */

	/* fetch file type */

	if ((rc = fstat(fd,&s)) == -1)
		return rc;	/* probably EBADF */

	/* deterimine appropriate ioctl command */

	if (!S_ISSOCK(s.st_mode))
	{
		cmd = TIOCSPGRP;
		if (*ownp < 0)
		{
			owner = -*ownp;
			ownp = &owner;
		}
		else	/* find the process group */
		{
			if ((owner = kgetpgrp(*ownp)) == (pid_t) -1)
			{
				if (errno == 0)
					errno = ESRCH;
				return -1;
			}
			ownp = &owner;
		}
	}
	else
		cmd = SIOCSPGRP;

	return __ioctl(fd,cmd,ownp,x);
}
