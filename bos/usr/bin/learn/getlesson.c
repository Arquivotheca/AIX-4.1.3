static char sccsid[] = "@(#)15 1.4  src/bos/usr/bin/learn/getlesson.c, cmdlearn, bos411, 9428A410j 3/22/93 13:17:24";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: getlesson
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
#include <sys/limits.h>
#include "lrnref.h"
#include "learn_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

char *
getlesson()
{
	register char *p;
	char ans[PATH_MAX], line[LINE_MAX+1];
	int isnum, found, fd[2];
	FILE *fp;

	sprintf(ans, "%s/%s/L%s", direct, sname, level);
	if (access(ans, 04) == 0)		/* there is a file */
		return(level);
	isnum = 1;
	for (p=level; *p; p++)		/* accept:  (digit|dot)*anychar  */
		if (*p != '.' && (*p < '0' || *p > '9') && *(p+1) != '\0')
			isnum = 0;
	if (isnum) {
		strcpy(line, level);
		p = level;
		while (*p != '.' && *p >= '0' && *p <= '9')
			p++;
		*p = '\0';
		strcat(level, ".1a");
		sprintf(ans, "%s/%s/L%s", direct, sname, level);
		if (access(ans, 04) == 0) {	/* there is a file */
			printf(MSGSTR(LNOLESSON, "There is no lesson %s; trying lesson %s instead.\n\n"), line, level);
			return(level);
		}
		printf(MSGSTR(LTHEREISNOLESSON, "There is no lesson %s.\n"), line);
		return(0);
	}
	/* fgrep through lessons for one containing the string in 'level' */
	pipe(fd);
	if (fork() == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		sprintf(ans,"cd %s/%s ; fgrep '%s' L?.* L??.* L???.*", direct, sname, level);
		execl("/usr/bin/sh", "sh", "-c", ans, 0);
		perror("/usr/bin/sh");
		fprintf(stderr, MSGSTR(LGETLESSONCANT, "Getlesson:  can't do %s\n"), ans);
	}
	close(fd[1]);
	fp = fdopen(fd[0], "r");
	found = 0;
	while (fgets(line, LINE_MAX, fp) != NULL) {
		for (p=line; *p != ':'; p++) ;
		p++;
		if (*p == '#')
			continue;
		else {
			found = 1;
			break;
		}
	}
	/*fclose(fp);*/
	if (found) {
		*--p = '\0';
		strcpy(level, &line[1]);
		sprintf(ans, "%s/%s/L%s", direct, sname, level);
		if (access(ans, 04) == 0) {	/* there is a file */
			printf(MSGSTR(LTRYINGLESSON, "Trying lesson %s.\n\n"), level);
			return(level);
		}
	}
	printf(MSGSTR(LNOLESSONCONTAIN, "There is no lesson containing \"%s\".\n"), level);
	return(0);
}
