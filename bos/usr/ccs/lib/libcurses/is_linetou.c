#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)37  1.1  src/bos/usr/ccs/lib/libcurses/is_linetou.c, libcurses, bos411, 9428A410j 9/3/93 15:09:33";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: is_linetouched
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

/* #ident	"@(#)curses:screen/is_linetou.c	1.3"		*/



#include	"curses_inc.h"

is_linetouched(win,line)
WINDOW	*win;
int	line;
{
    if (line < 0 || line >= win->_maxy)
        return (ERR);
    if (win->_firstch[line] == _INFINITY)
	return (FALSE);
    else
	return (TRUE);
}
