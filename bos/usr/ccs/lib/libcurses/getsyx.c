#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)23  1.1  src/bos/usr/ccs/lib/libcurses/getsyx.c, libcurses, bos411, 9428A410j 9/3/93 15:08:39";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _getsyx
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

/* #ident	"@(#)curses:screen/getsyx.c	1.7"		*/



#include	"curses_inc.h"

/*
 * Get the current screen coordinates (y, x).
 *
 * The current screen coordinates are defined as the last place that
 * the cursor was placed by a wnoutrefresh(), pnoutrefresh() or setsyx()
 * call. If leaveok() was true for the last window refreshed, then
 * return (-1, -1) so that setsyx() can reset the leaveok flag.
 *
 * This function is actually called by the macro getsyx(y, x), which is
 * defined in curses.h as:
 *
 * #define getsyx(y, x)	_getsyx(&y, &x)
 *
 * Note that this macro just adds in the '&'. In this way, getsyx()
 * is parallel with the other getyx() routines which don't require
 * ampersands. The reason that this can't all be a macro is that
 * that we need to access SP, which is normally not available in
 * user-level routines.
 */

_getsyx(yp, xp)
int	*yp, *xp;
{
    if (SP->virt_scr->_leave)
	*yp = *xp = -1;
    else
    {
	*yp = _virtscr->_cury - SP->Yabove;
	*xp = _virtscr->_curx;
    }
    return (OK);
}
