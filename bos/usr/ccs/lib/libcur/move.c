static char sccsid[] = "@(#)44	1.7  src/bos/usr/ccs/lib/libcur/move.c, libcur, bos411, 9428A410j 6/16/90 01:40:47";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wmove
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

#include        "cur99.h"

/*
 * NAME:                wmove
 *
 * FUNCTION: This routine changes the current location
 *      in the window.  The current location is defined to be the place
 *      where the next character would be added to the data structures.
 *
 * EXECUTION ENVIRONMENT:
 *
 *       wmove(win, y, x), where 'win' is a pointer to the window,
 *      'y' is the new line location, and 'x' is the new column location.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wmove(win, y, x)
register    WINDOW  *win;
register int    y,
                x;
{
    if (x >= win->_maxx || y >= win->_maxy || x < 0 || y < 0)
	return ERR;

    win->_curx = x;		/* set the current location of this
				   window... */
    win->_cury = y;

    return OK;
}
