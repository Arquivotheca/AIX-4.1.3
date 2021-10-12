static char sccsid[] = "@(#)29	1.10  src/bos/usr/ccs/lib/libc/dup.c, libcfs, bos411, 9428A410j 6/16/90 01:06:44";

/*
 * COMPONENT_NAME: LIBCFS - C Library File System Interfaces
 *
 * FUNCTIONS: dup, dup2
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/errno.h>

extern int errno;

/*
 * dup, dup2:  compatibility interfaces
 *
 * dup a file descriptor [(dup2:) to a known descriptor]
 */

dup(int old)
{
	return(fcntl(old, F_DUPFD, 0));
}

dup2(int old, int new)
{
	int	old_errno;	/* original errno on entering	*/
	
	old_errno = errno;
	if (new < 0  ||  new >= OPEN_MAX) {
		errno = EBADF;
		return(-1);
	}

	/* quick check to see that old file descriptor is valid	*/
	if (fcntl(old, F_GETFL, 0) < 0  &&  errno == EBADF)
		return(-1);
	errno = old_errno;

	if (old == new)
		return(new);	/* no dup, no error. */

	(void)close(new);	/* get rid of new */
	errno = old_errno;

	return(fcntl(old, F_DUPFD, new));
}
