static char sccsid[] = "@(#)84	1.3  src/bos/usr/bin/mh/sbr/m_seqflag.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:57";
/* 
 * COMPONENT_NAME: CMDMH m_seqflag.c
 * 
 * FUNCTIONS: m_seqflag 
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


/* m_seqflag.c - return the flag (bit) for a sequence */

#include "mh.h"


int     m_seqflag (mp, cp)
register struct msgs *mp;
register char   *cp;
{
    int     bits;
    register int    i;

    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], cp) == 0)
	    return (1 << (bits + i));

    return 0;
}
