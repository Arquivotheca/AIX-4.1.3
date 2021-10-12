static char sccsid[] = "@(#)39	1.2  src/bos/usr/bin/mh/sbr/atooi.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:16";
/* 
 * COMPONENT_NAME: CMDMH atooi.c
 * 
 * FUNCTIONS: atooi 
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

/* atooi.c - octal version of atoi() */


int atooi(cp)
register char *cp;
{
    register int    i,
                    base;

    i = 0;
    base = 8;
    while (*cp >= '0' && *cp <= '7') {
	i *= base;
	i += *cp++ - '0';
    }

    return i;
}
