static char sccsid[] = "@(#)10	1.7  src/bos/usr/ccs/lib/libcur/touchwin.c, libcur, bos411, 9428A410j 6/16/90 01:41:49";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: touchwin
 *
 * ORIGINS: 10, 27
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
 * NAME:                touchwin
 *
 * FUNCTION: This routine changes the optimization
 *      arrays of the window such that it looks like the window may have
 *      had every character changed, i.e. wrefresh() will check them all.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      touchwin(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

touchwin(win)
register    WINDOW  *win;
{
    register short  maxy,
                    maxx,
                    y;

    maxy = win->_maxy;
    maxx = win->_maxx - 1;

    for (y = 0; y < maxy; y++) {
	win->_firstch[y] = 0;
	win->_lastch[y] = maxx;
    }

    if (!win->_scroll && (win->_flags & _SCROLLWIN)) {
	win->_lastch[maxy - 1] = maxx - 1;
	return ERR;
    }

    return OK;
}
