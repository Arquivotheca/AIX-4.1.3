static char sccsid[] = "@(#)12	1.9  src/bos/usr/ccs/lib/libcur/addstr.c, libcur, bos411, 9428A410j 5/14/91 17:00:26";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: waddstr
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                waddstr
 *
 * FUNCTION: This routine adds a string to the window
 *      at the current location.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      waddstr(win, str), where 'win' is a pointer to the window
 *      and 'str' is a pointer to the string to be added.
 *
 * EXTERNAL REFERENCES: waddch()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

waddstr(win, str)
register    WINDOW  *win;
register    char    *str;
{
    int		ret;
    size_t	len;
    size_t	len_count=0;
    wchar_t	wc;

    while (*str) { 
	 ret = mbtowc(&wc, str, MB_CUR_MAX);
	 if( ret < 0 )	return ERR;
	 if (waddch(win, wc) == ERR)
	      return ERR;	
	 str += ret;
    }
    return OK;
}
