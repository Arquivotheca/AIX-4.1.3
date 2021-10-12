static char sccsid[] = "@(#)83	1.3  src/bos/usr/bin/mh/sbr/m_seqbits.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:53";
/* 
 * COMPONENT_NAME: CMDMH m_seqbits.c
 * 
 * FUNCTIONS: m_seqbits 
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


/* m_seqbits.c - return the sprintb() string for a sequence */

#include "mh.h"
#include <stdio.h>


char   *m_seqbits (mp)
register struct msgs *mp;
{
    int     bits;
    register int    i;
    static char buffer[BUFSIZ];

    bits = FFATTRSLOT;
    (void) strcpy (buffer, MBITS);
    for (i = 0; mp -> msgattrs[i]; i++)
	(void) sprintf (buffer + strlen (buffer), "%c%s",
		bits + i + 1, mp -> msgattrs[i]);

    return buffer;
}
