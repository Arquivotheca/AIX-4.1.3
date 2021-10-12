static char sccsid[] = "@(#)26 1.6  src/bos/usr/bin/learn/selsub.c, cmdlearn, bos411, 9428A410j 6/8/93 13:21:16";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: selsub, chknam, cntlessons
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
#include <stdlib.h>
#include "sys/types.h"
#include "sys/stat.h"
#include <dirent.h>
#include "lrnref.h"

#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

char learnrc[PATH_MAX];

selsub(argc,argv)
char *argv[];
{
	char ans1[LINE_MAX+1];
	static char ans2[LINE_MAX+1];
	static char dirname[PATH_MAX];
	static char subname[PATH_MAX];
	FILE *fp;
	char *home;

	if (argc > 1 && argv[1][0] == '-') {
		direct = argv[1]+1;
		argc--;
		argv++;
	}
	if (chdir(direct) != 0) {
		perror(direct);
		fprintf(stderr, MSGSTR(LCANTCD, "Selsub:  couldn't cd to non-standard directory\n"));
		exit(1);
	}
	direct = getcwd(0, PATH_MAX); /* D44521 */
	sname = argc > 1 ? argv[1] : 0;
	if (argc > 2) {
		strcpy (level=ans2, argv[2]);
		if (strcmp(level, "-") == 0)	/* no lesson name is - */
			ask = 1;
		else if (strcmp(level, "0") == 0)
			level = 0;
		else
			again = 1;	/* treat as if "again" lesson */
	}
	else
		level = 0;
	if (argc > 3 )
		speed = atoi(argv[3]);
	if ((home = getenv("HOME")) != NULL) {
		sprintf(learnrc, "%s/.learnrc", home);
		if ((fp=fopen(learnrc, "r")) != NULL) {
			char xsub[MAXNAMLEN], xlev[MAXNAMLEN]; int xsp;
			fscanf(fp, "%s %s %d", xsub, xlev, &xsp);
			fclose(fp);
			if (*xsub && *xlev && xsp >= 0	/* all read OK */
			    && (argc == 2 && strcmp(sname, xsub) == 0
			      || argc <= 1)) {
				strcpy(sname = subname, xsub);
				strcpy(level = ans2, xlev);
				speed = xsp;
				again = 1;
	PRINTF(MSGSTR(LTAKINGUP, "[ Taking up where you left off last time:  learn %s %s.\n"),
		sname, level);
	PRINTF(MSGSTR(LRMNRENETER, "%s\n  \"rm $HOME/.learnrc\", and re-enter with \"learn %s\". ]\n"),
		MSGSTR(LTOSTART, "  To start this sequence over leave learn by typing \"bye\", then"),
		sname);
			}
		}
	}
	if (!sname) {
		printf(MSGSTR(LTHESEAVAIL, "These are the available courses -\n"));
		list("Linfo");
		printf(MSGSTR(LWANTMOREINFO, "If you want more information about the courses,\n"));
		printf(MSGSTR(LWANTMOREINFO2, "or if you have never used 'learn' before,\n"));
		printf(MSGSTR(LWANTMOREINFO3, "press RETURN; otherwise type the name of\n"));
		printf(MSGSTR(LWANTMOREINFO4, "the course you want, followed by RETURN.\n"));
		fflush(stdout);
		gets(sname=subname);
		if (sname[0] == '\0') {
			list("Xinfo");
			do {
				printf(MSGSTR(LWHICHSUB, "\nWhich subject?  "));
				fflush(stdout);
				gets(sname=subname);
			} while (sname[0] == '\0');
		}
	}
	chknam(sname);
	total = cntlessons(sname);
	if (!level) {
		printf(MSGSTR(LSUBJ1, "If you were in the middle of this subject\n"));
		printf(MSGSTR(LSUBJ2, "and want to start where you left off, type\n"));
		printf(MSGSTR(LSUBJ3, "the last lesson number the computer printed.\n"));
		printf(MSGSTR(LSUBJ4, "If you don't know the number, type in a word\n"));
		printf(MSGSTR(LSUBJ5, "you think might appear in the lesson you want,\n"));
		printf(MSGSTR(LSUBJ6, "and I will look for the first lesson containing it.\n"));
		printf(MSGSTR(LSUBJ7, "To start at the beginning, just hit RETURN.\n"));
		fflush(stdout);
		gets(ans2);
		if (ans2[0]==0)
			strcpy(ans2,"0");
		else
			again = 1;
		level=ans2;
		getlesson();
	}

	/* make new directory for user to play in */
	if (chdir("/tmp") != 0) {
		perror("/tmp");
		fprintf(stderr, MSGSTR(LCANTCDPUB, "Selsub:  couldn't cd to public directory\n"));
		exit(1);
	}
	sprintf(dir=dirname, "pl%da", getpid());
	mkdir(dir, (mode_t)0755);
	if (chdir(dir) < 0) {
		perror(dir);
		fprintf(stderr, MSGSTR(LCOUNDNTPLAY, "Selsub:  couldn't make play directory with %s.\nBye.\n"), ans1);
		exit(1);
	}
	/* after this point, we have a working directory. */
	/* have to call wrapup to clean up */
	sprintf(ans1, "%s/%s/Init", direct, sname);
	if (access(ans1, 04) == 0) {
		sprintf(ans1, "%s/%s/Init %s", direct, sname, level);
		if (system(ans1) != 0) {
			printf(MSGSTR(LLEAVINGLRN, "Leaving learn.\n"));
			wrapup(1);
		}
	}
}

chknam(name)
char *name;
{
	if (access(name, 05) < 0) {
		printf(MSGSTR(LNOSUBORLESS, "Sorry, there is no subject or lesson named %s.\nBye.\n"), name);
		exit(1);
	}
}


cntlessons(sname)	/* return number of entries in lesson directory; */
char *sname;		/* approximate at best since I don't count L0, Init */
{			/* and lessons skipped by good students */
	struct dirent dbuf;
	register struct dirent *ep = &dbuf;	/* directory entry pointer */
	int n = 0;
	DIR *dp;

	if ((dp = opendir(sname)) == NULL) {
		perror(sname);
		wrapup(1);
	}
	for (ep = readdir(dp); ep != NULL; ep = readdir(dp)) {
		if (ep->d_ino != 0)
			n++;
	}
	closedir(dp);
	return (n - 2);				/* minus . and .. */
}
