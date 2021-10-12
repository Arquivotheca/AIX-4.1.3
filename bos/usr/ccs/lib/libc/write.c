static char sccsid[] = "@(#)91	1.3  src/bos/usr/ccs/lib/libc/write.c, libcfs, bos411, 9428A410j 12/7/93 19:20:50";
/*
 * COMPONENT_NAME: LIBCFS - File System interfaces in the C library
 *
 * FUNCTIONS: write, writev, writex, writevx
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/uio.h>

/*
* The functions in this file implement the "write" system call with
* BSD style restartability.  In the BSD kernel, restartable system
* calls are implemented in the kernel by modifying the saved state
* of the process so that a system call that transfered no data is
* restarted after a signal handler returns.  Since the V3 system
* call interfaces uses volatile registers to pass the arguments,
* that approach is unworkable.  Hence, the system call restarts are
* handled explicity here in the c library.  If the "write" system
* call is interrupted before any data is transfered, then the system
* call returns ERESTART implying that the system call needs to be
* restarted.  If data has been transfered, then the system call
* reports the amount transfered up to the interruption.
*/

#define	KWRITEV(fd,iov,iovcnt,x,rc)				\
	while ( (rc = kwritev(fd,iov,iovcnt,x)) == -1)		\
		if (errno != ERESTART)				\
			break;

ssize_t
write(int fd, const void *buf, size_t count)
{
	struct iovec	iov;
	int		rc;

	iov.iov_base = buf;
	iov.iov_len = count;

	KWRITEV(fd,&iov,1,0,rc);
	return rc;
}

writev(int fd, const struct iovec *iov, int iovcnt)
{
	int	rc;

	KWRITEV(fd,iov,iovcnt,0,rc);
	return rc;
}

int
writex(int fd, char *buf, unsigned int count, long x)
{
	struct iovec	iov;
	int		rc;

	iov.iov_base = buf;
	iov.iov_len = count;

	KWRITEV(fd,&iov,1,x,rc);
	return rc;
}

writevx(fd, iov, iovcnt, x)
int		fd;
struct iovec *	iov;
int		iovcnt;
long		x;
{
	int	rc;

	KWRITEV(fd,iov,iovcnt,x,rc);
	return rc;
}
