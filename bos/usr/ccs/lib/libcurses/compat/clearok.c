static char sccsid[] = "@(#)39  1.1  src/bos/usr/ccs/lib/libcurses/compat/clearok.c, libcurses, bos411, 9428A410j 9/2/93 12:15:22";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   clearok
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
 * NAME:        clearok
 */

clearok(win,bf)	
WINDOW *win;
int bf;
{
#ifdef DEBUG
	if (win == stdscr)
		printf("it's stdscr: ");
	if (win == curscr)
		printf("it's curscr: ");
	if (outf) fprintf(outf, "clearok(%x, %d)\n", win, bf);
#endif
	if (win==curscr)
		SP->doclear = 1;
	else
		win->_clear = bf;
}
