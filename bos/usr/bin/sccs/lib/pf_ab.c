static char sccsid[] = "@(#)67 1.11 src/bos/usr/bin/sccs/lib/pf_ab.c, cmdsccs, bos411, 9428A410j 3/28/94 19:13:37";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: pf_ab
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

# include       "defines.h"

pf_ab(s,pp,all)
char *s;
register struct pfile *pp;
int all;
{
	register char *p;
	register int i;
	extern char *Datep;
	char *xp, *sid_ab(), *fmalloc(), *strncpy();
	char stmp[MAXLINE];

	xp = p = stmp;
	strncpy(p, s, MAXLINE);
	for (; *p; p++)
		if (*p == '\n') {
			*p = 0;
			break;
		}
	p = xp;
	p = sid_ab(p,&pp->pf_gsid);
	++p;
	p = sid_ab(p,&pp->pf_nsid);
	++p;
	i = strcspn(p, " ");
	pp->pf_user[0] = 0;
	if (((unsigned)i) < LOGSIZE) {
		strncpy(pp->pf_user,p,i);
		pp->pf_user[i] = 0;
	}
	else
		fatal("bad p-file format (co17)");
	p = p + i + 1;
	date_ab(p,&pp->pf_date);
	p = Datep;
	pp->pf_ilist = 0;
	pp->pf_elist = 0;
	if (!all || !*p)
		return;
	p += 2;
	/* Buffer p does not have the whole string. It has only the first
	   MAXLINE characters. Actual string is in argument buffer s. So
	   point to the actual buffer and create a copy of it*/
	p = s + (p - stmp);
	xp = fmalloc(size(p));
	copy(p,xp);
	p = xp;
	for (; *p; p++)
		if (*p == '\n') {
			*p = 0;
			break;
		}
	p = xp;
	if (*p == 'i') {
		pp->pf_ilist = ++p;
		for (; *p; p++)
			if (*p == ' ') {
				*p++ = 0;
				p++;
				break;
			}
	}
	if (*p == 'x')
		{
		pp->pf_elist = ++p;
		for(;*p;p++)
			if(*p == ' ')
			{
				*p++ = 0;
				p++;
				break;
			}
	}
}
