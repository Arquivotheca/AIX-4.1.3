static char sccsid[] = "@(#)96	1.3  src/bos/usr/bin/mh/sbr/peekc.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:40";
/* 
 * COMPONENT_NAME: CMDMH peekc.c
 * 
 * FUNCTIONS: peekc 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* peekc.c - peek at the next character in a stream */

#include "mh.h"
#include <stdio.h>


int	peekc(ib)
register FILE *ib;
{
    register int    c;

    c = getc (ib);
    (void) ungetc (c, ib);

    return c;
}
