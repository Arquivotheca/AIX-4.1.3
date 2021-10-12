static char sccsid[] = "@(#)27 1.3  src/bos/usr/bin/learn/selunit.c, cmdlearn, bos411, 9428A410j 3/22/93 13:26:03";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: selunit, abs, grand
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "stdio.h"
#include "lrnref.h"

#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

int	nsave	= 0;
int	review	= 0;

selunit()
{
	static char dobuff[50];
	static char saved[LINE_MAX+1];
	char fnam[PATH_MAX], s[PATH_MAX], zb[LINE_MAX+1];
	char posslev[LEN_L][LEN_L];
	int diff[LINE_MAX+1], i, k, m, n, best, alts;
	char *getlesson();
	FILE *f;

	if (again) {
		again = 0;
		if (todo=getlesson()) {
			if (!review)
				unsetdid(todo);
			return;
		}
		wrapup(1);
	}
	while (ask) {
		printf(MSGSTR(LWHATELSSN, "What lesson? "));
		fflush(stdout);
		gets(dobuff);
		if (STRCMP(dobuff, MSGSTR(LBYE, "bye")) == 0)
			wrapup(1);
		level = dobuff;
		if (todo=getlesson()) {
			return;
		}
	}
	alts = 0;
retry:
	f = scrin;			/* use old lesson to find next */
	if (f==NULL) {
		sprintf(fnam, "%s/%s/L%s", direct, sname, level);
		f = fopen(fnam, "r");
		if (f==NULL) {
			perror(fnam);
			fprintf(stderr, MSGSTR(LNOSCRIPT, "Selunit:  no script for lesson %s.\n"), level);
			wrapup(1);
		}
		while (fgets(zb, LINE_MAX, f)) {
			trim(zb);
			if (STRCMP(zb, MSGSTR(LPNEXT, "#next"))==0)
				break;
		}
	}
	if (feof(f)) {
		printf(MSGSTR(LCONGRAT, "Congratulations; you have finished this sequence.\n"));
		fflush(stdout);
		todo = 0;
		wrapup(-1);
	}
	for(i=0; fgets(s, LEN_MAX, f); i++) {
		sscanf(s, "%s %d", posslev[i], &diff[i]);
	}
	best = -1;
	/* cycle through lessons from random start */
	/* first try the current place, failing that back up to
	     last place there are untried alternatives (but only one backup) */
	n = grand()%i;
	for(k=0; k<i; k++) {
		m = (n+k)%i;
		if (already(posslev[m]))
			continue;
		if (best<0)
			best = m;
		alts++;				/* real alternatives */
		if (abs(diff[m]-speed) < abs(diff[best]-speed))
			best = m;
	}
	if (best < 0 && nsave) {
		nsave--;
		strcpy(level, saved);
		goto retry;
	}
	if (best < 0) {
		/* lessons exhausted or missing */
		printf(MSGSTR(LNOALTLESS, "Sorry, there are no alternative lessons at this stage.\n"));
		printf(MSGSTR(LGETHELP, "See someone for help.\n"));
		fflush(stdout);
		todo = 0;
		return;
	}
	strcpy (dobuff, posslev[best]);
	if (alts>1) {
		nsave = 1;
		strcpy(saved, level);
	}
	todo = dobuff;
	fclose(f);
}

abs(x)
{
	return(x>=0 ? x : -x);
}

grand()
{
	static int garbage;
	int a[2], b;

	time(a);
	b = a[1]+10*garbage++;
	return(b&077777);
}
