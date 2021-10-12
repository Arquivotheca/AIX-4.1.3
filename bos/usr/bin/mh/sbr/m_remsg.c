static char sccsid[] = "@(#)79  1.6  src/bos/usr/bin/mh/sbr/m_remsg.c, cmdmh, bos411, 9428A410j 3/27/91 17:48:36";
/* 
 * COMPONENT_NAME: CMDMH m_remsg.c
 * 
 * FUNCTIONS: MSGSTR, m_remsg 
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
/* static char sccsid[] = "m_remsg.c	7.1 87/10/13 17:10:53"; */

/* m_remsg.c - realloc a msgs structure */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


struct msgs *m_remsg (mp, lo, hi)
register struct msgs *mp;
int	lo,
	hi;
{
    int     msgnum;
#ifdef	MTR
    register short *sp,
		   *pp;
#endif	MTR

    if (lo == 0 && (lo = mp -> lowmsg) == 0)
	lo = 1;
    if (hi < mp -> hghmsg)
	hi = mp -> hghmsg + (MAXFOLDER - mp -> nummsg);
    if (hi <= mp -> hghmsg)
	hi = mp -> hghmsg + MAXFOLDER;
    if (lo == mp -> lowmsg && hi == mp -> hghmsg)
	return mp;

#ifndef	MTR
    mp = (struct msgs  *) realloc ((char *) mp, MSIZE (mp, lo, hi));
    if (mp == NULL)
	adios (NULLCP, MSGSTR(NORAFSTOR, "unable to re-allocate folder storage")); /*MSG*/
#else	MTR
    if ((sp = (short *) calloc ((unsigned) 1, MSIZEX (mp, lo, hi))) == NULL)
	adios (NULLCP, MSGSTR(NORAMSTOR, "unable to re-allocate messages storage")); /*MSG*/

    pp = sp - lo;
    if (pp < 0)
	adios (NULLCP, MSGSTR(BOTCH1, "m_remsg() botch -- you lose big[1]")); /*MSG*/
    for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
	pp[msgnum] = mp -> msgstats[msgnum];
    free ((char *) mp -> msgbase);
    mp -> msgstats = sp;
#endif	MTR
    mp -> lowoff = lo;
    mp -> hghoff = hi;
#ifdef	MTR
    mp -> msgstats = (mp -> msgbase = mp -> msgstats) - mp -> lowoff;
    if (mp -> msgstats < 0)
	adios (NULLCP, MSGSTR(BOTCH2, "m_remsg() botch -- you lose big[2]")); /*MSG*/
#endif	MTR
    for (msgnum = mp -> lowmsg - 1; msgnum >= lo; msgnum--)
	mp -> msgstats[msgnum] = (int)NULL;
    for (msgnum = mp -> hghmsg + 1; msgnum <= hi; msgnum++)
	mp -> msgstats[msgnum] = (int)NULL;

    return mp;
}
