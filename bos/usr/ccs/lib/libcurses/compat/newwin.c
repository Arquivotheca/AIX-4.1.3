static char sccsid[] = "@(#)70  1.1  src/bos/usr/ccs/lib/libcurses/compat/newwin.c, libcurses, bos411, 9428A410j 9/2/93 13:17:50";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: newwin
 *
 * ORIGINS: 3, 10, 27
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
# include	<signal.h>

char	*calloc();
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:
 *
 * FUNCTION:
 *
 *      Allocate space for and set up defaults for a new _window.
 */

WINDOW *
newwin(nlines, ncols, by, bx)
register int	nlines, ncols, by, bx;
{
	register WINDOW	*win;
	register chtype	*sp;
	register int i;
	char *calloc();

	if (by + nlines > LINES)
		nlines = LINES - by;
	if (bx + ncols > COLS)
		ncols = COLS - bx;

	if (nlines == 0)
		nlines = LINES - by;
	if (ncols == 0)
		ncols = COLS - bx;

	if ((win = makenew(nlines, ncols, by, bx)) == NULL)
		return NULL;
	for (i = 0; i < nlines; i++)
		if ((win->_y[i] = (chtype *) calloc(ncols, sizeof (chtype)))
								 == NULL) {
			register int j;

			for (j = 0; j < i; j++)
				cfree((char *)win->_y[j]);
			cfree((char *)win->_firstch);
			cfree((char *)win->_lastch);
			cfree((char *)win->_y);
			cfree((char *)win);
			return NULL;
		}
		else
			for (sp = win->_y[i]; sp < win->_y[i] + ncols; )
				*sp++ = ' ';
	return win;
}
