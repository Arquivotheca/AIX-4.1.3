static char sccsid[] = "@(#)68	1.9  src/bos/usr/ccs/lib/libcur/clrtobot.c, libcur, bos411, 9428A410j 3/8/94 11:08:29";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wclrtobot
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

#define bgattrs(attr) ((attr) & (B_BLACK | B_RED | B_BLUE | B_GREEN | \
		    	         B_BROWN | B_MAGENTA | B_CYAN | B_WHITE))

/*
 * NAME:                wclrtobot
 *
 * FUNCTION: This routine erases everything on the
 *      window, starting at the current (y,x) coordinates of the window.
 *      It leaves the current (y,x) coordinates unchanged.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wclrtobot(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: getyx()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wclrtobot(win)
register    WINDOW  *win;
{
    register    ATTR    *ap;
    register int    y,
                    startx,
                    minx;
    register    NLSCHAR *sp,
			*end,
			*maxx;

/* starting at current (y,x), set characters = blanks & attributes = bg _csbp */
    for (getyx(win, y, startx); y < win->_maxy; y++) {

	end = &win->_y[y][win->_maxx];/* set characters to blanks, and */
	ap = &win->_a[y][startx];/* attributes to bg _csbp           */

	for (minx = _NOCHANGE, sp = &win->_y[y][startx]; sp < end; sp++, ap++)
	    if (*sp != NLSBLANK || *ap != bgattrs(win->_csbp)) {
		maxx = sp;
		if (minx == _NOCHANGE) {
		    /* if the start point for clearing is on the
		       second byte of a two byte character, back
		       up one position and erase the first byte */
		    if (sp - win->_y[y] && is_second_byte (*sp))
			sp--;
		    minx = sp - win->_y[y];
		}
		*sp = NLSBLANK;
		*ap = bgattrs(win->_csbp);
	    }

	if (minx != _NOCHANGE) {
	    if (win->_firstch[y] > minx || win->_firstch[y] == _NOCHANGE)
		win->_firstch[y] = minx;
	    if (win->_lastch[y] < maxx - win->_y[y])
		win->_lastch[y] = maxx - win->_y[y];
	}
	startx = 0;
    }

    return OK;
}
