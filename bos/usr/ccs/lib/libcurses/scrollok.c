#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)17  1.8  src/bos/usr/ccs/lib/libcurses/scrollok.c, libcurses, bos411, 9428A410j 9/3/93 14:47:49";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: scrollok
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

/* #ident	"@(#)curses:screen/scrollok.c	1.7"		*/



#include	"curses_inc.h"

scrollok(win,bf)
WINDOW	*win;
int	bf;
{
    win->_scroll = (bf) ? TRUE : FALSE;
    return (OK);
}
