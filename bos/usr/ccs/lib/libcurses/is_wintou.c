#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)38  1.1  src/bos/usr/ccs/lib/libcurses/is_wintou.c, libcurses, bos411, 9428A410j 9/3/93 15:09:37";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: is_wintouched
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

/* #ident	"@(#)curses:screen/is_wintou.c	1.1"		*/



#include	"curses_inc.h"

is_wintouched(win)
WINDOW	*win;
{
    return (win->_flags & _WINCHANGED);
}
