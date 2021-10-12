static char sccsid[] = "@(#)11 1.4  src/bos/usr/bin/learn/begin.c, cmdlearn, bos411, 9428A410j 11/15/93 09:31:57";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: begin
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

#include <stdio.h>
#include <stdlib.h>
#include "lrnref.h"
#include <sys/types.h>
#include <dirent.h>
#include <locale.h>
#include "learn_msg.h"
#include <ctype.h>
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

begin(lesson)
char *lesson;
{
	struct dirent dbuf;
	register struct dirent *ep = &dbuf;	/* directory entry pointer */
	int c, n;
	wchar_t temp;
	char where [PATH_MAX];
	DIR *dp;
	setlocale(LC_ALL, "");
	catd = catopen(MF_LEARN, NL_CAT_LOCALE);

	if (((dp = opendir(".")) == NULL)) {	/* clean up play directory */
		perror(MSGSTR(LSTPLYDIR, "Start:  play directory"));
		wrapup(1);
	}	
	for (ep = readdir(dp); ep != NULL; ep = readdir(dp)) {
		if (ep->d_ino == 0)
			continue;
		n = ep->d_namlen;
		if (ep->d_name[n-2] == '.' && ep->d_name[n-1] == 'c')
			continue;
		mbtowc(&temp, ep->d_name, MB_CUR_MAX);
		if (iswalpha(temp))
			unlink(ep->d_name);
	}
	closedir(dp);
	if (ask)
		return;
	sprintf(where, "%s/%s/L%s", direct, sname, lesson);
	if (access(where, 04)==0)	/* there is a file */
		return;
	perror(where);
	fprintf(stderr, MSGSTR(LSTRTNOLESS, "Start:  no lesson %s\n"),lesson);
	wrapup(1);
}
