static char sccsid[] = "@(#)55  1.1  src/bos/usr/ccs/lib/libcurses/compat/keypad.c, libcurses, bos411, 9428A410j 9/2/93 12:48:25";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   keypad
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

/*
 * NAME:        keypad
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => special keys should be passed as a single character by
 *      getch.
 */

keypad(win,bf)
WINDOW *win; int bf;
{
	win->_use_keypad = bf;
}
