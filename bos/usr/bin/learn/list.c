static char sccsid[] = "@(#)34 1.4  src/bos/usr/bin/learn/list.c, cmdlearn, bos411, 9428A410j 3/22/93 13:19:59";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: list, stop
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
#include "signal.h"

int istop;

list(r)
char *r;
{
	int stop(void), intrpt(void);
	FILE *ft;
	char s[LINE_MAX+1];

	if (r==0)
		return;
	istop = 1;
	signal(SIGINT, (void (*)(int)) stop);
	ft = fopen(r, "r");
	if (ft != NULL) {
		while (fgets(s, LINE_MAX, ft) && istop)
			fputs(s, stdout);
		fclose(ft);
	}
	signal(SIGINT, (void (*)(int))intrpt);
}

stop(void)
{
	istop=0;
}
