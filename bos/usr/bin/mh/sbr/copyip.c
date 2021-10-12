static char sccsid[] = "@(#)45	1.2  src/bos/usr/bin/mh/sbr/copyip.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:35";
/* 
 * COMPONENT_NAME: CMDMH copyip.c
 * 
 * FUNCTIONS: copyip 
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

/* copyip.c - copy a string array and return pointer to end */


char  **copyip (p, q)
register char  **p,
	       **q;
{
    while (*p)
	*q++ = *p++;
    *q = 0;

    return q;
}
