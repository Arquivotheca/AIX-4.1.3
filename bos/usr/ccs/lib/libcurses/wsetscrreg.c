#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)29  1.1  src/bos/usr/ccs/lib/libcurses/wsetscrreg.c, libcurses, bos411, 9428A410j 9/3/93 15:33:10";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wsetscrreg
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

/* #ident	"@(#)curses:screen/wsetscrreg.c	1.3"		*/



#include	"curses_inc.h"

/*
 *	Change scrolling region. Since we depend on the values
 *	of tmarg and bmarg in various ways, this can no longer
 *	be a macro.
 */

wsetscrreg(win,topy,boty)
WINDOW	*win;
int	topy, boty;
{
    if (topy < 0 || topy >= win->_maxy || boty < 0 || boty >= win->_maxy)
	return (ERR);

    win->_tmarg = topy;
    win->_bmarg = boty;
    return (OK);
}
