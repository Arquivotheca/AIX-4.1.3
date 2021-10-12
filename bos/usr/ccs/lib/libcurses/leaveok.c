#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)56  1.6  src/bos/usr/ccs/lib/libcurses/leaveok.c, libcurses, bos411, 9428A410j 9/3/93 14:45:18";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: leaveok
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

/* #ident	"@(#)curses:screen/leaveok.c	1.5"		*/



#include	"curses_inc.h"

leaveok(win,bf)
WINDOW	*win;
int	bf;
{
    win->_leave = bf;
    return (OK);
}
