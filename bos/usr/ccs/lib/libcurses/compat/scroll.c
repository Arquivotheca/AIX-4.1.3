static char sccsid[] = "@(#)06  1.5  src/bos/usr/ccs/lib/libcurses/compat/scroll.c, libcurses, bos411, 9428A410j 6/16/90 01:52:17";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   scroll
 *
 * ORIGINS: 3, 10, 26, 27
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

# include	"cursesext.h"

/*
 * NAME:        scroll
 *
 * FUNCTION:
 *
 *      This routine scrolls the window up a line.
 */

scroll(win)
WINDOW *win;
{
	_tscroll(win, 1);
}
