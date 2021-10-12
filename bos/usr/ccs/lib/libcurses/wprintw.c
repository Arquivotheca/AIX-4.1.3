#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)83  1.6  src/bos/usr/ccs/lib/libcurses/wprintw.c, libcurses, bos411, 9428A410j 9/3/93 14:50:12";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wprintw
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

/* #ident	"@(#)curses:screen/wprintw.c	1.7"		*/



/*
 * printw and friends
 *
 */

# include	"curses_inc.h"
# include	<varargs.h>

/*
 *	This routine implements a printf on the given window.
 */
/*VARARGS*/
wprintw(va_alist)
va_dcl
{
	va_list ap;
	register WINDOW	*win;
	register char * fmt;

	va_start(ap);
	win = va_arg(ap, WINDOW *);
	fmt = va_arg(ap, char *);
	return vwprintw(win, fmt, ap);
}
