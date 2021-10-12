#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)45  1.1  src/bos/usr/ccs/lib/libcurses/_mvwaddnstr.c, libcurses, bos411, 9428A410j 9/3/93 15:03:49";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwaddnstr
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

/* #ident	"@(#)curses:screen/_mvwaddnstr.c	1.1"	*/



#define	NOMACROS

#include	"curses_inc.h"

mvwaddnstr(win, y, x, s, n)
WINDOW	*win;
int	y, x;
char	*s;
int	n;
{
    return (wmove(win, y, x)==ERR?ERR:waddnstr(win, s, n));
}
