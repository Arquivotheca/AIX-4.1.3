#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)44  1.1  src/bos/usr/ccs/lib/libcurses/_mvwaddchstr.c, libcurses, bos411, 9428A410j 9/3/93 15:03:46";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwaddchstr
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

/* #ident	"@(#)curses:screen/_mvwaddchstr.c	1.1"	*/



#define	NOMACROS

#include	"curses_inc.h"

mvwaddchstr(win, y, x, ch)
WINDOW	*win;
int	y, x;
chtype	*ch;
{
    return (wmove(win, y, x)==ERR?ERR:waddchstr(win, ch));
}
