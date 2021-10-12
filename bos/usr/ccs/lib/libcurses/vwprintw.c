#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)16  1.1  src/bos/usr/ccs/lib/libcurses/vwprintw.c, libcurses, bos411, 9428A410j 9/3/93 15:14:56";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: vwprintw
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

/* #ident	"@(#)curses:screen/vwprintw.c	1.4"		*/



/*
 * printw and friends
 *
 */
# include	"curses_inc.h"
# include	<varargs.h>

/*
 *	This routine actually executes the printf and adds it to the window
 *
 *	This code now uses the vsprintf routine, which portably digs
 *	into stdio.  We provide a vsprintf for older systems that don't
 *	have one.
 */

/*VARARGS2*/
vwprintw(win, fmt, ap)
register WINDOW	*win;
register char * fmt;
va_list ap;
{
	char	buf[BUFSIZ];
	register int n;

	n = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (n == ERR)
		return ERR;
	return waddstr(win, buf);
}
