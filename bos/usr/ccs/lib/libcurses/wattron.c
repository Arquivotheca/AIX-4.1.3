#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)50  1.6  src/bos/usr/ccs/lib/libcurses/wattron.c, libcurses, bos411, 9428A410j 9/3/93 14:49:56";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wattron
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

/* #ident	"@(#)curses:screen/wattron.c	1.10"		*/



#include	"curses_inc.h"

wattron(win,a)
WINDOW	*win;
chtype	a;
{
    /* if 'a' contains color information, then if we are on color terminal */
    /* erase color information from window attribute, otherwise erase      */
    /* color information from 'a'					   */

    if (a & A_COLOR)
        if (SP->_pairs_tbl)
            win->_attrs &= ~A_COLOR;
	else
	    a &= ~A_COLOR;

    win->_attrs |= (a & A_ATTRIBUTES);
    return (1);
}
