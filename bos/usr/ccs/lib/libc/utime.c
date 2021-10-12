static char sccsid[] = "@(#)61	1.9  src/bos/usr/ccs/lib/libc/utime.c, libcfs, bos411, 9428A410j 6/16/90 01:07:10";
/*
 * COMPONENT_NAME: LIBCFS - C Library File System Interfaces
 *
 * FUNCTIONS: utime
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

#include <sys/time.h>
#include <unistd.h>
#include <utime.h>

/*
 * utime()
 *
 * changes the modification and access times on a file
 * (this is a compatibility routine that invokes the utimes system call)
 */

utime(char *fname, struct utimbuf *times)
{
	struct timeval tvp[2];

	if (times)
	{
		/* intialize the variables */
		tvp[0].tv_sec  = times->actime;
		tvp[0].tv_usec = 0;
		tvp[1].tv_sec  = times->modtime;
		tvp[1].tv_usec = 0;
	}

	return(utimes(fname, times ? tvp : (struct timeval *)NULL));
}
