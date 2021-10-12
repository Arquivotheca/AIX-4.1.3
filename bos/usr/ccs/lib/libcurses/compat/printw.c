static char sccsid[] = "@(#)75  1.1  src/bos/usr/ccs/lib/libcurses/compat/printw.c, libcurses, bos411, 9428A410j 9/2/93 13:26:28";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: printw
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

/*
 * printw and friends
 *
 * 1/26/81 (Berkeley) 
 */

# include	"cursesext.h"
# include	<varargs.h>

/*
 * NAME:        printw
 *
 * FUNCTION:
 *
 *      This routine implements a printf on the standard screen.
 */

printw(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	return _sprintw(stdscr, fmt, ap);
}
