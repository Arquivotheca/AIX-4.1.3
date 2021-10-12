static char sccsid[] = "@(#)38	1.4  src/bos/usr/ccs/lib/libc/tcsetpgrp.c, libctty, bos411, 9428A410j 12/4/91 12:48:34";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: tcsetpgrp
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

int tcsetpgrp(int fd, pid_t pgrp)
{
    return isatty(fd) ? ioctl(fd, TXSPGRP, &pgrp) : -1;
}
