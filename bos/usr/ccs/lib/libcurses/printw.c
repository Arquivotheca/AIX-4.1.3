#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)84  1.6  src/bos/usr/ccs/lib/libcurses/printw.c, libcurses, bos411, 9428A410j 9/3/93 14:47:06";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: printw
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

/* #ident	"@(#)curses:screen/printw.c	1.7"		*/



/*
 * printw and friends
 *
 */

# include	"curses_inc.h"
# include	<varargs.h>

/*
 *	This routine implements a printf on the standard screen.
 */
/*VARARGS1*/
printw(va_alist)
va_dcl
{
	register char * fmt;
	va_list ap;

	va_start(ap);
	fmt = va_arg(ap, char *);
	return vwprintw(stdscr, fmt, ap);
}
