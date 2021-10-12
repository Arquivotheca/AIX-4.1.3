static char sccsid[] = "@(#)35	1.3  src/bos/usr/ccs/lib/libc/tcsendbreak.c, libctty, bos411, 9428A410j 6/16/90 01:35:28";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: tcsendbreak
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

int tcsendbreak(int fd, int duration)
{
    return isatty(fd) ? ioctl(fd, TCSBREAK, duration) : -1;
}
