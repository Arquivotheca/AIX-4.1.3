static char sccsid[] = "@(#)26	1.6  src/bos/usr/bin/csh/hist.c, cmdcsh, bos411, 9428A410j 11/12/92 13:33:51";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: savehist enthist hfree dohist dohist1 phist
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include "sh.h"

void
savehist(struct wordent *sp)
{
	register struct Hist *hp, *np;
	int histlen;
	register uchar_t *cp;

	/* throw away null lines */
	if (sp->next->word[0] == '\n')
		return;

	cp = value("history");
	if (*cp == 0)
		histlen = 0;
	else {
		while (*cp && digit(*cp))
			cp++;
		if (*cp)
			set("history", (uchar_t *)"10");
		histlen = getn(value("history"));
	}
	for (hp = &Histlist; np = hp->Hnext;)
		if (eventno - np->Href >= histlen || histlen == 0)
			hp->Hnext = np->Hnext, hfree(np);
		else
			hp = np;
	enthist(++eventno, sp, 1);
}

struct Hist *
enthist(int event, register struct wordent *lp, bool docopy)
{
	register struct Hist *np;

	np = (struct Hist *)calloc(1, sizeof *np);
	np->Hnum = np->Href = event;
	if (docopy)
		copylex(&np->Hlex, lp);
	else {
		np->Hlex.next = lp->next;
		lp->next->prev = &np->Hlex;
		np->Hlex.prev = lp->prev;
		lp->prev->next = &np->Hlex;
	}
	np->Hnext = Histlist.Hnext;
	Histlist.Hnext = np;
	return (np);
}

void
hfree(register struct Hist *hp)
{

	freelex(&hp->Hlex);
	xfree((uchar_t *)hp);
}

void
dohist(uchar_t **vp)
{
	int n, rflg = 0, hflg = 0;
	if (getn(value("history")) == 0)
		return;
	if (setintr)
		sigrelse(SIGINT);
	vp++;
	while (*vp && *vp[0] == '-') {
		if (*vp && EQ(*vp, "-h")) 
			hflg++;
		else if (*vp && EQ(*vp, "-r")) 
			rflg++;
		else {
			char e[NL_TEXTMAX];
			sprintf (e,MSGSTR(M_OPTION,"Unknown option : %s"), *vp);
			setname((uchar_t *)"history");
			bferr(e);
		}
		vp++;
	}
	if (*vp)
		n = getn(*vp);
	else {
		n = getn(value("history"));
	}
	dohist1(Histlist.Hnext, &n, rflg, hflg);
}

void
dohist1(struct Hist *hp, int *np, int rflg, int hflg)
{
	bool print = (*np) > 0;
top:
	if (hp == 0)
		return;
	(*np)--;
	hp->Href++;
	if (rflg == 0) {
		dohist1(hp->Hnext, np, rflg, hflg);
		if (print)
			phist(hp, hflg);
		return;
	}
	if (*np >= 0)
		phist(hp, hflg);
	hp = hp->Hnext;
	goto top;
}

void
phist(register struct Hist *hp, int hflg)
{

	if (hflg == 0)
		printf("%6d\t", hp->Hnum);
	prlex(&hp->Hlex);
}
