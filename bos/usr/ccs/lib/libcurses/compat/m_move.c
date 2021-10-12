static char sccsid[] = "@(#)43  1.5  src/bos/usr/ccs/lib/libcurses/compat/m_move.c, libcurses, bos411, 9428A410j 6/16/90 01:49:13";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_move
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
 * NAME:        m_move
 *
 * FUNCTION:
 *
 *      Move to given location on stdscr.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Update curses notion of where we
 *      are (stdscr). This could be a macro
 *      and originally was, but if you do
 *          move(line++, 0)
 *      it will increment line twice, which is a lose.
 *
 *      mini.c contains versions of curses routines for minicurses.
 *      They work just like their non-mini counterparts but draw on
 *      std_body rather than stdscr.  This cuts down on overhead but
 *      restricts what you are allowed to do - you can't get stuff back
 *      from the screen and you can't use multiple windows or things
 *      like insert/delete line (the logical ones that affect the screen).
 */

m_move(row, col)
int row, col;
{
	stdscr->_cury=row;
	stdscr->_curx=col;
	_ll_move(row,col);
}
