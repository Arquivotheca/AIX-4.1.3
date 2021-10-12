static char sccsid[] = "@(#)53  1.1  src/bos/usr/ccs/lib/libcurses/compat/idlok.c, libcurses, bos411, 9428A410j 9/2/93 12:43:50";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   idlok
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
 * NAME:        idlok
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => OK to use insert/delete line.
 *
 */

idlok(win,bf)
WINDOW *win;
int bf;
{
	win->_use_idl = bf;
	return OK; /* P46613 */
}
