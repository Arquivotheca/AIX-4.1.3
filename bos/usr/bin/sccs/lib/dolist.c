static char sccsid[] = "@(#)71 1.10 src/bos/usr/bin/sccs/lib/dolist.c, cmdsccs, bos411, 9428A410j 2/6/94 10:35:12";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: dolist, getasid
 *
 * ORIGINS: 3, 10, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

# include       "defines.h"

/* _DLS and _BR are the default message strings for the BR and DLS messages */

#define _BR "\nThe 2 limits of a delta range must be in ascending order. (co12)\n"

#define _DLS "\nThe delta list syntax is not correct. (co13)\n"

dolist(pkt,list,ch)
struct packet *pkt;
register char *list;
char ch;
{
	char *getasid();
	struct sid lowsid, highsid, sid;
	int n;

	while (*list) {
		list = getasid(list,&lowsid);
		if (*list == '-') {
			++list;
			list = getasid(list,&highsid);
			if (lowsid.s_br == 0) {
				if ((highsid.s_br || highsid.s_seq ||
					highsid.s_rel < lowsid.s_rel ||
					(highsid.s_rel == lowsid.s_rel &&
					highsid.s_lev < lowsid.s_lev)))
						fatal(MSGCO(BR,_BR));
				sid.s_br = sid.s_seq = 0;
				for (sid.s_rel = lowsid.s_rel; sid.s_rel <= 
                                    highsid.s_rel; sid.s_rel++) {
					sid.s_lev = (sid.s_rel == lowsid.s_rel      				       	    ? lowsid.s_lev : 1);
					if (sid.s_rel == highsid.s_rel)
					    for (;(n = sidtoser(&sid,pkt))&&
					        sid.s_lev <= highsid.s_lev;
						sid.s_lev++)
						    enter(pkt,ch,n,&sid);
					else
					    for ( ;n = sidtoser(&sid,pkt);
					        sid.s_lev++)
					            enter(pkt,ch,n,&sid);
				}
			}
			else {
				if (!(highsid.s_rel == lowsid.s_rel &&
					highsid.s_lev == lowsid.s_lev &&
					highsid.s_br == lowsid.s_br &&
					highsid.s_seq >= lowsid.s_seq))
						fatal(MSGCO(BR,_BR));
				for (; lowsid.s_seq <= highsid.s_seq &&
				    (n = sidtoser(&lowsid,pkt)); lowsid.s_seq++)
					enter(pkt,ch,n,&lowsid);
			}
		}
		else {
			if (n = sidtoser(&lowsid,pkt))
				enter(pkt,ch,n,&lowsid);
		}
		if (*list == ',')
			++list;
	}
}


char *
getasid(p,sp)
register char *p;
register struct sid *sp;
{
	register char *old;
	char *sid_ab();

	p = sid_ab(old = p,sp);
	if (old == p || sp->s_rel == 0)
		fatal(MSGCO(DLS, _DLS));  /* MSG */
	if (sp->s_lev == 0) {
		sp->s_lev = MAXR;
		if (sp->s_br || sp->s_seq)
			fatal(MSGCO(DLS, _DLS));  /* MSG */
	}
	else if (sp->s_br) {
		if (sp->s_seq == 0)
			sp->s_seq = MAXR;
	}
	else if (sp->s_seq)
		fatal(MSGCO(DLS, _DLS));  /* MSG */
	return(p);
}
