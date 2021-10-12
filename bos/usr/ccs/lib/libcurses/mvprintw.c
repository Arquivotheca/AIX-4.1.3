#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)50  1.7  src/bos/usr/ccs/lib/libcurses/mvprintw.c, libcurses, bos411, 9428A410j 9/3/93 14:45:55";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvprintw
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

/* #ident	"@(#)curses:screen/mvprintw.c	1.5"		*/



# include	"curses_inc.h"
# include	<varargs.h>

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */

/*VARARGS*/
mvprintw(va_alist)
va_dcl
{
	register int	y, x;
	register char * fmt;
	va_list ap;

	va_start(ap);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
	return move(y, x) == OK ? vwprintw(stdscr, fmt, ap) : ERR;
}
