static char sccsid[] = "@(#)79	1.7  src/bos/usr/ccs/lib/libcur/clrtoeol.c, libcur, bos411, 9428A410j 3/8/94 11:08:42";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wclrtoeol
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
 * NAME:                wclrtoeol
 *
 * FUNCTION: This routine clears to the end of the
 *      line from the current location in the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wclrtoeol(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: waddch(), wmove()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wclrtoeol(win)
register    WINDOW  *win;
{
    register int    y,
                    x;
    ATTR saved_attrs =  win->_csbp;

    getyx(win, y, x);		/* save current location in window */
    win->_csbp &= B_BLACK | B_RED | B_BLUE | B_GREEN | B_BROWN | 
	    	  B_MAGENTA | B_CYAN | B_WHITE;

    waddch(win, NLSNL);		/* a <nl> will clear to the end of the
				   line */

    win->_csbp = saved_attrs;
    wmove(win, y, x);		/* reset current location in window */
}
