static char sccsid[] = "@(#)72  1.1  src/bos/usr/ccs/lib/libcurses/compat/nodelay.c, libcurses, bos411, 9428A410j 9/2/93 13:21:39";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: nodelay
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
 * NAME:        nodelay
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => don't wait for input, but return -1 instead.
 */

nodelay(win,bf)
WINDOW *win; int bf;
{
	_fixdelay(win->_nodelay, bf);
	win->_nodelay = bf;
}
