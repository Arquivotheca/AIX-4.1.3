static char sccsid[] = "@(#)46	1.3  src/bos/usr/ccs/lib/libc/cfgetospeed.c, libctty, bos411, 9428A410j 1/12/93 11:12:50";

/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: cfgetospeed
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <termios.h>

#ifdef cfgetospeed
#undef cfgetospeed
#endif

/*
 * Note that the cfgetospeed and cfgetispeed macros must track any
 * changes to these routines.
 */

speed_t cfgetospeed(const struct termios *p)
{
    return (speed_t) (p->c_cflag & _CBAUD);
}
