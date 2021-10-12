#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)63  1.1  src/bos/usr/ccs/lib/libcurses/redrawwin.c, libcurses, bos411, 9428A410j 9/3/93 15:11:20";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: redrawwin
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

/* #ident	"@(#)curses:screen/redrawwin.c	1.3"		*/



#include	"curses_inc.h"

redrawwin(win)
WINDOW	*win;
{
    return (wredrawln(win,0,win->_maxy));
}
