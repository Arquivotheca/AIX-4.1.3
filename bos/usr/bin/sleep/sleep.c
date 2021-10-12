static char sccsid[] = "@(#)37  1.10  src/bos/usr/bin/sleep/sleep.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:06";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.0
 */

/*
 *	sleep -- suspend execution for an interval
 */                                                                   

#include	<stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <nl_types.h>
#include "sleep_msg.h"
static nl_catd catd;
#define MSGSTR(c,d)	catgets(catd,MS_SLEEP,c,d)

main(argc, argv)
char **argv;
{
	int	c, n=0;
	char	*s;

	(void) setlocale (LC_ALL,"");
        catd = catopen(MF_SLEEP,NL_CAT_LOCALE);

	while (( c = getopt(argc, argv, "")) != EOF) {
        switch (c) {
	default:
		usage();
		break;

		}
	}

	if (optind +1 != argc) {
		usage();
	}
	s = argv[optind];
	while(c = *s++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, MSGSTR(BADCHAR,			/*MSG*/
				"sleep: bad character in argument\n"));	/*MSG*/
			exit(2);
		}
		n = n*10 + c - '0';
	}
	(void) sleep(n);	/* Don't worry how long we actually slept. */
	exit (0);
}

/*
 * NAME: usage
 *
 * FUNCTION: prints usage message
 */

static usage()
{
        fprintf(stderr, MSGSTR(USG, "usage: sleep time\n")); /*MSG*/
                exit(2);
}

