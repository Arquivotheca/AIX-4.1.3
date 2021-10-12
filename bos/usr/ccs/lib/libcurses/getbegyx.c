#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)19  1.1  src/bos/usr/ccs/lib/libcurses/getbegyx.c, libcurses, bos411, 9428A410j 9/3/93 15:08:07";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: getbegx
 *		getbegy
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

/* #ident	"@(#)curses:screen/getbegyx.c	1.3"		*/


#include	"curses_inc.h"

getbegy(win)
WINDOW	*win;
{
    return (win->_begy);
}

getbegx(win)
WINDOW	*win;
{
    return (win->_begx);
}
