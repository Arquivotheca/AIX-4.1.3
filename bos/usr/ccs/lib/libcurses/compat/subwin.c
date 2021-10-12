static char sccsid[] = "@(#)95  1.1  src/bos/usr/ccs/lib/libcurses/compat/subwin.c, libcurses, bos411, 9428A410j 9/2/93 13:58:12";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: subwin
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
 * NAME:        subwin
 */

WINDOW *
subwin(orig, num_lines, num_cols, begy, begx)
register WINDOW	*orig;
int	num_lines, num_cols, begy, begx;
{

	register int i;
	register WINDOW	*win;
	register int by, bx, nlines, ncols;
	register int j, k;

	by = begy;
	bx = begx;
	nlines = num_lines;
	ncols = num_cols;

	/*
	 * make sure window fits inside the original one
	 */
# ifdef	DEBUG
	if(outf) fprintf(outf,
		"SUBWIN(%0.2o, %d, %d, %d, %d)\n",
		orig, nlines, ncols, by, bx);
# endif
	if (by < orig->_begy || bx < orig->_begx
	    || by + nlines > orig->_begy + orig->_maxy
	    || bx + ncols  > orig->_begx + orig->_maxx)
		return NULL;
	if (nlines == 0)
		nlines = orig->_maxy - orig->_begy - by;
	if (ncols == 0)
		ncols = orig->_maxx - orig->_begx - bx;
	if ((win = makenew(nlines, ncols, by, bx)) == NULL)
		return NULL;
	j = by - orig->_begy;
	k = bx - orig->_begx;
	for (i = 0; i < nlines; i++)
		win->_y[i] = &orig->_y[j++][k];
	win->_flags = _SUBWIN;
	return win;
}
