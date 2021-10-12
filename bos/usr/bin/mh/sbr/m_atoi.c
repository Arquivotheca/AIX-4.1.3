static char sccsid[] = "@(#)61	1.3  src/bos/usr/bin/mh/sbr/m_atoi.c, cmdmh, bos411, 9428A410j 6/15/90 22:12:34";
/* 
 * COMPONENT_NAME: CMDMH m_atoi.c
 * 
 * FUNCTIONS: m_atoi 
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


/* m_atoi.c - parse a string representation of a message number */

#include "mh.h"


m_atoi (str)
register char *str;
{
    register int    i;
    register char  *cp;

    i = 0;
    cp = str;
    while (*cp) {
	if (*cp < '0' || *cp > '9')
	    return 0;
	i *= 10;
	i += *cp++ - '0';
    }

    return i;
}
