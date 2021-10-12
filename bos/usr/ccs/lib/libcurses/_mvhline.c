#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)32  1.1  src/bos/usr/ccs/lib/libcurses/_mvhline.c, libcurses, bos411, 9428A410j 9/3/93 15:03:15";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvhline
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

/* #ident	"@(#)curses:screen/_mvhline.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvhline(y, x, c, n)
int	y, x, n;
chtype	c;
{
    return (mvwhline(stdscr, y, x, c, n));
}
