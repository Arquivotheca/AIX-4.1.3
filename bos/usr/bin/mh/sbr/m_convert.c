static char sccsid[] = "@(#)63        1.6  src/bos/usr/bin/mh/sbr/m_convert.c, cmdmh, bos411, 9428A410j 3/27/91 17:48:12";
/* 
 * COMPONENT_NAME: CMDMH m_convert.c
 * 
 * FUNCTIONS: MSGSTR, attr, getnew, m_conv, m_convert 
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
/* static char sccsid[] = "m_convert.c	7.1 87/10/13 17:07:23"; */

/* m_convert.c - parse a message sequence and set SELECTED */

#include "mh.h"
#include <stdio.h>
#include <ctype.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	BADLST	(-1)
#define	BADMSG	(-2)
#define	BADRNG	(-3)
#define	BADNEW	(-4)
#define	BADNUM	(-5)

#define	FIRST	1
#define	LAST	2

#define	getnew(mp)	(mp -> hghmsg + 1)

static int  convdir;
static char *delimp;
static attr();
static m_conv();

/*  */

m_convert (mp, name)
register struct msgs *mp;
register char   *name;
{
    register int    first,
                    last;
    register char  *bp,
		   *cp;
    int     found,
            range,
            err,
            flags;

    switch (err = attr (mp, cp = name)) {
	case NOTOK: 
	    return 0;

	case OK: 
	    break;

	default: 
	    return 1;
    }

    found = 0;
    flags = mp -> msgflags & MHPATH ? EXISTS | SELECT_EMPTY : EXISTS;

    if ((mp -> msgflags & MHPATH) && strcmp (cp, "new") == 0)
	if ((err = first = getnew (mp)) <= 0)
	    goto badmsg;
	else
	    goto single;
    if (strcmp (cp, "all") == 0)
	cp = "first-last";
    if ((err = first = m_conv (mp, cp, FIRST)) <= 0)
	goto badmsg;
    if (*(cp = delimp) && *cp != '-' && *cp != ':') {
badelim: ;
	advise (NULLCP, MSGSTR(BADDEL, "illegal argument delimiter: `%c'(0%o)"), *delimp, *delimp); /*MSG*/
	return 0;
    }
    if (*cp == '-') {
	cp++;
	if ((err = last = m_conv (mp, cp, LAST)) <= 0) {
    badmsg: ;
	    switch (err) {
		case BADMSG: 
		    advise (NULLCP, MSGSTR(NOMES, "no %s message"), cp); /*MSG*/
		    break;

		case BADNUM: 
		    advise (NULLCP, MSGSTR(NOTHERE, "message %s doesn't exist"), cp); /*MSG*/
		    break;

		case BADRNG: 
		    advise (NULLCP, MSGSTR(OUTOFRAN, "message %s out of range 1-%d"), cp, mp -> hghmsg); /*MSG*/
		    break;

		case BADLST: 
	    badlist: ;
		    advise (NULLCP, MSGSTR(BADLIST, "bad message list %s"), name); /*MSG*/
		    break;

		case BADNEW:
		    advise (NULLCP, MSGSTR(FULL, "folder full, no %s message"), name); /*MSG*/
		    break;

		default: 
		    advise (NULLCP, MSGSTR(NOMAT, "no messages match specification")); /*MSG*/
	    }
	    return 0;
	}
	if (last < first)
	    goto badlist;
	if (*delimp)
	    goto badelim;
	if (first > mp -> hghmsg || last < mp -> lowmsg) {
    rangerr: ;
	    advise (NULLCP, MSGSTR(NONE, "no messages in range %s"), name); /*MSG*/
	    return 0;
	}
	if (last > mp -> hghmsg)
	    last = mp -> hghmsg;
	if (first < mp -> lowmsg)
	    first = mp -> lowmsg;
	}
    else
	if (*cp == ':') {
	    cp++;
	    if (*cp == '-') {
		convdir = -1;
		cp++;
	    }
	    else
		if (*cp == '+') {
		    convdir = 1;
		    cp++;
		}
	    if ((range = atoi (bp = cp)) == 0)
		goto badlist;
	    while (isdigit (*bp))
		bp++;
	    if (*bp)
		goto badelim;
	    if ((convdir > 0 && first > mp -> hghmsg)
		    || (convdir < 0 && first < mp -> lowmsg))
		goto rangerr;
	    if (first < mp -> lowmsg)
		first = mp -> lowmsg;
	    if (first > mp -> hghmsg)
		first = mp -> hghmsg;
	    for (last = first;
		    last >= mp -> lowmsg && last <= mp -> hghmsg;
		    last += convdir)
		if (mp -> msgstats[last] & EXISTS)
		    if (--range <= 0)
			break;
	    if (last < mp -> lowmsg)
		last = mp -> lowmsg;
	    if (last > mp -> hghmsg)
		last = mp -> hghmsg;
	    if (last < first) {
		range = last;
		last = first;
		first = range;
	    }
	}
	else {
	    if (!(mp -> msgflags & MHPATH))
		if (first > mp -> hghmsg
			|| first < mp -> lowmsg
			|| !(mp -> msgstats[first] & EXISTS)) {
		    if (strcmp (name, "cur") == 0 || strcmp (name, ".") == 0)
			advise (NULLCP, MSGSTR(NOMES, "no %s message"), name); /*MSG*/
		    else
			advise (NULLCP, MSGSTR(NOEXIST, "message %d doesn't exist"), first); /*MSG*/
		    return 0;
		}
    single: ;
	    last = first;
	    if (mp -> msgflags & MHPATH)
		mp -> msgstats[first] |= SELECT_EMPTY;
	}
    for (; first <= last; first++)
	if (mp -> msgstats[first] & flags) {
	    if (!(mp -> msgstats[first] & SELECTED)) {
		mp -> numsel++;
		mp -> msgstats[first] |= SELECTED;
		if (mp -> lowsel == 0 || first < mp -> lowsel)
		    mp -> lowsel = first;
		if (first > mp -> hghsel)
		    mp -> hghsel = first;
	    }
	    found++;
	}
    if (!found)
	goto rangerr;

    return 1;
}

