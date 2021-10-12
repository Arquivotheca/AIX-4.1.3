static char sccsid[] = "@(#)36 1.3  src/bos/usr/bin/learn/whatnow.c, cmdlearn, bos411, 9428A410j 3/22/93 13:26:55";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: whatnow
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

extern	char	togo[];
extern	int	review;

whatnow()
{
	if (again) {
		if (!review)
			printf(MSGSTR(LTHATWASLESS, "\nOK.  That was lesson %s.\n\n"), todo);
		fflush(stdout);
		strcpy(level, togo);
		return;
	}
	if (skip) {
		printf(MSGSTR(LOKTHATWAS, "\nOK.  That was lesson %s.\n"), todo);
		printf(MSGSTR(LSKIPPING, "Skipping to next lesson.\n\n"));
		fflush(stdout);
		strcpy(level, todo);
		skip = 0;
		return;
	}
	if (todo == 0) {
		more=0;
		return;
	}
	if (didok) {
		strcpy(level,todo);
		if (speed<=9) speed++;
	}
	else {
		speed -= 4;
		/* the 4 above means that 4 right, one wrong leave
		    you with the same speed. */
		if (speed <0) speed=0;
	}
	if (wrong) {
		speed -= 2;
		if (speed <0 ) speed = 0;
	}
	if (didok && more) {
		printf(MSGSTR(LGOODLESSON, "\nGood.  That was lesson %s.\n\n"),level);
		fflush(stdout);
	}
}
