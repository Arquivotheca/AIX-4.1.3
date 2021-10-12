#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)94  1.7  src/bos/usr/ccs/lib/libcurses/mvwscanw.c, libcurses, bos411, 9428A410j 9/3/93 14:46:18";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwscanw
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

/* #ident	"@(#)curses:screen/mvwscanw.c	1.6"		*/



# include	"curses_inc.h"
# include	<varargs.h>

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 */

/*VARARGS*/
mvwscanw(va_alist)
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
	return wmove(win, y, x) == OK ? vwscanw(win, fmt, ap) : ERR;
}
