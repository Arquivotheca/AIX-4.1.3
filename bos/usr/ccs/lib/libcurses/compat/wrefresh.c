static char sccsid[] = "@(#)30  1.1  src/bos/usr/ccs/lib/libcurses/compat/wrefresh.c, libcurses, bos411, 9428A410j 9/2/93 14:23:17";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   wrefresh
 *
 * ORIGINS: 3, 10, 26, 27
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

# include	"cursesext.h"

/*
 * NAME:        wrefresh
 *
 * FUNCTION:
 *
 *      Put out window and update screen.
 *      Make the current screen look like "win" over the area covered by
 *      win.
 */

wrefresh(win)
WINDOW	*win;
{
	wnoutrefresh(win);
	return doupdate();
}
