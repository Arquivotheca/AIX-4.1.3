static char sccsid[] = "@(#)13	1.7  src/bos/usr/ccs/lib/libcurses/compat/insertln.c, libcurses, bos411, 9428A410j 10/10/91 16:10:11";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   winsertln
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
 * NAME:        winsertln
 *
 * FUNCTION:
 *
 *      This routine performs an insert-line on the _window, leaving
 *      (_cury,_curx) unchanged.
 */

winsertln(win)
register WINDOW	*win; {

	register chtype	*temp;
	register int		y;
	register chtype	*end;

	temp = win->_y[win->_maxy-1];
	win->_firstch[win->_cury] = 0;
	win->_lastch[win->_cury] = win->_maxx - 1;
	for (y = win->_maxy - 1; y > win->_cury; --y) {
	    if (win->_flags & _SUBWIN) {
		int i;
		for (i=0; i < win->_maxx; i++)
		    win->_y[y][i] = win->_y[y-1][i];
	    }
	    else
		win->_y[y] = win->_y[y-1];
	    win->_firstch[y] = 0;
	    win->_lastch[y] = win->_maxx - 1;
	}
	if (win->_flags & _SUBWIN) {
	    int i;
	    for (i=0; i < win->_maxx; i++)
		win->_y[win->_cury][i] = ' ';
	}
	else {
	    for (end = &temp[win->_maxx]; temp < end; )
		*temp++ = ' ';
	    win->_y[win->_cury] = temp - win->_maxx;
	}
	if (win->_cury == LINES - 1 && win->_y[LINES-1][COLS-1] != ' ')
		if (win->_scroll && !(win->_flags&_ISPAD)) {
			wrefresh(win);
			scroll(win);
			win->_cury--;
		}
		else
			return ERR;
	return OK;
}
