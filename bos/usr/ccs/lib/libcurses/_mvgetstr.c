#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)31  1.1  src/bos/usr/ccs/lib/libcurses/_mvgetstr.c, libcurses, bos411, 9428A410j 9/3/93 15:03:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvgetstr
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

/* #ident	"@(#)curses:screen/_mvgetstr.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvgetstr(y, x, str)
int	y, x;
char	*str;
{
    return (wmove(stdscr, y, x)==ERR?ERR:wgetstr(stdscr, str));
}
