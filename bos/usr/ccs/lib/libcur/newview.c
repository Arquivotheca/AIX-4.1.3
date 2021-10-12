static char sccsid[] = "@(#)88	1.8  src/bos/usr/ccs/lib/libcur/newview.c, libcur, bos411, 9428A410j 6/16/90 01:41:05";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: newview
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

#include        "cur99.h"

/*
 * NAME:                newview
 *
 * FUNCTION: This routine creates a subwindow which is
 *      suitable for use as "viewport" on another window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      newview(orig, rows, cols), where 'orig' is a pointer to
 *      the window which we are going to make a viewport on, and 'rows' &
 *      'cols' are the size of the viewport.
 *
 * EXTERNAL REFERENCES: getyx(), subwin()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

WINDOW *
newview(orig, rows, cols)
register    WINDOW  *orig;
int     rows,
        cols;
{
    register    WINDOW  *vwin;
    register int    winrow,
                    wincol;

    if (orig->_flags & _ISVIEW)	/* don't allow a view of a view... */
	return(WINDOW *) ERR;	/* (it's a matter of principle...) */

    getyx(orig, winrow, wincol);/* get the current location in 'orig' */

				/* make the subwindow */
    if ((vwin = subwin(orig,	/* create subwin        */
		    rows,	/* size of subwin       */
		    cols,
		    winrow + orig->_begy,/* display origin       */
		    wincol + orig->_begx))
	    == NULL)		/* if error in subwin   */
	return(WINDOW *) ERR;	/* -- return error now  */

 /* now make the subwindow suitable for viewport scrolling */
    vwin->_winy = winrow;
    vwin->_winx = wincol;
    vwin->_view = orig;
    vwin->_flags |= _ISVIEW;
    vwin->_csbp = orig->_csbp;

    orig->_view = vwin;
    orig->_flags |= _HASVIEW;

    return vwin;
}
