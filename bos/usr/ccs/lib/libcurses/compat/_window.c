static char sccsid[] = "@(#)18  1.6  src/bos/usr/ccs/lib/libcurses/compat/_window.c, libcurses, bos411, 9428A410j 6/16/90 01:45:01";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _window
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

#include "cursesext.h"

/*
 * NAME:        _window
 *
 * FUNCTION:
 *
 *      Set the desired window to the box with the indicated boundaries.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      All scrolling should only affect the area inside the window.
 *      We currently ignore the last 2 args since we're only using this
 *      for scrolling and want to use the feature on vt100's as well as
 *      on concept 100's.  left and right are for future expansion someday.
 *
 *      Note that we currently assume cursor addressing within the window
 *      is relative to the screen, not the window.  This will have to be
 *      generalized if concept windows are to be used.
 */

_window(top, bottom, left, right)
int top, bottom, left, right;
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"_window old top=%d, bot %d; new top=%d, bot %d\n",
			SP->des_top_mgn, SP->des_bot_mgn, top, bottom);
#endif
	if (change_scroll_region || set_window) {
		SP->des_top_mgn = top;
		SP->des_bot_mgn = bottom;
	}
#ifdef DEBUG
	else
		if(outf) fprintf(outf, "window setting ignored\n");
#endif
}
