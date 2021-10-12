static char sccsid[] = "@(#)55	1.4  src/bos/usr/bin/uucp/uucpadm/build.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:48";
/* 
 * COMPONENT_NAME: CMDUUCP build.c
 * 
 * FUNCTIONS: build 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <string.h>
#include "defs.h"

extern char entries[MAXENT][MAXLINE];

int derror();

char *build(target)
struct files *target;
{
	static char tline[MAXEDIT];
	int count, i;
	count = 0;
	for (i=0;i < MAXENT;i++) {
		if (entries[i] != '\0')
			count += strlen(entries[i]) + 1;
	}
	if (count >= MAXEDIT)
			derror(EX_SOFTWARE,"Build line too long. Use editor.");
	for (i=0;i < MAXENT;i++) {
		if (entries[i][0] != '\0') {
			if (i == 0)
				(void) strcpy(tline,entries[i]);
			else
				(void) strcat(tline,entries[i]);
			(void) strncat(tline,target->delimit,(size_t) 1);
		}
		else
			break;
	}
	/* Remove trailing delimiter. */
	tline[count-1] = '\0';
	return(tline);
}
