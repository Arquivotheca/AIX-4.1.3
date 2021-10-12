#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)46  1.1  src/bos/usr/ccs/lib/libcurses/winchstr.c, libcurses, bos411, 9428A410j 9/3/93 15:47:02";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: winchstr
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

/* #ident	"@(#)curses:screen/winchstr.c	1.3"		*/



#include	"curses_inc.h"

winchstr(win,str)
WINDOW	*win;
chtype	*str;
{
    return (winchnstr(win,str,win->_maxx - win->_curx));
}
