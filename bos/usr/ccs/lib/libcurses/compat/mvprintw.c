static char sccsid[] = "@(#)63  1.1  src/bos/usr/ccs/lib/libcurses/compat/mvprintw.c, libcurses, bos411, 9428A410j 9/2/93 13:04:32";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: mvprintw
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
 * NAME:        mvprintw
 */

mvprintw(y, x, fmt, args)
register int		y, x;
char		*fmt;
int		args; {

	return move(y, x) == OK ? _sprintw(stdscr, fmt, &args) : ERR;
}
