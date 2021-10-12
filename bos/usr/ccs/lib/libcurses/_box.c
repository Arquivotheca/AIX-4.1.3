#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)92  1.1  src/bos/usr/ccs/lib/libcurses/_box.c, libcurses, bos411, 9428A410j 9/3/93 15:01:33";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		box
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

/* #ident	"@(#)curses:screen/_box.c	1.2"		*/



#define		NOMACROS
#include	"curses_inc.h"

box(win, v, h)
WINDOW	*win;
chtype	v, h;
{
    return (wborder(win, v, v, h, h,
			 (chtype) 0, (chtype) 0, (chtype) 0, (chtype) 0));
}
