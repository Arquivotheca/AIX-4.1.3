static char sccsid[] = "@(#)14 1.3  src/bos/usr/bin/learn/dounit.c, cmdlearn, bos411, 9428A410j 3/22/93 13:16:33";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: dounit
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

int	remind = 2;		/* to remind user of "again" and "bye" */
extern	int	noclobber;

dounit()
{
	char tbuff[LINE_MAX+1];

	if (todo == 0)
		return;
	wrong = 0;
retry:
	if (!noclobber) {
		begin(todo);		/* clean up play directory */
	}
	sprintf(tbuff, "%s/%s/L%s", direct, sname, todo); /* script = lesson */
	scrin = fopen(tbuff, "r");
	if (scrin == NULL) {
		perror(tbuff);
		fprintf(stderr, MSGSTR(LDOUNIT, "Dounit:  no lesson %s.\n"), tbuff);
		wrapup(1);
	}

	copy(0, scrin);			/* print lesson, usually */
	if (more == 0)
		return;
	copy(1, stdin);			/* user takes over */
	if (skip)
		setdid(todo, sequence++);
	if (again || skip)		/* if "again" or "skip" */
		return;
	if (more == 0)
		return;
	copy(0, scrin);			/* evaluate user's response */

	if (comfile >= 0)
		close(comfile);
	wait(&didok);
	didok = (status == 0);
	if (!didok) {
		wrong++;
		if (wrong > 1)
			printf(MSGSTR(LSORRYSTILL, "\nSorry, that's still not right.  Do you want to try again?  "));
		else
			printf(MSGSTR(LSORRY, "\nSorry, that's not right.  Do you want to try again?  "));
		fflush(stdout);
		for(;;) {
			gets(tbuff);
			if (rpmatch(tbuff) == 1) {
				printf(MSGSTR(LTRYPROBAGN, "Try the problem again.\n"));
				if (remind--) {
					printf(MSGSTR(LWHENEVER, "[ Whenever you want to re-read the lesson, type \"again\".\n"));
					printf(MSGSTR(LYOUCANLEAVE, "  You can always leave learn by typing \"bye\". ]\n"));
				}
				goto retry;
			} else if (STRCMP(tbuff, MSGSTR(LBYE, "bye")) == 0) {
				wrapup(0);
			} else if (rpmatch(tbuff) == 0) {
				wrong = 0;
				printf(MSGSTR(LOKTHATWAS, "\nOK.  That was lesson %s.\n"), todo);
				printf(MSGSTR(LSKIPPING, "Skipping to next lesson.\n\n"));
				fflush(stdout);
				break;
			} else {
				printf(MSGSTR(LPLEASETYPE, "Please type yes, no or bye:  "));
				fflush(stdout);
			}
		}
	}
	setdid(todo, sequence++);
}
