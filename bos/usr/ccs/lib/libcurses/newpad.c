#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)16  1.6  src/bos/usr/ccs/lib/libcurses/newpad.c, libcurses, bos411, 9428A410j 9/3/93 14:46:24";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: newpad
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

/* #ident	"@(#)curses:screen/newpad.c	1.3"		*/



#include	"curses_inc.h"

WINDOW	*
newpad(l,nc)
int	l,nc;
{
    WINDOW	*pad;

    pad = newwin(l,nc,0,0);
    pad->_flags |= _ISPAD;
    return (pad);
}
