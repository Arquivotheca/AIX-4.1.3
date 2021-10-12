#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)55  1.1  src/bos/usr/ccs/lib/libcurses/pechochar.c, libcurses, bos411, 9428A410j 9/3/93 15:10:49";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: pechochar
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

/* #ident	"@(#)curses:screen/pechochar.c	1.7"		*/



/*
 *  These routines short-circuit much of the innards of curses in order to get
 *  a single character output to the screen quickly!
 *
 *  pechochar(WINDOW *pad, chtype ch) is functionally equivalent to
 *  waddch(WINDOW *pad, chtype ch), prefresh(WINDOW *pad, `the same arguments
 *  as in the last prefresh or pnoutrefresh')
 */

#include	"curses_inc.h"

pechochar(pad, ch)
register	WINDOW	*pad;
chtype			ch;
{
    register WINDOW *padwin;
    int	     rv;

    /*
     * If pad->_padwin exists (meaning that p*refresh have been
     * previously called), call wechochar on it.  Otherwise, call
     * wechochar on the pad itself
     */

    if ((padwin = pad->_padwin) != NULL)
    {
	padwin->_cury = pad->_cury - padwin->_pary;
	padwin->_curx = pad->_curx - padwin->_parx;
	rv = wechochar (padwin, ch);
	pad->_cury = padwin->_cury + padwin->_pary;
	pad->_curx = padwin->_curx + padwin->_parx;
	return (rv);
    }
    else
        return (wechochar (pad, ch));
}
