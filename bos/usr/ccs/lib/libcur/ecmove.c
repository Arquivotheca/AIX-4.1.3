static char sccsid[] = "@(#)12	1.1  src/bos/usr/ccs/lib/libcur/ecmove.c, libcur, bos411, 9428A410j 5/14/91 17:15:42";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecmove, winch
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cur99.h"

extern	short	_ly, _lx;	/* current glass cursor position */
extern  char    _curwin;        /* refresh() on curscr? */
extern	WINDOW	*_win;		/* global copy of 'win' pointer for mvcur */

/*
 * NAME:                ecmove
 *
 * FUNCTION:    Move physical cursor to indicated location.
 */

ecwmove (win, y, x)
register WINDOW *win;
register int y, x;
{
    /* the following four lines of code are from wmove in move.c */
    if (x >= win->_maxx || y >= win->_maxy || x < 0 || y < 0)
	return ERR;

    win->_curx = x; 	/* set the current location of this window... */
    win->_cury = y;

    _win = win;
    _ly = curscr->_cury;
    _lx = curscr->_curx;

    domvcur(_ly, _lx, win->_cury + win->_begy, win->_curx + win->_begx);
    _ly = curscr->_cury = win->_cury + win->_begy;
    _lx = curscr->_curx = win->_curx + win->_begx;
    eciofl(stdout);         /* flush stdout in case user is i/o buffering */
    return(OK);
}
