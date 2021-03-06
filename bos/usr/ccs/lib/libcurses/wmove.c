#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)25  1.1  src/bos/usr/ccs/lib/libcurses/wmove.c, libcurses, bos411, 9428A410j 9/3/93 15:32:58";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wmove
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

/* #ident	"@(#)curses:screen/wmove.c	1.2"		*/



#include	"curses_inc.h"

/* This routine moves the cursor to the given point */

wmove(win, y, x)
register	WINDOW	*win;
register	int	y, x;
{
#ifdef	DEBUG
    if (outf)
    {
	fprintf(outf, "MOVE to win ");
	if (win == stdscr)
	    fprintf(outf, "stdscr ");
	else
	    fprintf(outf, "%o ", win);
	fprintf(outf, "(%d, %d)\n", y, x);
    }
#endif	/* DEBUG */
    if (x < 0 || y < 0 || x >= win->_maxx || y >= win->_maxy)
	return (ERR);
    win->_curx = x;
    win->_cury = y;
    win->_flags |= _WINMOVED;
    return (win->_immed ? wrefresh(win) : OK);
}
