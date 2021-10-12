static char sccsid[] = "@(#)46	1.9  src/bos/usr/ccs/lib/libcur/chgat.c, libcur, bos411, 9428A410j 4/26/93 10:17:11";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wchgat
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                wchgat
 *
 * FUNCTION: This routine changes the attributes of
 *      text already associated with a (portion of a) line of a window
 *      starting at the current location in the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      (wchgat(win, number, mode), where 'win' is a pointer to a
 *      window, 'number' is the number of characters to effect, and
 *      'mode' is the new attribute.)
 *       
 *      -> 'number' is the number of culumns of wchar_t.   Apr/19/91
 *
 * EXTERNAL REFERENCES: check_scroll(), getyx()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wchgat(win, number, mode)
register    WINDOW  *win;
register int    number;
register int    mode;
{
    register int    i,
                    y,
                    startx;

    getyx(win, y, startx);	/* get current window location */

    if (number < 0)
	return ERR;		/* guard against bad parameters */
    if (number == 0)
	return OK;

    if (startx + number > win->_maxx)/* handle boundry conditions in */
	number = win->_maxx - startx;/* a friendly manner            */

    /* expand the region to alter to include complete characters */
    if (is_second_byte (win->_y[y][startx])) {
	startx--;
	number++;
    }

    if (is_first_byte (win->_y[y][startx + number - 1]))
	number++;

/* deleted the if(win->_firstch[y]==_NOCHANGE)...dadada condition */

    for (i = startx; i < (startx + number); i++)
				/* add the new attributes */
				/* (but only if they are different) */
	if (win->_a[y][i] != (ATTR) mode) {
	    win->_a[y][i] = (ATTR) mode;
            if(win-> _firstch[y] == _NOCHANGE) /* checks this for no change */
            {
              win -> _firstch[y] = i;
              win -> _lastch[y]  = i;
            }
	    else 
            {
       	     if (win->_firstch[y] > i)
		win->_firstch[y] = i;
	     if (win->_lastch[y] < i)
		win->_lastch[y] = i;
            }
	}

    return(check_scroll(win, y));
				/* handle scrolling boundry conditions */
}
