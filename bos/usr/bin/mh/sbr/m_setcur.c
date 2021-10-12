static char sccsid[] = "@(#)86	1.3  src/bos/usr/bin/mh/sbr/m_setcur.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:05";
/* 
 * COMPONENT_NAME: CMDMH m_setcur.c
 * 
 * FUNCTIONS: m_setcur 
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


/* m_setcur.c - set "cur" */

#include "mh.h"
#include <stdio.h>


void m_setcur (mp, num)
register struct  msgs *mp;
register int     num;
{
    int     bits,
            public;
    register int    i;

    public = mp -> msgflags & READONLY ? 0 : 1;
    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], current) == 0) {
	    public = mp -> attrstats & (1 << (bits + i)) ? 0 : 1;
	    break;
	}

    if (!m_seqnew (mp, current, public))
	return;
    (void) m_seqadd (mp, current, mp -> curmsg = num, public);
}
