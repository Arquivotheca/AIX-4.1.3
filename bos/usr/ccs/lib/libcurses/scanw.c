#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)95  1.6  src/bos/usr/ccs/lib/libcurses/scanw.c, libcurses, bos411, 9428A410j 9/3/93 14:47:42";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: scanw
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

/* #ident	"@(#)curses:screen/scanw.c	1.7"		*/



/*
 * scanw and friends
 *
 */

# include	"curses_inc.h"
# include	<varargs.h>

/*
 *	This routine implements a scanf on the standard screen.
 */
/*VARARGS1*/
scanw(va_alist)
va_dcl
{
	register char	*fmt;
	va_list	ap;

	va_start(ap);
	fmt = va_arg(ap, char *);
	return vwscanw(stdscr, fmt, ap);
}
