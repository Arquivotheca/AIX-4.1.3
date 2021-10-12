static char sccsid[] = "@(#)66	1.11  src/bos/usr/ccs/lib/libcur/getstr.c, libcur, bos411, 9428A410j 4/15/93 14:56:15";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wgetstr
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
#include        "cur02.h"

#include <stdlib.h>
#include <ctype.h>

/*
 * NAME:                wgetstr
 *
 * FUNCTION: This routine makes successive calls to
 *      wgetch() to get a string from the terminal.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wgetstr(win, str), where 'win' is a pointer to the window,
 *      and 'str' is a pointer to where the results are to be placed.
 *      Note that a common error is calling this routine when 'str' does
 *      not point to an area of static storage.  Stack storage is a no-no.
 *
 * EXTERNAL REFERENCES: wgetch()
 *
 * DATA STRUCTURES =    WINDOW (struct _win_st)
 * RETURNS:             normal -> OK            error -> ERR
 */

extern char _extended;

wgetstr(win, str)
register    WINDOW  *win;
register char  *str;
{
    wchar_t retcode;
    char extnd_save;
    int len;

    extnd_save = _extended;
    _extended = FALSE;
    while ((retcode=wgetch(win)) > 0 && /* while return is not err or null*/
	   !IS_PADKEY(retcode) &&       /* - nor a keypad item */
	    retcode != '\r' &&          /* - nor newline */
	    retcode != '\n') {          /* - (in either mode) */
	    *str++ = retcode;
    }
    _extended = extnd_save;

    if (retcode == (int)('\n') ||/* if end because of newline     */
	    retcode == (int)('\r') ||/*    or carriage return	 */
	    retcode == KEY_NEWL) {
	retcode = OK;		/* set return for normal end	 */
    }

    *str = '\0';		/* null terminate the string */

    return retcode;
}
