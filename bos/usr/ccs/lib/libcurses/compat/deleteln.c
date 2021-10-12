static char sccsid[] = "@(#)64	1.8  src/bos/usr/ccs/lib/libcurses/compat/deleteln.c, libcurses, bos411, 9428A410j 10/10/91 16:10:48";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wdeleteln
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

#include	"curses.h"

/*
 * NAME:        wdeleteln
 *
 * FUNCTION:
 *
 *      This routine deletes a line from the screen.  It leaves
 *      (_cury,_curx) unchanged.
 */

wdeleteln(win)
register WINDOW	*win; {

	register chtype	*temp;
	register int		y;
	register chtype	*end;

	temp = win->_y[win->_cury];
	for (y = win->_cury; y < win->_maxy; y++) {
	    if (win->_flags & _SUBWIN) {
		int i;
		for (i=0; i < win->_maxx; i++)
		    win->_y[y][i] = win->_y[y+1][i];
	    }
	    else
		win->_y[y] = win->_y[y+1];
	    win->_firstch[y] = 0;
	    win->_lastch[y] = win->_maxx - 1;
	}
	if (win->_flags & _SUBWIN) {
	    int i;
	    for (i=0; i < win->_maxx; i++)
		win->_y[win->_maxy -1][i] = ' ';
	}
	else {
	    for (end = &temp[win->_maxx]; temp < end; )
		*temp++ = ' ';
	    win->_y[win->_maxy-1] = temp - win->_maxx;
	}

	return OK; /* P46613 */
}
