static char sccsid[] = "@(#)77	1.7  src/bos/usr/ccs/lib/libcur/mvwin.c, libcur, bos411, 9428A410j 6/16/90 01:41:01";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: mvwin
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
 * NAME:                mvwin
 *
 * FUNCTION: This routine will move the starting
 *      position of a window relative to the glass and call touchwin as a
 *      result.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mvwin(win, y, x), where 'win' is a pointer to the window
 *      and 'y' & 'x' are the new starting coordinates of the window on
 *      the glass.
 *
 * EXTERNAL REFERENCES: touchwin()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

mvwin(win, y, x)
register    WINDOW  *win;
register int    y,
                x;
{
    if (y + win->_maxy > LINES || x + win->_maxx > COLS || y < 0 || x < 0)
	return ERR;

    win->_begy = y;
    win->_begx = x;

    touchwin(win);

    return OK;
}