/*  */

static  m_conv (mp, str, call)
register struct msgs *mp;
register char   *str;
register int     call;
{
    register int    i;
    register char  *cp,
                   *bp;
    char    buf[16];

    convdir = 1;
    cp = bp = str;
    if (isdigit (*cp)) {
	while (isdigit (*bp))
	    bp++;
	delimp = bp;
	return ((i = atoi (cp)) <= mp -> hghmsg ? i
		: *delimp || call == LAST ? mp -> hghmsg + 1
		: mp -> msgflags & MHPATH ? BADRNG : BADNUM);
    }

    bp = buf;
    while ((*cp >= 'a' && *cp <= 'z') || *cp == '.')
	*bp++ = *cp++;
    *bp++ = (int)NULL;
    delimp = cp;

    if (strcmp (buf, "first") == 0)
	return (mp -> hghmsg || !(mp -> msgflags & MHPATH)
		? mp -> lowmsg : BADMSG);

    if (strcmp (buf, "last") == 0) {
	convdir = -1;
	return (mp -> hghmsg || !(mp -> msgflags & MHPATH)
		? mp -> hghmsg : BADMSG);
    }

    if (strcmp (buf, "cur") == 0 || strcmp (buf, ".") == 0)
	return (mp -> curmsg > 0 ? mp -> curmsg : BADMSG);

    if (strcmp (buf, "prev") == 0) {
	convdir = -1;
	for (i = (mp -> curmsg <= mp -> hghmsg) ? mp -> curmsg - 1 : mp -> hghmsg;
		i >= mp -> lowmsg; i--) {
	    if (mp -> msgstats[i] & EXISTS)
		return i;
	}
	return BADMSG;
    }

    if (strcmp (buf, "next") == 0) {
	for (i = (mp -> curmsg >= mp -> lowmsg) ? mp -> curmsg + 1 : mp -> lowmsg;
		i <= mp -> hghmsg; i++) {
	    if (mp -> msgstats[i] & EXISTS)
		return i;
	}
	return BADMSG;
    }

    return BADLST;
}

/*  */

static  attr (mp, cp)
register struct msgs *mp;
register char   *cp;
{
    int     bits,
            found,
            inverted;
    register int    i,
                    j;
    register char  *dp;
    char hlds[NL_TEXTMAX];

    if (strcmp (cp, "cur") == 0)/* hack for "cur-xyz", etc. */
	return OK;

    if (inverted = (dp = m_find (nsequence)) && *dp && ssequal (dp, cp))
	cp += strlen (dp);

    bits = FFATTRSLOT;
    for (i = 0; mp -> msgattrs[i]; i++)
	if (strcmp (mp -> msgattrs[i], cp) == 0)
	    break;
    if (mp -> msgattrs[i] == NULL)
	return OK;

    found = 0;
    for (j = mp -> lowmsg; j <= mp -> hghmsg; j++)
	if ((mp -> msgstats[j] & EXISTS)
		&& inverted ? !(mp -> msgstats[j] & (1 << (bits + i)))
		: mp -> msgstats[j] & (1 << (bits + i))) {
	    if (!(mp -> msgstats[j] & SELECTED)) {
		mp -> numsel++;
		mp -> msgstats[j] |= SELECTED;
		if (mp -> lowsel == 0 || j < mp -> lowsel)
		    mp -> lowsel = j;
		if (j > mp -> hghsel)
		    mp -> hghsel = j;
	    }
	    found++;
	}
    if (found > 0)
	return found;

    strcpy (hlds, MSGSTR(SEQ, "sequence %s %s")); /*MSG*/
    advise (NULLCP, hlds, cp, inverted ? MSGSTR(OFULL, "full") : MSGSTR(EMPTY1, "empty")); /*MSG*/
    return NOTOK;
}
