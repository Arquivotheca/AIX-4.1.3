static char sccsid[] = "@(#)39  1.5  src/bos/usr/ccs/lib/libcurses/compat/wstandout.c, libcurses, bos411, 9428A410j 6/16/90 01:55:15";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wstandout
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
 * NAME:        wstandout
 *
 * FUNCTION:
 *
 *      Enter standout mode.
 */

wstandout(win)
register WINDOW	*win;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "WSTANDOUT(%x)\n", win);
#endif

	win->_attrs |= A_STANDOUT;
	return 1;
}
