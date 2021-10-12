static char sccsid[] = "@(#)68  1.1  src/bos/usr/ccs/lib/libcurses/compat/mvwscanw.c, libcurses, bos411, 9428A410j 9/2/93 13:14:23";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: mvwscanw
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

# include	"cursesext.h"

/*
 * NAME:        mvwscanw
 */

mvwscanw(win, y, x, fmt, args)
register WINDOW	*win;
register int		y, x;
char		*fmt;
int		args; {

	return wmove(win, y, x) == OK ? __sscans(win, fmt, &args) : ERR;
}
