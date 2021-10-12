static char sccsid[] = "@(#)22	1.6  src/bos/usr/ccs/lib/libcur/longname.c, libcur, bos411, 9428A410j 6/16/90 01:40:37";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: longname
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

extern char ttytype[];		/* global variable set up in setupterm  */

/*
 * NAME:                longname
 *
 * FUNCTION: This routine returns the long name of the
 *      terminal found in the terminfo database.
 *
 * EXTERNAL REFERENCES: strcpy()
 *
 * DATA STRUCTURES:     none
 *
 * RETURNS:             normal -> <char *>
 */

char   *
        longname () {
    register char  *bp;

    for (bp = &(ttytype[0]); *(bp++););/* find the end of the string   */

				/* back up to '|' or beginning  */
    while (*(--bp) != '|' && bp >= &(ttytype[0]));

    return(++bp);
}
