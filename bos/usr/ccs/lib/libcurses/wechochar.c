#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)40  1.1  src/bos/usr/ccs/lib/libcurses/wechochar.c, libcurses, bos411, 9428A410j 9/3/93 15:46:40";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wechochar
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

/* #ident	"@(#)curses:screen/wechochar.c	1.6"		*/



#include	"curses_inc.h"

/*
 *  These routines short-circuit much of the innards of curses in order to get
 *  a single character output to the screen quickly! It is used by getch()
 *  and getstr().
 *
 *  wechochar(WINDOW *win, chtype ch) is functionally equivalent to
 *  waddch(WINDOW *win, chtype ch), wrefresh(WINDOW *win)
 */

wechochar (win, ch)
register WINDOW *win;
chtype ch;
{
    int	saveimm = win->_immed, rv;

    immedok(win,TRUE);
    rv = waddch(win,ch);
    win->_immed = saveimm;
    return (rv);
}
