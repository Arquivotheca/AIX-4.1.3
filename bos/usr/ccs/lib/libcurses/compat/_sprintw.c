static char sccsid[] = "@(#)14  1.6  src/bos/usr/ccs/lib/libcurses/compat/_sprintw.c, libcurses, bos411, 9428A410j 6/16/90 01:44:44";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _sprintw
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
 * NAME:        _sprintw
 *
 * FUNCTION:
 *
 *      This routine actually executes the printf and adds it to the window
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This code now uses the vsprintf routine, which portably digs
 *	into stdio.  We provide a vsprintf for older systems that don't
 *	have one.
 */

_sprintw(win, fmt, ap)
WINDOW	*win;
char	*fmt;
va_list ap;
{
	char	buf[BUFSIZ];

	vsprintf(buf, fmt, ap);
	return waddstr(win, buf);
}
