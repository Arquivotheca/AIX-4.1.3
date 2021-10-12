static char sccsid[] = "@(#)77	1.8  src/bos/usr/ccs/lib/libcur/standout.c, libcur, bos411, 9428A410j 6/16/90 01:41:44";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: xstandout, xstandend
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
 * NAME:                xstandout
 *
 * FUNCTION: move the window in
 *      "standout" mode, where standout is any attribute pattern
 *      except NORMAL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      xstandout(win, mode), where 'win' is a pointer to the
 *      window, and 'mode' is the new standout pattern.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

xstandout(win, mode)
register    WINDOW  *win;
int     mode;
{
    win->_flags |= _STANDOUT;
    win->_csbp = (ATTR) mode;	/* Current Standout Bit Pattern */
    return OK;
}

/*
 * NAME:                xstandend
 *
 * FUNCTION: move the window out of "standout" mode.
 */

xstandend(win)
register    WINDOW  *win;
{
    win->_flags &= ~_STANDOUT;
    win->_csbp = (ATTR) NORMAL;
    return OK;
}
