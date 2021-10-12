static char sccsid[] = "@(#)23  1.5  src/bos/usr/ccs/lib/libcurses/compat/intrflush.c, libcurses, bos411, 9428A410j 6/16/90 01:48:23";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   intrflush
 *
 * ORIGINS: 3, 10, 27
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

#include "cursesext.h"

/*
 * NAME:        intrflush
 *
 * FUNCTION:
 *
 *      Flush input when an interrupt key is pressed.
 */

intrflush(win,bf)
WINDOW *win; int bf;
{
#ifdef USG
	if (bf)
		(cur_term->Nttyb).c_lflag &= ~NOFLSH;
	else
		(cur_term->Nttyb).c_lflag |= NOFLSH;
#else
	/* can't do this in 4.1BSD or V7 */
#endif
	reset_prog_mode();
}
