#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)54  1.1  src/bos/usr/ccs/lib/libcurses/_mvwinnstr.c, libcurses, bos411, 9428A410j 9/3/93 15:04:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwinnstr
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

/* #ident	"@(#)curses:screen/_mvwinnstr.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvwinnstr(win, y, x, s, n)
WINDOW *win;
int	y, x, n;
char	*s;
{
    return (wmove(win, y, x)==ERR?ERR:winnstr(win, s, n));
}
