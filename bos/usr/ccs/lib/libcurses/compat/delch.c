static char sccsid[] = "@(#)54  1.6  src/bos/usr/ccs/lib/libcurses/compat/delch.c, libcurses, bos411, 9428A410j 6/16/90 01:46:33";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wdelch
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


/*
 * NAME:        wdelch
 *
 * FUNCTION:
 *
 *      This routine performs an delete-char on the line, leaving
 *      (_cury,_curx) unchanged.
 */

wdelch(win)
register WINDOW	*win; {

	register chtype	*temp1, *temp2;
	register chtype	*end;

	end = &win->_y[win->_cury][win->_maxx - 1];
	temp2 = &win->_y[win->_cury][win->_curx + 1];
	temp1 = temp2 - 1;
	while (temp1 < end)
		*temp1++ = *temp2++;
	*temp1 = ' ';
	win->_lastch[win->_cury] = win->_maxx - 1;
	if (win->_firstch[win->_cury] == _NOCHANGE ||
	    win->_firstch[win->_cury] > win->_curx)
		win->_firstch[win->_cury] = win->_curx;
	return OK;
}
