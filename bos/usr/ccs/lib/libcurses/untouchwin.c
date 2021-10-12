#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)13  1.1  src/bos/usr/ccs/lib/libcurses/untouchwin.c, libcurses, bos411, 9428A410j 9/3/93 15:14:45";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: untouchwin
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

/* #ident	"@(#)curses:screen/untouchwin.c	1.3"		*/



#include	"curses_inc.h"

untouchwin(win)
WINDOW	*win;
{
    return (wtouchln(win,0,win->_maxy,FALSE));
}
