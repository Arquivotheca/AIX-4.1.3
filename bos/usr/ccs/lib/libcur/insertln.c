static char sccsid[] = "@(#)00	1.7  src/bos/usr/ccs/lib/libcur/insertln.c, libcur, bos411, 9428A410j 6/16/90 01:40:16";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: winsertln
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
 * NAME:                winsertln
 *
 * FUNCTION: This routine does an insert-line at the
 *      current window location, leaving that location unchanged (i.e.
 *      the new blank line becomes the current location).
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

winsertln(win)
register    WINDOW  *win;
{
    return(winsdel(TRUE, win));	/* invoke real insert function  */
}
