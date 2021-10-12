static char sccsid[] = "@(#)44	1.6  src/bos/usr/ccs/lib/libcur/scanw.c, libcur, bos411, 9428A410j 6/16/90 01:41:29";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: scanw, wscanw, _sscans
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
 * NAME:                scanw
 *
 * FUNCTION: scan the stdscr.
 */

scanw(fmt, args)
char   *fmt;
int     args;
{
    return _sscans(stdscr, fmt, &args);
}

/*
 * NAME:                wscanw
 *
 * FUNCTION: These routines implement a scanf on the
 *      appropriate window.  They cannot be macros because of the variable
 *      number of arguements taken by "scanf"-like functions.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wscanw(win, fmt, args), where 'win' is a pointer to the
 *      window, and 'fmt' & 'args' are the format specification and
 *      arguements of the scanf.
 *
 * EXTERNAL REFERENCES: wgetstr(), strlen()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wscanw(win, fmt, args)
register    WINDOW  *win;
char    *fmt;
int     args;
{
    return _sscans(win, fmt, &args);
}


/*
 * NAME:                _sscans
 *
 * FUNCTION: This routine actually executes the scanf from the window.
 *
 *      This is really a modified version of "sscanf".  As such,
 *      it assumes that sscanf interfaces with the other scanf functions
 *      in a certain way.  If this is not how your system works, you
 *      will have to modify this routine to use the interface that your
 *      "sscanf" uses.
 */

_sscans(win, fmt, args)
register    WINDOW  *win;
char    *fmt;
int     *args;
{
#ifdef _IOLBF
    unsigned			/* newer stdio.h defines as	 */
				/* unsigned, else just char     */
# endif
    char    buf[100];
    FILE junk;

    junk._flag = _IOREAD	/* base flag for all stdio vers */
#ifdef _IOSTRG
	| _IOSTRG		/* for old stdio use this flag	 */
#endif
#ifdef _IOLBF
	| _IOLBF		/* new stdio uses this flag	 */
#endif
	;			/* this ; is the end of the	 */
				/* junk._flag assignment        */
    junk._base = buf;
    junk._ptr = buf;
    if (wgetstr(win, buf) == ERR)
	return ERR;
    junk._cnt = strlen(buf);
    return _doscan(&junk, fmt, args);
}
