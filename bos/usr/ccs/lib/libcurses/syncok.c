#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)91  1.1  src/bos/usr/ccs/lib/libcurses/syncok.c, libcurses, bos411, 9428A410j 9/3/93 15:13:19";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: syncok
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

/* #ident	"@(#)curses:screen/syncok.c	1.3"		*/



#include	"curses_inc.h"

syncok(win,bf)
WINDOW	*win;
int	bf;
{
   return (win->_parent ? win->_sync = bf : ERR);
}
