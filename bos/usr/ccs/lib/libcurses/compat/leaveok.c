static char sccsid[] = "@(#)57  1.1  src/bos/usr/ccs/lib/libcurses/compat/leaveok.c, libcurses, bos411, 9428A410j 9/2/93 12:53:34";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: leaveok
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
 * NAME:        leaveok
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => OK to leave cursor where it happens to fall after refresh.
 */

leaveok(win,bf)
WINDOW *win; int bf;
{
	win->_leave = bf;
}
