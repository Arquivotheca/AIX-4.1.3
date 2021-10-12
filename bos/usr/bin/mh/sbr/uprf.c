static char sccsid[] = "@(#)16	1.2  src/bos/usr/bin/mh/sbr/uprf.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:49";
/* 
 * COMPONENT_NAME: CMDMH uprf.c
 * 
 * FUNCTIONS: uprf 
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


/* uprf.c - "unsigned" lexical prefix  */


uprf (c1, c2)
register char  *c1,
               *c2;
{
    register int    c;

    while (c = *c2++)
	if ((c | 040) != (*c1 | 040))
	    return 0;
	else
	    c1++;

    return 1;
}
