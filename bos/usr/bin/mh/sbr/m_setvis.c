static char sccsid[] = "@(#)89	1.3  src/bos/usr/bin/mh/sbr/m_setvis.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:13";
/* 
 * COMPONENT_NAME: CMDMH m_setvis.c
 * 
 * FUNCTIONS: m_setvis 
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


/* m_setvis.c - set the unseen-sequence */

#include "mh.h"
#include <stdio.h>


void m_setvis (mp, seen)
register struct msgs *mp;
int	seen;
{
    register int    msgnum;
    register char  *cp,
                   *dp,
                  **ap;

    dp = NULL;
    if ((cp = m_find (usequence)) == NULL
	    || (ap = brkstring (dp = getcpy (cp), " ", "\n")) == NULL
	    || *ap == NULL) {
	if (dp)
	    free (dp);
	return;
    }

    for (; *ap; ap++)
	if (seen) {
	    if (m_seqflag (mp, *ap))
		for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		    if (mp -> msgstats[msgnum] & UNSEEN)
			(void) m_seqdel (mp, *ap, msgnum);
	}
	else
	    for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
		if (mp -> msgstats[msgnum] & UNSEEN)
		    (void) m_seqadd (mp, *ap, msgnum, -1);

    if (dp)
	free (dp);
}
