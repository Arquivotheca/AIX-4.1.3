#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)05  1.9  src/bos/usr/ccs/lib/libcurses/touchwin.c, libcurses, bos411, 9428A410j 9/3/93 14:48:48";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: touchwin
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

/* #ident	"@(#)curses:screen/touchwin.c	1.6"		*/



#include	"curses_inc.h"

touchwin(win)
WINDOW	*win;
{
    return (wtouchln(win,0,win->_maxy,TRUE));
}
