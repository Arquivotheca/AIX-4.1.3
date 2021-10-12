static char sccsid[] = "@(#)89	1.11  src/bos/usr/ccs/lib/libcur/insch.c, libcur, bos411, 9428A410j 5/11/94 11:32:52";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: winsch
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                winsch
 *
 * FUNCTION: This routine inserts a character at the
 *      current line location in the window, and increments the current
 *      character location by one.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      winsch(win, c), where 'win' is a pointer to the window,
 *      and 'c' is the character to be inserted.
 *
 * EXTERNAL REFERENCES: getyx(), waddch()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

winsch(win, c)
register    WINDOW  *win;
wchar_t	c;
{
    register    ATTR    *ap1,
			*ap2;
    register    NLSCHAR *sp1,
			*sp2,
			*cur_loc;
    register int    y,
                    x;
    int         chrlen,
                len;

    getyx(win, y, x);		/* get the current location */

 /* insert before character at (x,y) position in window */
    if (x && is_second_byte (win->_y[y][x]))
	x = --win->_curx;

 /* set-up all the shifting pointers for the next operation */
    cur_loc = &win->_y[y][x];	/* address of current location */
    sp1 = &win->_y[y][win->_maxx - 1];/* init:  last character */
    ap1 = &win->_a[y][win->_maxx - 1];/* init:  last attribute */

    /* insert enough space to hold the character */
	chrlen = (((len = wcwidth(c)) <= 0) ? 1 : len);
    sp2 = sp1 - chrlen;
    ap2 = ap1 - chrlen;

    while (sp1 > cur_loc) {	/* shift characters & attributes over by 1
				   */
	*sp1-- = *sp2--;
	*ap1-- = *ap2--;
    }

    while (chrlen--)
	*cur_loc++ = ' ';       /* replace by an ascii character, since if
				   the last byte waddch will overwrite
				   happens to be the first byte of a two
				   byte character, it will replace the next
				   byte by a blank, which would be wrong. */

    waddch(win, c);             /* put the new character in at the current
				   location */

 /* if last character is first byte of a two byte character, replace it by
   the partial character indicator. */
    cur_loc = &win->_y[y][win->_maxx-1];
    if (is_first_byte (*cur_loc))
	*cur_loc = PART_CHAR;

 /* update the optimization arrays for this line */
    if (win->_firstch[y] == _NOCHANGE || win->_firstch[y] > x)
	win->_firstch[y] = x;
    win->_lastch[y] = win->_maxx - 1;

    return OK;
}
