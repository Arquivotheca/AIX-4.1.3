#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)05  1.6  src/bos/usr/ccs/lib/libcurses/subwin.c, libcurses, bos411, 9428A410j 9/3/93 14:48:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: subwin
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

/* #ident	"@(#)curses:screen/subwin.c	1.5"		*/



#include	"curses_inc.h"

WINDOW	*
subwin(win,l,nc,by,bx)
WINDOW	*win;
int	l,nc,by,bx;
{
    return (derwin(win,l,nc,by - win->_begy,bx - win->_begx));
}
