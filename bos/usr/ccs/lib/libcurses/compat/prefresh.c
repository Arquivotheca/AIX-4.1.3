static char sccsid[] = "@(#)74  1.1  src/bos/usr/ccs/lib/libcurses/compat/prefresh.c, libcurses, bos411, 9428A410j 9/2/93 13:24:59";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: prefresh
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

/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

#include	"cursesext.h"

/*
 * NAME:        prefresh
 *
 * FUNCTION:
 *
 *      Like wrefresh but refreshing from a pad.
 */

prefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
WINDOW	*pad;
int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
{
	pnoutrefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow,
								smaxcol);
	return doupdate();
}
