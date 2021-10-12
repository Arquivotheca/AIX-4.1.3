#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)47  1.1  src/bos/usr/ccs/lib/libcurses/winnstr.c, libcurses, bos411, 9428A410j 9/3/93 15:47:05";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: winnstr
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/winnstr.c	1.3"		*/



#include	"curses_inc.h"

/*
 * Copy n chars in window win from current cursor position to end
 * of window into char buffer str.  Return the number of chars copied.
 */

winnstr(win, str, ncols)
register	WINDOW	*win;
register	char	*str;
register	int	ncols;
{
    register	int	counter = 0;
    int			cy = win->_cury;
    register	chtype	*ptr = &(win->_y[cy][win->_curx]),
			*pmax = &(win->_y[cy][win->_maxx]);

    if (ncols == -1)
	ncols = MAXINT;

    while (counter < ncols)
    {
	str[counter++] = *ptr++ & A_CHARTEXT;

	if (ptr == pmax)
	{
	    if (++cy == win->_maxy)
		break;

	    ptr = &(win->_y[cy][0]);
	    pmax = ptr + win->_maxx;
	}
    }
    str[counter] = '\0';
    return (counter);
}
