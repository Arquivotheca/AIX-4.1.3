static char sccsid[] = "@(#)13  1.1  src/bos/usr/ccs/lib/libcurses/compat/vsscanf.c, libcurses, bos411, 9428A410j 9/2/93 14:15:51";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   vsscanf
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <varargs.h>

/*
 * NAME:        vsscanf
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine implements vsscanf (nonportably) until such time
 *      as one is available in the system (if ever).
 */

vsscanf(buf, fmt, ap)
char	*buf;
char	*fmt;
va_list	ap;
{
	FILE	junk;

	junk._flag = _IOREAD;
	junk._file = -1;
	junk._base = (unsigned char *) buf;
	junk._ptr  = (unsigned char *) buf;
	junk._cnt  = strlen(buf);
	return _doscan(&junk, fmt, ap);
}
