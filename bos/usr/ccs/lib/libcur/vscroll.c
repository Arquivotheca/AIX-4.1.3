static char sccsid[] = "@(#)53	1.7  src/bos/usr/ccs/lib/libcur/vscroll.c, libcur, bos411, 9428A410j 6/16/90 01:42:09";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: vscroll
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
 * NAME:                vscroll
 *
 * FUNCTION: This routine moves a viewport (of a
 *      window) around on the window - simulating scrolling.  The direc-
 *      tion of the movement is detemined from the sign of the arguements.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      vscroll(vwin, dy, dx), where 'vwin' is a pointer to the
 *      viewport, and 'dy' & 'dx' are the vectors describing the magnitude
 *      and direction of the movement.
 *
 * EXTERNAL REFERENCES: touchwin()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

vscroll(vwin, dy, dx)
register    WINDOW  *vwin;
register int    dy,
                dx;
{
    register int    i,
                    y,
                    x;

    if (!(vwin->_flags & _ISVIEW))/* "prevent" undesirable use of */
	return ERR;		/* this routine accidentally.	 */

    y = vwin->_winy;		/* get the relative offsets of the
				   viewport */
    x = vwin->_winx;		/* within the window it's on              
				    */

 /* make sure the viewport isn't scrolled off the window */
    if (dy > vwin->_view->_maxy - y - vwin->_maxy)
	dy = vwin->_view->_maxy - y - vwin->_maxy;
    else
	if (dy < -y)
	    dy = -y;
    if (dx > vwin->_view->_maxx - x - vwin->_maxx)
	dx = vwin->_view->_maxx - x - vwin->_maxx;
    else
	if (dx < -x)
	    dx = -x;

 /* if there is a scroll of 1 cell or more, then move the pointers */
    if (dy != 0 || dx != 0) {
	for (i = 0; i < vwin->_maxy; i++) {
	    vwin->_y[i] = &(vwin->_view->_y[y + dy + i][x + dx]);
	    vwin->_a[i] = &(vwin->_view->_a[y + dy + i][x + dx]);
	}
	touchwin(vwin);		/* must do a touchwin() after a scroll */
	vwin->_winy += dy;	/* update the offsets */
	vwin->_winx += dx;
    }

    return OK;
}
