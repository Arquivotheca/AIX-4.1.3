static char sccsid[] = "@(#)12	1.2  src/bos/usr/bin/mh/sbr/strindex.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:34";
/* 
 * COMPONENT_NAME: CMDMH strindex.c
 * 
 * FUNCTIONS: stringdex 
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


/* strindex.c - "unsigned" lexical index */


int  stringdex (p1, p2)
register char  *p1,
               *p2;
{
    register char  *p;

    for (p = p2; *p; p++)
	if (uprf (p, p1))
	    return (p - p2);

    return (-1);
}
