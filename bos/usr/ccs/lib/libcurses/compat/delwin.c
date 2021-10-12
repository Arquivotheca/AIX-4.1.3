static char sccsid[] = "@(#)43  1.1  src/bos/usr/ccs/lib/libcurses/compat/delwin.c, libcurses, bos411, 9428A410j 9/2/93 12:22:07";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   delwin
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
 * NAME:        delwin
 *
 * FUNCTION:
 *
 *      This routine deletes a _window and releases it back to the system.
 */

extern WINDOW *lwin;

delwin(win)
register WINDOW	*win; {

	register int	i;

	if (!(win->_flags & _SUBWIN))
		for (i = 0; i < win->_maxy && win->_y[i]; i++)
			cfree((char *) win->_y[i]);
	cfree((char *) win->_firstch);
	cfree((char *) win->_lastch);
	cfree((char *) win->_y);
	cfree((char *) win);
 
	/* if this was last win, don't care about refreshing it in doupdate */
	if (win == lwin) {
		lwin = NULL;
	}
	return OK; /* P46613 */
}
