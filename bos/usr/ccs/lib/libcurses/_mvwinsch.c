#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)55  1.1  src/bos/usr/ccs/lib/libcurses/_mvwinsch.c, libcurses, bos411, 9428A410j 9/3/93 15:04:16";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwinsch
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

/* #ident	"@(#)curses:screen/_mvwinsch.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvwinsch(win, y, x, c)
WINDOW	*win;
int	y, x;
chtype	c;
{
    return (wmove(win, y, x)==ERR?ERR:winsch(win, c));
}
