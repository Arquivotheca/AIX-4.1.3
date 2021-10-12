static char sccsid[] = "@(#)92  1.1  src/bos/usr/ccs/lib/libcurses/compat/scrollok.c, libcurses, bos411, 9428A410j 9/2/93 13:52:57";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: scrollok
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
 * NAME:        scrollok
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => OK to scroll screen up when you run off the bottom.
 *
 */

scrollok(win,bf)
WINDOW *win;
int bf;
{
	/* Should consider using scroll/page mode of some terminals. */
	win->_scroll = bf;
	
	return OK; /*P46613*/
}
