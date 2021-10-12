static char sccsid[] = "@(#)77	1.3  src/bos/usr/bin/mh/sbr/m_name.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:31";
/* 
 * COMPONENT_NAME: CMDMH m_name.c
 * 
 * FUNCTIONS: m_name 
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


/* m_name.c - return a message number as a string */

#include "mh.h"
#include <stdio.h>


static char name[BUFSIZ];

char   *m_name (num)
register int     num;
{
    if (num <= 0)
	return "?";

    (void) sprintf (name, "%d", num);
    return name;
}
