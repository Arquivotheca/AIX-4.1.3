static char sccsid[] = "@(#)36	1.4  src/bos/usr/ccs/lib/libc/tcsetattr.c, libctty, bos411, 9428A410j 1/21/94 02:45:24";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: tcsetattr
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <termio.h>
#include <sys/errno.h>

int tcsetattr(int fd, int optional_actions, const struct termios *term)
{
    
    if (!isatty(fd))
	return -1;

    switch (optional_actions) {
    case TCSANOW:
	return ioctl(fd, TCSETS, term);
    case TCSADRAIN:
	return ioctl(fd, TCSETSW, term);
    case TCSAFLUSH:
	return ioctl(fd, TCSETSF, term);
    default:
	errno = EINVAL;
	return -1;
    }
}
