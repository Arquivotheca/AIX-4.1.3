static char sccsid[] = "@(#)80	1.6  src/bos/usr/ccs/lib/libcurses/compat/__sscans.c, libcurses, bos411, 9428A410j 6/16/90 01:42:33";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   __sscans
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"
# include	<varargs.h>

/*
 * NAME:        __sscans
 *
 * FUNCTION:    executes the scanf from the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This code calls vsscanf, which is like sscanf except
 *      that it takes a va_list as an argument pointer instead
 *      of the argument list itself.  We provide one until
 *      such a routine becomes available.
 */

__sscans(win, fmt, ap)
WINDOW	*win;
char	*fmt;
va_list	ap;
{
	char	buf[256];

	if (wgetstr(win, buf) == ERR)
		return ERR;

	return vsscanf(buf, fmt, ap);
}
