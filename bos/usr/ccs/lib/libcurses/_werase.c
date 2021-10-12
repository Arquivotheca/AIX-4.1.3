#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)92  1.1  src/bos/usr/ccs/lib/libcurses/_werase.c, libcurses, bos411, 9428A410j 9/3/93 15:06:22";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: werase
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

/* #ident	"@(#)curses:screen/_werase.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

werase(win)
WINDOW	*win;
{
    (void) wmove(win, 0, 0);
    return (wclrtobot(win));
}
