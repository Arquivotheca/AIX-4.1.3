static char sccsid[] = "@(#)85  1.6  src/bos/usr/bin/mh/sbr/m_seqnew.c, cmdmh, bos411, 9428A410j 3/27/91 17:50:54";
/* 
 * COMPONENT_NAME: CMDMH m_seqnew.c
 * 
 * FUNCTIONS: MSGSTR, m_seqadd, m_seqdel, m_seqnew, m_seqok 
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
/* static char sccsid[] = "m_seqnew.c	7.1 87/10/13 17:12:20"; */

/* m_seqnew.c - manage sequences */

#include "mh.h"
#include <ctype.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 
static int  m_seqok();


int     m_seqnew (mp, cp, public)
register struct msgs *mp;
register char   *cp;
register int	public;
{
    int     bits;
    register int    i,
                    j;

    if (!m_seqok (cp))
	return 0;

    if (public == -1)		/* XXX */
	public = mp -> msgflags & READONLY ? 0 : 1;

    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], cp) == 0) {
	    for (j = mp -> lowmsg; j <= mp -> hghmsg; j++)
		mp -> msgstats[j] &= ~(1 << (bits + i));
	    if (public)
		mp -> attrstats &= ~(1 << (bits + i));
	    else
		mp -> attrstats |= 1 << (bits + i);
	    mp -> msgflags |= SEQMOD;

	    return 1;
	}

    if (i >= NATTRS) {
	advise (NULLCP, MSGSTR(ONLY, "only %d sequences allowed (no room for %s)!"), NATTRS, cp); /*MSG*/
	return 0;
    }

    mp -> msgattrs[i] = getcpy (cp);
    for (j = mp -> lowmsg; j <= mp -> hghmsg; j++)
	mp -> msgstats[j] &= ~(1 << (bits + i));
    if (public)
	mp -> attrstats &= ~(1 << (bits + i));
    else
	mp -> attrstats |= 1 << (bits + i);
    mp -> msgflags |= SEQMOD;

    mp -> msgattrs[++i] = NULL;

    return 1;
}

/*  */

int     m_seqadd (mp, cp, j, public)
register struct msgs *mp;
register char   *cp;
register int     j,
		 public;
{
    int     bits;
    register int    i,
                    k;

    if (!m_seqok (cp))
	return 0;

    if (public == -1)		/* XXX */
	public = mp -> msgflags & READONLY ? 0 : 1;

    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], cp) == 0) {
	    mp -> msgstats[j] |= 1 << (bits + i);
	    if (public)
		mp -> attrstats &= ~(1 << (bits + i));
	    else
		mp -> attrstats |= 1 << (bits + i);
	    mp -> msgflags |= SEQMOD;

	    return 1;
	}

    if (i >= NATTRS) {
	advise (NULLCP, MSGSTR(ONLY, "only %d sequences allowed (no room for %s)!"), NATTRS, cp); /*MSG*/
	return 0;
    }

    mp -> msgattrs[i] = getcpy (cp);
    for (k = mp -> lowmsg; k <= mp -> hghmsg; k++)
	mp -> msgstats[k] &= ~(1 << (bits + i));
    mp -> msgstats[j] |= 1 << (bits + i);
    if (public)
	mp -> attrstats &= ~(1 << (bits + i));
    else
	mp -> attrstats |= 1 << (bits + i);
    mp -> msgflags |= SEQMOD;

    mp -> msgattrs[++i] = NULL;

    return 1;
}

/*  */

int     m_seqdel (mp, cp, j)
register struct msgs *mp;
register char   *cp;
register int     j;
{
    int     bits;
    register int    i;

    if (!m_seqok (cp))
	return 0;

    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], cp) == 0) {
	    mp -> msgstats[j] &= ~(1 << (bits + i));
	    mp -> msgflags |= SEQMOD;

	    return 1;
	}

    advise (NULLCP, MSGSTR(NOSUCH, "no such sequence as %s"), cp); /*MSG*/
    return 0;
}

/*  */

static int  m_seqok (cp)
register char   *cp;
{
    register char  *pp;

    if (cp == NULL || *cp == (int)NULL) {
	advise (NULLCP, MSGSTR(EMPSNM, "empty sequence name")); /*MSG*/
	return 0;
    }

    if (strcmp (cp, "new") == 0
#ifdef	notdef
	    || strcmp (cp, "cur") == 0
#endif	notdef
	    || strcmp (cp, "all") == 0
	    || strcmp (cp, "first") == 0
	    || strcmp (cp, "last") == 0
	    || strcmp (cp, "prev") == 0
	    || strcmp (cp, "next") == 0) {
	advise (NULLCP, MSGSTR(ILLSNM, "illegal sequence name: %s"), cp); /*MSG*/
	return 0;
    }

    if (!isalpha (*cp)) {
	advise (NULLCP, MSGSTR(ILLSNM, "illegal sequence name: %s"), cp); /*MSG*/
	return 0;
    }
    for (pp = cp + 1; *pp; pp++)
	if (!isalnum (*pp)) {
	    advise (NULLCP, MSGSTR(ILLSNM, "illegal sequence name: %s"), cp); /*MSG*/
	    return 0;
	}

    return 1;
}
