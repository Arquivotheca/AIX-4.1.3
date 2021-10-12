static char sccsid[] = "@(#)33	1.3  src/bos/usr/ccs/lib/libc/tcgetattr.c, libctty, bos411, 9428A410j 6/16/90 01:35:20";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: tcgetattr
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

#include <termio.h>

int tcgetattr(int fd, struct termios *term)
{
    return isatty(fd) ? ioctl(fd, TCGETS, term) : -1;
}
