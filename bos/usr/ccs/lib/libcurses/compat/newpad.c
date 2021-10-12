static char sccsid[] = "@(#)69  1.1  src/bos/usr/ccs/lib/libcurses/compat/newpad.c, libcurses, bos411, 9428A410j 9/2/93 13:16:03";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: newpad
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
 * NAME:        newpad
 *
 * FUNCTION:
 *
 *      Like newwin, but makes a pad instead of a window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      A pad is not associated with part of the screen, so it can be
 *      bigger.
 */

WINDOW *
newpad(nlines, ncols)
register int	nlines;
{
	register WINDOW	*win;
	register chtype	*sp;
	register int i;
	char *calloc();

	if ((win = makenew(nlines, ncols, 0, 0)) == NULL)
		return NULL;
	win->_flags |= _ISPAD;
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
