static char sccsid[] = "@(#)44	1.3  src/bos/usr/bin/mh/sbr/copy.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:31";
/* 
 * COMPONENT_NAME: CMDMH copy.c
 * 
 * FUNCTIONS: copy 
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

/* copy.c - copy a string and return pointer to NULL terminator */


char   *copy (from, to)
register char  *from,
               *to;
{
    while (*to++ = *from++)
	continue;

    return (to - 1);
}
