#ifndef lint
static char sccsid[] = "@(#)88	1.2  src/bos/usr/ccs/lib/libc/gtty.c, libctty, bos411, 9428A410j 6/16/90 01:34:57";
#endif

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: stty
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

/*
 * FUNCTION:	Berkeley backward compatable routine to get a tty's
 *		characteristics.
 *
 * RETURN VALUE DESCRIPTION:	
 *		Returns 0 open success.  Possible errors are EFAULT, EPERM,
 *		and EINVAL.
 *
 */

#include <sgtty.h>
int gtty(int fd, struct sgttyb *sg)
{
    return ioctl(fd, TIOCGETP, sg);
}
