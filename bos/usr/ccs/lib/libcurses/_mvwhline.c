#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)50  1.1  src/bos/usr/ccs/lib/libcurses/_mvwhline.c, libcurses, bos411, 9428A410j 9/3/93 15:04:02";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwhline
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

/* #ident	"@(#)curses:screen/_mvwhline.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvwhline(win, y, x, c, n)
WINDOW	*win;
int	y, x, n;
chtype	c;
{
    return (wmove(win, y, x)==ERR?ERR:whline(win, c, n));
}
