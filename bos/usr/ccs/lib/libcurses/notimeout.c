#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)48  1.1  src/bos/usr/ccs/lib/libcurses/notimeout.c, libcurses, bos411, 9428A410j 9/3/93 15:10:21";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: notimeout
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

/* #ident	"@(#)curses:screen/notimeout.c	1.6"		*/



#include	"curses_inc.h"

notimeout(win,bf)
WINDOW	*win;
int	bf;
{
    win->_notimeout = (bf) ? TRUE : FALSE;
    return (OK);
}
