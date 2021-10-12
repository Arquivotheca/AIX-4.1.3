#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)44  1.1  src/bos/usr/ccs/lib/libcurses/winch.c, libcurses, bos411, 9428A410j 9/3/93 15:46:55";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: winch
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

/* #ident	"@(#)curses:screen/winch.c	1.2"		*/



#include	"curses_inc.h"

chtype
winch(win)
register	WINDOW	*win;
{
    return (win->_y[win->_cury][win->_curx]);
}
