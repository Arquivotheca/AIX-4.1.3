static char sccsid[] = "@(#)88	1.3  src/bos/usr/bin/mh/sbr/m_setseq.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:09";
/* 
 * COMPONENT_NAME: CMDMH m_setseq.c
 * 
 * FUNCTIONS: m_setseq 
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


/* m_setseq.c - set the previous-sequence */

#include "mh.h"
#include <stdio.h>


void m_setseq (mp)
register struct msgs *mp;
{
    register int    msgnum;
    register char  *cp,
                   *dp,
                  **ap;

    dp = NULL;
    if ((cp = m_find (psequence)) == NULL
	    || (ap = brkstring (dp = getcpy (cp), " ", "\n")) == NULL
	    || *ap == NULL) {
	if (dp)
	    free (dp);
	return;
    }

    for (; *ap; ap++)
	if (m_seqnew (mp, *ap, -1))
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    (void) m_seqadd (mp, *ap, msgnum, -1);

    if (dp)
	free (dp);
}
