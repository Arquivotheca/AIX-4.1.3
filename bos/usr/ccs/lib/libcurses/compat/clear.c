static char sccsid[] = "@(#)28  1.6  src/bos/usr/ccs/lib/libcurses/compat/clear.c, libcurses, bos411, 9428A410j 6/16/90 01:45:39";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wclear
 *
 * ORIGINS: 3, 10, 26, 27
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

/*
 * NAME:        wclear
 *
 * FUNCTION:
 *
 *      This routine clears the _window.
 *
 */

wclear(win)
register WINDOW	*win; {

	if (win == curscr)
		win = stdscr;
	werase(win);
	win->_clear = TRUE;
	return OK;
}
