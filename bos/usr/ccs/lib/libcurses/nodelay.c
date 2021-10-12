#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)83  1.6  src/bos/usr/ccs/lib/libcurses/nodelay.c, libcurses, bos411, 9428A410j 9/3/93 14:46:47";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: nodelay
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

/* #ident	"@(#)curses:screen/nodelay.c	1.5"		*/



/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses_inc.h"

/*
 * TRUE => don't wait for input, but return -1 instead.
 */

nodelay(win,bf)
WINDOW *win; int bf;
{
    win->_delay = (bf) ? 0 : -1;
    return (OK);
}
