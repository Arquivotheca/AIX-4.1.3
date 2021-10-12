static char sccsid[] = "@(#)56	1.6  src/bos/usr/ccs/lib/libcur/delwin.c, libcur, bos411, 9428A410j 6/16/90 01:37:39";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: delwin
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
 * NAME:                delwin
 *
 * FUNCTION: This routine deletes a window by
 *      releasing the window's allocated space back to the program data
 *      segment.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      delwin(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: cfree()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

delwin(win)
register    WINDOW  *win;
{
    if (!(win->_flags & _SUBWIN)) {
				/* regular window, release everything */
	register int    i;
	for (i = 0; i < win->_maxy && win->_y[i]; i++) {
	    cfree(win->_y[i]);
	    cfree(win->_a[i]);
	}
    }
    cfree(win->_y);
    cfree(win->_a);
    cfree(win->_firstch);
    cfree(win->_lastch);
    free(win);
    return OK;
}
