static char sccsid[] = "@(#)58	1.5  src/bos/usr/bin/dirname/dirname.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:51";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: dirname
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
 *
 */

/*
 * Standards: P1003.2/D12, XPG4
 */
#include	<stdio.h>
#include	<locale.h>

#include	"dirname_msg.h"
#define		MSGSTR(Num, Str)	catgets(catd, MS_DIRNAME, Num, Str)
static nl_catd		catd;

/*
 * NAME: dirname
 * FUNCTION: Returns the directory name of a string parameter
 *
 * Note: If more messages are added to the dirname command, the catopen() call
 *	 shall be moved out of the if statement to right after the setlocale().
 */
main(argc, argv)
char **argv;
{
	register char *p1, *p2, *p3;

	(void) setlocale (LC_ALL, "");

	if (strcmp(argv[1], "-?") == 0) {
		catd = catopen(MF_DIRNAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "Usage: dirname string\n"));
		exit(1);
	}

	if (strcmp(argv[1], "--") == 0) {
		argv++;
		argc--;
	}
		
	/* Defect 36064 - return . on dirname <ret> */
 	if (argc < 2) {
		puts(".");
		exit(0);
	}

	p1 = argv[1];
	p2 = p1;


	/**********
	  Get to the end of the string
	**********/
	while(*p1)
		p1++;
	p1--;

	/**********
	  backup over any trailing '/'
	**********/
	for (p1=p1; p1>p2; p1--)
		if (*p1 != '/')
			break;
	/*********
	  skip anything that isn't a '/'
	**********/
	for (p1=p1; p1>=p2; p1--)
		if (*p1 == '/')
			break;
	/**********
	  skip any more '/', 
	**********/
	for (p1=p1; p1>p2; p1--)
		if (*p1 != '/')
			break;

	*++p1 = '\0';

	if (*p2 == '\0') {
		*p2 = '.';
		*(p2+1) = '\0';
	}
	puts(p2);
	exit(0);
}
