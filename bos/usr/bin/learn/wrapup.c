static char sccsid[] = "@(#)31 1.3  src/bos/usr/bin/learn/wrapup.c, cmdlearn, bos411, 9428A410j 3/22/93 13:27:48";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: wrapup
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

#include "signal.h"
#include "stdio.h"
#include "lrnref.h"

#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

extern char learnrc[];

wrapup(n)
int n;
{
	FILE *fp;
/* this routine does not use 'system' because it wants interrupts turned off */

	signal(SIGINT, SIG_IGN);
	chdir("..");
	if (fork() == 0) {
		signal(SIGHUP, SIG_IGN);
		execl("/usr/bin/rm", "rm", "-rf", dir, 0);
		perror("/usr/bin/rm");
		fprintf(stderr, MSGSTR(LCANTRM, "Wrapup:  can't find 'rm' command.\n"));
		exit(1);
	}
	if (n == -1)
		unlink(learnrc);
	else if (!n && todo) {
		if ((fp=fopen(learnrc, "w")) == NULL)
			exit(0);
		fprintf(fp, "%s %s %d\n", sname, todo, speed);
		fclose(fp);
	}
	printf(MSGSTR(LBYEDOT, "Bye.\n"));
	/* not only does this reassure user but it stalls for time while deleting directory */
	fflush(stdout);
	wait(0);
	exit(n);
}
