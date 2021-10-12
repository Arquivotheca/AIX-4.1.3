static char sccsid[] = "@(#)55	1.7  src/bos/usr/ccs/lib/libcur/mvprintw.c, libcur, bos411, 9428A410j 6/16/90 01:40:51";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: mvprintw, mvwprintw
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

#include        "cur99.h"

/*
 * NAME:                mvprintw
 *
 * FUNCTION: move the cursor and print
 */

mvprintw(y, x, fmt, args)
register int    y,
                x;
char   *fmt;
int     args;
{
    return wmove(stdscr, y, x) == OK ? _sprw(stdscr, &fmt) : ERR;
}

/*
 * NAME:                mvwprintw
 *
 * FUNCTION: These routines implement the move-printw
 *      statements.  They cannot be macros because of the variable number
 *      of arguements to any "printf" like statement.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mvwprintw(win, y, x, fmt, args), where 'win' is a pointer
 *      to the window, 'y' & 'x' are the new location coordinates, and
 *      'fmt' & 'args' are the "printf"-like format and arguements.
 *
 * EXTERNAL REFERENCES: _sprw(), wmove()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

mvwprintw(win, y, x, fmt, args)
register    WINDOW  *win;
register int    y,
                x;
char    *fmt;
int     args;
{
    return wmove(win, y, x) == OK ? _sprw(win, &fmt) : ERR;
}
