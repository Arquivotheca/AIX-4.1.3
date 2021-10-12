static char sccsid[] = "@(#)59	1.11  src/bos/usr/bin/yes/yes.c, cmdmisc, bos411, 9428A410j 11/12/93 11:33:22";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * NOTE: yes [expletive]
 *      yes repeatedly outputs expletive if given, otherwise it outputs
 *      the "yes" string for the current locale.  Termination is by
 *	interruption.
 */

#include <locale.h>
#include <langinfo.h>

main(argc, argv)
char **argv;
{
	char *ystr;

	(void ) setlocale(LC_ALL, "");

	if (argc > 1)
		ystr = argv[1];
	else {
		ystr = nl_langinfo(YESSTR);
		(void) strtok(ystr, ":");
	}

	for (;;)
		printf("%s\n", ystr);
}
