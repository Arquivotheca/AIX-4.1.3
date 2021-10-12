static char sccsid[] = "@(#)51  1.8  src/bos/usr/bin/whoami/whoami.c, cmdstat, bos41B, 9504A 12/21/94 13:41:58";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <pwd.h>
#include <locale.h>

#include "whoami_msg.h"

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_WHOAMI, Num, Str)

/*
 * whoami
 */

main()
{
	register struct passwd *pp;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_WHOAMI,NL_CAT_LOCALE);

	pp = getpwuid((uid_t)geteuid());
	if (pp == 0) {
		printf(MSGSTR(INTRUDER,"whoami: The user name is not recognized.\n"));
		exit(1);
	}
	printf("%s\n", pp->pw_name);
	exit(0);
}
