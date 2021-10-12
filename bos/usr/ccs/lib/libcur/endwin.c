static char sccsid[] = "@(#)44	1.8  src/bos/usr/ccs/lib/libcur/endwin.c, libcur, bos411, 9428A410j 6/16/90 01:39:52";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: endwin
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
 * NAME:                endwin
 *
 * FUNCTION: This routine returns the terminal to the
 *      state it started in, in preparation for exiting the user program.
 *
 * EXTERNAL REFERENCES: resetty(), _puts()
 *
 * DATA STRUCTURES: WINDOW (struct _win_st)
 *
 * RETURNS: normal -> OK            error -> ERR
 */

endwin() {

    resetty(FALSE);		/* reset but do not clear       */

    if (curscr) {
	_endwin = TRUE;
    }

    if (do_colors) {
	restore_colors();
    }

    return OK;
}
