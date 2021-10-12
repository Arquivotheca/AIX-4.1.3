#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)83  1.7  src/bos/usr/ccs/lib/libcurses/mvwprintw.c, libcurses, bos411, 9428A410j 9/3/93 14:46:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwprintw
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

/* #ident	"@(#)curses:screen/mvwprintw.c	1.5"		*/



# include	"curses_inc.h"
# include	<varargs.h>

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */

/*VARARGS*/
mvwprintw(va_alist)
va_dcl
{
	register WINDOW	*win;
	register int	y, x;
	register char	*fmt;
	va_list ap;

	va_start(ap);
	win = va_arg(ap, WINDOW *);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
	return wmove(win, y, x) == OK ? vwprintw(win, fmt, ap) : ERR;
}
