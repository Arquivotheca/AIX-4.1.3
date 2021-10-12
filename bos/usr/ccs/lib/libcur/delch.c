static char sccsid[] = "@(#)34	1.8  src/bos/usr/ccs/lib/libcur/delch.c, libcur, bos411, 9428A410j 5/14/91 17:00:56";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wdelch
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
 * NAME:                wdelch
 *
 * FUNCTION: This routine performs a delete character
 *      at the current location (line) in the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wdelch(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wdelch(win)
register        WINDOW  *win;
{
    register    ATTR    *ap1,
			*ap2;
    register    NLSCHAR *sp1,
			*sp2,
			*end;
    register    int     y;
    int         backup;
    int         start;

    y = win->_cury;		/* set y to current line */

    end = &win->_y[y][win->_maxx - 1];
				/* get ending address of this line */
    sp2 = &win->_y[y][win->_curx + 1];
				/* get address of next character on this
				   line */
    ap2 = &win->_a[y][win->_curx + 1];
				/* get address of next attribute on this
				   line */

    sp1 = sp2 - 1;
    ap1 = ap2 - 1;

    /* if on second byte of two byte character, back up one byte */
    if (backup = (win->_curx && is_second_byte (*sp1))) {
	sp1--;
	ap1--;
    } else if (is_first_byte (*sp1)) {  /* if on first byte of a two byte
					   character, start source for copy
					   one byte later. */
	sp2++;
	ap2++;
    }

    while (sp2 <= end) {        /* shift line by one */
	*sp1++ = *sp2++;
	*ap1++ = *ap2++;
    }

    *sp1 = NLSBLANK;		/* add a blank & _csbp for last (new)
				   character */
    *ap1 = win->_csbp;


    /* deleted two columns, so must add two blanks */
    if (sp2 - sp1 > 1) {
	*++sp1 = NLSBLANK;
	*++ap1 = win->_csbp;
    }

    win->_lastch[y] = win->_maxx - 1;
    start = backup ? win->_curx - 1 : win->_curx;
    if (win->_firstch[y] == _NOCHANGE || win->_firstch[y] > start)
	win->_firstch[y] = start;

    return OK;
}
