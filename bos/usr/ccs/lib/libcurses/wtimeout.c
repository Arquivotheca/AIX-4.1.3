#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)32  1.1  src/bos/usr/ccs/lib/libcurses/wtimeout.c, libcurses, bos411, 9428A410j 9/3/93 15:33:19";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wtimeout
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

/* #ident	"@(#)curses:screen/wtimeout.c	1.3"		*/



#include	"curses_inc.h"

void
wtimeout(win,tm)
WINDOW	*win;
int	tm;
{
    win->_delay = tm;
}
