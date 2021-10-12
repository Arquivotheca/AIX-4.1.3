static char sccsid[] = "@(#)55	1.10  src/bos/usr/ccs/lib/libcur/scroll.c, libcur, bos411, 9428A410j 5/19/93 11:50:41";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: scroll
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
 * NAME:                scroll
 *
 * FUNCTION: This routine scrolls the window up 1 line.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      scroll(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: putchar(), touchwin()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

scroll(win)
register    WINDOW  *win;
{
    register    NLSCHAR *sp,
			*temp;
    register int    i;
    register    ATTR    *ap,
			*temp1;
    register int    top,
                    bot;

    if (!win->_scroll || (win->_flags & _ISVIEW))
	return ERR;

    top = win->_tmarg;
    bot = win->_bmarg;

    temp = win->_y[top];
    temp1 = win->_a[top];

    for (i = top; i < bot; i++) {
	if (win->_flags & _SUBWIN) { /* need to do char by char copy */
	    int j;
	    for (j = 0; j < win->_maxx; j++) {
		win->_y[i][j] = win ->_y[i+1][j];
        	win->_a[i][j] = win ->_a[i+1][j];
	    }
        }	
	else {
	    win->_y[i] = win->_y[i+1];
	    win->_a[i] = win->_a[i+1];
	}
    }
    
    /* blank the new line at bottom of the window */
    if (win->_flags & _SUBWIN) { 
	int j;
	for (j = 0; j < win->_maxx; j++) {
	    win->_y[bot][j] = NLSBLANK;
	    win->_y[bot][j] = win->_csbp;
        }
    } 
    else {
	for (sp = temp, ap = temp1 ; sp - temp < win->_maxx; ) {
 	    *sp++ = NLSBLANK;
	    *ap++ = win->_csbp;
	}
	win->_y[bot] = temp;
	win->_a[bot] = temp1;
    }
    
    win->_cury--;

    if (win == curscr) {
	putchar('\n');
	if (!_pfast)
	    win->_curx = 0;
    }
    else
	touchwin(win);

    return OK;
}
