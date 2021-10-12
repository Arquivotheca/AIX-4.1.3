#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)29  1.7  src/bos/usr/ccs/lib/libcurses/clearok.c, libcurses, bos411, 9428A410j 9/3/93 14:43:46";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: clearok
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

/* #ident	"@(#)curses:screen/clearok.c	1.5"		*/



#include	"curses_inc.h"

clearok(win,bf)
WINDOW	*win;
int	bf;
{
    win->_clear = bf;
    return (OK);
}
