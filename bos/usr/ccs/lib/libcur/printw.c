static char sccsid[] = "@(#)22	1.10  src/bos/usr/ccs/lib/libcur/printw.c, libcur, bos411, 9428A410j 6/16/90 01:41:19";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: printw, wprintw, _sprw
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
 * NAME:                printw
 *
 * FUNCTION: print to the screen.
 */

printw(fmt, args)
char   *fmt;
int     args;
{
    return _sprw(stdscr, &fmt);
}

/*
 * NAME:                wprintw
 *
 * FUNCTION: These routines implement the printw
 *      statements.  They cannot be macros because of the variable number
 *      of arguements to any "printf" like statement.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wprintw(win, fmt, args), where 'win' is a pointer to the
 *      window, 'y' & 'x' are the new location coordinates, and 'fmt' &
 *      'args' are the "printf"-like format and arguements.
 *
 * EXTERNAL REFERENCES: waddstr(), vsprintf(), _doprnt()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wprintw(win, fmt, args)
register    WINDOW  *win;
char    *fmt;
int     args;
{
    return _sprw(win, &fmt);
}

/*
 * NAME:                _sprw
 *
 * FUNCTION:This routine actually executes the printf and adds it to the
 *      window.
 *      This is really a modified version of "sprintf".  As such,
 *      it assumes that sprintf interfaces with the other printf functions
 *      in a certain way.  If this is not how your system works, you
 *      will have to modify this routine to use the interface that your
 *      "sprintf" uses.
 */

_sprw(win, aptr)
register    WINDOW  *win;
register char   **aptr;
{
    char    buf[512];
#if    defined(IS1) || defined(IS2) || defined(V7)
    FILE strbuf;

    strbuf._flag = _IOWRT + _IOSTRG;
    strbuf._ptr = buf;
    strbuf._cnt = 512;
    _doprnt(aptr, &strbuf);
    *strbuf._ptr = '\0';
#else
    register char  *fmt;
    fmt = *aptr++;
    vsprintf((char *)buf, (const char *)fmt, (va_list) aptr);
#endif
    return waddstr(win, buf);
}
