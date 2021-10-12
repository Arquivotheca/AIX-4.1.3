static char sccsid[] = "@(#)34	1.4  src/bos/usr/ccs/lib/libc/tcgetpgrp.c, libctty, bos411, 9428A410j 6/2/91 00:40:56";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: tcgetpgrp
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

#include <sys/ioctl.h>
#include <sys/types.h>

pid_t tcgetpgrp(int fd)
{
    pid_t pgrp;

    return (isatty(fd) && !ioctl(fd, TXGPGRP, &pgrp)) ? pgrp : (pid_t)-1;
}
