static char sccsid[] = "@(#)16  1.1  src/bos/usr/ccs/lib/libcurses/compat/wattroff.c, libcurses, bos411, 9428A410j 9/2/93 14:17:21";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   wattroff
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
 * NAME:        wattroff
 *
 * FUNCTION:
 *
 *      Turn off selected attributes.
 */

wattroff(win, attrs)
register WINDOW	*win;
int attrs;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "WATTRON(%x, %o)\n", win, attrs);
#endif

	win->_attrs &= ~attrs;
	return 1;
}
