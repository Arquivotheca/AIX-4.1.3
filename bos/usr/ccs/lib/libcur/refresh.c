static char sccsid[] = "@(#)33	1.10  src/bos/usr/ccs/lib/libcur/refresh.c, libcur, bos411, 9428A410j 6/16/90 01:41:24";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wrefresh
 *
 * ORIGINS: 3, 10, 27
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

#include        "cur99.h"

extern short    _ly,
                _lx;		/* current glass cursor position */
extern char _curwin;		/* refresh() on curscr? */
extern  WINDOW  *_win;          /* global copy of 'win' pointer for mvcur
				*/

/*
 * NAME:                wrefresh
 *
 * FUNCTIONAL DESCRIPTION =     This routine is called to make the
 *      current screen (and the glass) look like 'win' over the area
 *      covered by 'win'.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wrefresh(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: domvcur(), _puts(), makech(), werase(),
 *                      touchwin(), putchar(), fflush()
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wrefresh(win)
register    WINDOW  *win;
{
    register short  wy;
    register int    retval = OK;

    if (_endwin) {		/* make sure we're in visual state */
	_tputvs(TI);
	_endwin = FALSE;
    }
				/* get current cursor position */
    _ly = curscr->_cury;	/* these 3 are global for makech.c */
    _lx = curscr->_curx;
    _win = win;

    _curwin = (win == curscr);	/* if we're working a refresh() on curscr 
				*/
 /* should we clear the screen before we start? */
    if (win->_clear || curscr->_clear || _curwin) {
	if ((win->_flags & _FULLWIN) || curscr->_clear) {
	    _puts(CL)
		_ly = _lx = 0;
	    if (!_curwin) {	/* we just cleared the glass, erase curscr
				   */
		curscr->_clear = FALSE;
		curscr->_curx = curscr->_cury = 0;
		curscr->_csbp &= (~sw_mask);
				/* all switch attributes are    */
				/* cleared by CL, others remain */
		werase(curscr);
	    }
	    touchwin(win);	/* make sure all of win gets updated */
	}
	win->_clear = FALSE;
    }
    if (!CA) {			/* if this terminal is not cursor
				   addressable... */
	if (win->_curx != 0)
	    eciopc(NLSNL);
	if (!_curwin)
	    werase(curscr);
    }
 /* for each line, make updates as necessary */
    for (wy = 0; wy < win->_maxy; wy++) {
	if (win->_firstch[wy] != _NOCHANGE) {
	    if (makech(win, wy) == ERR)
		retval = ERR;
	    win->_firstch[wy] = _NOCHANGE;
	}
    }

    if (_curwin)
	domvcur(_ly, _lx, win->_cury, win->_curx);
    else
	if (win->_leave) {
	    curscr->_cury = _ly;
	    curscr->_curx = _lx;
	    _ly -= win->_begy;
	    _lx -= win->_begx;
	    if (_ly >= 0 && _ly < win->_maxy && _lx >= 0 && _lx < win->_maxx){
		win->_cury = _ly;
		win->_curx = _lx;
	    }
	    else
		win->_cury = win->_curx = 0;
	}
    else {
	domvcur(_ly, _lx, win->_cury + win->_begy, win->_curx + win->_begx);
	curscr->_cury = win->_cury + win->_begy;
	curscr->_curx = win->_curx + win->_begx;
    }

    _win = NULL;
    eciofl(stdout);		/* flush stdout in case user is i/o
				   buffering */

    eciock();			/* check if error during refresh and retry
				        */

    if ((win->_csbp == NORMAL)||(curscr->_csbp != NORMAL)) {
	if (do_colors)
	    chg_attr_mode(~NORMAL, NORMAL);
	else
	    chg_attr_mode(curscr->_csbp, NORMAL);
	curscr->_csbp = NORMAL;
    }

    return retval;
}
