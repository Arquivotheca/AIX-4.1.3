static char sccsid[] = "@(#)36	1.4  src/bos/usr/ccs/lib/libc/isatty.c, libctty, bos411, 9428A410j 6/16/90 01:35:01";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS:  isatty
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
/*
 * isatty(fildes)
 * Returns 1 if file is a tty
 */
/* The isatty() function includes all the POSIX requirements */

#define _ALL_SOURCE
#include <sys/ioctl.h>
#include <sys/errno.h>

int isatty(int f)
{
    int result;
    extern int errno;

    if (!(result = ioctl(f, TXISATTY, 0) == 0) && errno != EBADF)
	errno = ENOTTY;
    return result;
}
