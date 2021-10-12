#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)61  1.6  src/bos/usr/ccs/lib/libcurses/wattrset.c, libcurses, bos411, 9428A410j 9/3/93 14:50:05";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wattrset
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

/* #ident	"@(#)curses:screen/wattrset.c	1.10"		*/



#include	"curses_inc.h"

wattrset(win,a)
WINDOW	*win;
chtype	a;
{
    chtype temp_bkgd;

    /* if 'a' contains color information, then if we are not on color	*/
    /* terminal erase color information from 'a'		 	*/

    if ((a & A_COLOR) && (SP->_pairs_tbl == NULL))
	 a &= ~A_COLOR;

    /* combine 'a' with the background.  if 'a' contains color 		*/
    /* information delete color information from the background		*/

    temp_bkgd = (a & A_COLOR) ? (win->_bkgd & ~A_COLOR) : win->_bkgd;
    win->_attrs = (a | temp_bkgd) & A_ATTRIBUTES;
    return (1);
}
