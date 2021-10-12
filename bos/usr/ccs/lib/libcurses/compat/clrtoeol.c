static char sccsid[] = "@(#)32  1.8  src/bos/usr/ccs/lib/libcurses/compat/clrtoeol.c, libcurses, bos411, 9428A410j 8/30/90 17:03:26";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wclrtoeol
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
 * NAME:        wclrtoeol
 *
 * FUNCTION:
 *
 *      This routine clears up to the end of line.
 */

wclrtoeol(win)
register WINDOW	*win; {

	register chtype	*sp, *end;
	register int		y, x;
	register chtype	*maxx;
	register int		minx;

	y = win->_cury;
	x = win->_curx;
	end = &win->_y[y][win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_y[y][x];
	for (sp = maxx; sp < end; sp++)
		if (*sp != ' ') {
			maxx = sp;
			if (minx == _NOCHANGE)
				minx = sp - win->_y[y];
			*sp = ' ';
		}
	/*
	 * update firstch and lastch for the line
	 */
# ifdef DEBUG
if(outf) fprintf(outf, "CLRTOEOL: \
line %d minx = %d, maxx = %d, firstch = %d, lastch = %d, next firstch %d\n",
	 y, minx, maxx - win->_y[y], win->_firstch[y], win->_lastch[y],
		win->_firstch[y+1]);
# endif
	if (minx != _NOCHANGE) {
		if (win->_firstch[y] > minx || win->_firstch[y] == _NOCHANGE)
			win->_firstch[y] = minx;
		if (win->_lastch[y] < maxx - win->_y[y])
			win->_lastch[y] = maxx - win->_y[y];
	}

	return OK; /* P46613 */
}
