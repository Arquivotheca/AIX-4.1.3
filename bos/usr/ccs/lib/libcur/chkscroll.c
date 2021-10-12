static char sccsid[] = "@(#)57	1.7  src/bos/usr/ccs/lib/libcur/chkscroll.c, libcur, bos411, 9428A410j 6/16/90 01:36:54";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: check_scroll
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

/*
 * NAME:                check_scroll
 *
 * FUNCTION: This routine is a common subroutine to
 *      check for scrolling characteristics in a window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      check_scroll(win, y), where 'win' is a pointer to the
 *      window, and 'y' is the line number currently in question.
 *
 * EXTERNAL REFERENCES: scroll()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

check_scroll(win, y)
register    WINDOW  *win;
register int    y;
{
    if (y > win->_bmarg)        /* protect against boundry conditions */
	if (win->_scroll) {	/* if window is allowed to scroll, */
	    scroll(win);	/* then we scroll it here          */
	    return(OK);
	}
	else
	    y = win->_bmarg;    /* set to lastline in the window */

/* if last line - last character has changed, check for scroll */
    if (y == win->_maxy - 1 && win->_lastch[y] >= win->_maxx - 1 &&
	    win->_firstch[y] != _NOCHANGE) {
	if (win->_scroll)	/* if window is allowed to scroll, */
	    scroll(win);	/* then we scroll it here          */
    /* else if !scroll and this is a _SCROLLWIN, then we recover */
	else
	    if (win->_flags & _SCROLLWIN) {
		if (win->_firstch[y] >= win->_maxx - 1)
		    win->_firstch[y] = win->_lastch[y] = _NOCHANGE;
		else
		    win->_lastch[y] = win->_maxx - 2;
		return ERR;
	    }
    }				/* end if last line ... */

    return OK;
}
