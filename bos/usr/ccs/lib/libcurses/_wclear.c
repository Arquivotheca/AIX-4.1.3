#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)90  1.1  src/bos/usr/ccs/lib/libcurses/_wclear.c, libcurses, bos411, 9428A410j 9/3/93 15:06:16";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wclear
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

/* #ident	"@(#)curses:screen/_wclear.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

wclear(win)
WINDOW	*win;
{
    win->_clear = TRUE;
    return (werase(win));
}
