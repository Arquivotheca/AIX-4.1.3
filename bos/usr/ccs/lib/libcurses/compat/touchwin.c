static char sccsid[] = "@(#)98  1.1  src/bos/usr/ccs/lib/libcurses/compat/touchwin.c, libcurses, bos411, 9428A410j 9/2/93 14:05:35";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   touchwin, touchline
 *
 * ORIGINS: 3, 10, 26, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"

/*
 * NAME:        touchwin
 *
 * FUNCTION:
 *
 *      Make it look like the whole window has been changed.
 */

touchwin(win)
register WINDOW	*win;
{
	register int		y, maxy, maxx;

#ifdef DEBUG
	if (outf) fprintf(outf, "touchwin(%x)\n", win);
#endif
	maxy = win->_maxy;
	maxx = win->_maxx - 1;
	for (y = 0; y < maxy; y++)
	    touchline(win, y, 0, maxx);

}

/*
 * NAME:        touchline
 */

touchline(win, y, sx, ex)
register WINDOW *win;
register int    y, sx, ex;
{
	if (win->_firstch[y] == _NOCHANGE) {
		win->_firstch[y] = sx;
		win->_lastch[y] = ex;
	}
	else {
		if (win->_firstch[y] > sx)
			win->_firstch[y] = sx;
		if (win->_lastch[y] < ex)
			win->_lastch[y] = ex;
	}
}


