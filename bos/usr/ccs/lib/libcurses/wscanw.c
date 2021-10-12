#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)17  1.6  src/bos/usr/ccs/lib/libcurses/wscanw.c, libcurses, bos411, 9428A410j 9/3/93 14:50:25";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wscanw
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

/* #ident	"@(#)curses:screen/wscanw.c	1.7"		*/



/*
 * scanw and friends
 *
 */

# include	"curses_inc.h"
# include	<varargs.h>

/*
 *	This routine implements a scanf on the given window.
 */
/*VARARGS*/
wscanw(va_alist)
va_dcl
{
	register WINDOW	*win;
	register char	*fmt;
	va_list	ap;

	va_start(ap);
	win = va_arg(ap, WINDOW *);
	fmt = va_arg(ap, char *);
	return vwscanw(win, fmt, ap);
}
