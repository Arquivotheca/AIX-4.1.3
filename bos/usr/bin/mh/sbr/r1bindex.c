static char sccsid[] = "@(#)04	1.2  src/bos/usr/bin/mh/sbr/r1bindex.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:08";
/* 
 * COMPONENT_NAME: CMDMH r1bindex.c
 * 
 * FUNCTIONS: r1bindex 
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


/* r1bindex.c - right plus 1 or beginning index */


char *r1bindex(str, chr)
register char *str;
register int chr;
{
    register char  *cp;

    for (cp = str; *cp; cp++)
	continue;
    --cp;
    while (cp >= str && *cp != chr)
	--cp;
    return (++cp);
}
