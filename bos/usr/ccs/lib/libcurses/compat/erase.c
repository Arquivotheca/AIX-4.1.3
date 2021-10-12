static char sccsid[] = "@(#)50  1.7  src/bos/usr/ccs/lib/libcurses/compat/erase.c, libcurses, bos411, 9428A410j 8/30/90 17:04:26";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   werase
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
 * NAME:        werase
 *
 * FUNCTION:
 *
 *      This routine erases everything on the _window.
 */

werase(win)
register WINDOW	*win; {

	register int		y;
	register chtype	*sp, *end, *start, *maxx;
	register int		minx;

# ifdef DEBUG
	if(outf) fprintf(outf, "WERASE(%0.2o), _maxx %d\n", win, win->_maxx);
# endif
	for (y = 0; y < win->_maxy; y++) {
		minx = _NOCHANGE;
		maxx = NULL;
		start = win->_y[y];
		end = &start[win->_maxx];
		for (sp = start; sp < end; sp++) {
#ifdef DEBUG
			if (y == 23) if(outf) fprintf(outf,
				"sp %x, *sp %c %o\n", sp, *sp, *sp);
#endif
			if (*sp != ' ') {
				maxx = sp;
				if (minx == _NOCHANGE)
					minx = sp - start;
				*sp = ' ';
			}
		}
		if (minx != _NOCHANGE) {
			if (win->_firstch[y] > minx
			     || win->_firstch[y] == _NOCHANGE)
				win->_firstch[y] = minx;
			if (win->_lastch[y] < maxx - win->_y[y])
				win->_lastch[y] = maxx - win->_y[y];
		}
# ifdef DEBUG
if(outf) fprintf(outf,
	"WERASE: minx %d maxx %d _firstch[%d] %d, start %x, end %x\n",
		minx, maxx ? maxx-start : NULL, y, win->_firstch[y],
			start, end);
# endif
	}
	win->_curx = win->_cury = 0;

	return OK; /* P46613 */
}
