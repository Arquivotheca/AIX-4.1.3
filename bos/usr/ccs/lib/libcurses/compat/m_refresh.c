static char sccsid[] = "@(#)54  1.5  src/bos/usr/ccs/lib/libcurses/compat/m_refresh.c, libcurses, bos411, 9428A410j 6/16/90 01:49:18";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_refresh
 *
 * ORIGINS: 3, 10, 26, 27
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

# include	"cursesext.h"
# include	<signal.h>

/*
 * NAME:        m_refresh
 *
 * FUNCTION:
 *
 *      Update the screen.  Like refresh but minicurses version.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mini.c contains versions of curses routines for minicurses.
 *      They work just like their non-mini counterparts but draw on
 *      std_body rather than stdscr.  This cuts down on overhead but
 *      restricts what you are allowed to do - you can't get stuff back
 *      from the screen and you can't use multiple windows or things
 *      like insert/delete line (the logical ones that affect the screen).
 */

m_refresh()
{
	/* Tell the back end where to leave the cursor */
	if (stdscr->_leave) {
		_ll_move(-1, -1);
	} else {
		_ll_move(stdscr->_cury+stdscr->_begy,
			stdscr->_curx+stdscr->_begx);
	}
	return _ll_refresh(stdscr->_use_idl);
}
