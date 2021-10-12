static char sccsid[] = "@(#)64	1.13  src/bos/usr/bin/basename/basename.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:25";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: basename
 *
 * ORIGINS: 26, 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

/*
 *
 * Standards: P1003.2/D12, XPG4
 *
 */
#include	<stdio.h>
#include	<locale.h>

#include	"basename_msg.h"
#define		MSGSTR(Num, Str)	catgets(catd, MS_BASENAME, Num, Str)
static  nl_catd		catd;

/*
 * NAME: basename
 * FUNCTION: Returns the base name of a string parameter
 * 
 * Note: If more messages are added to the basename command, the catopen()
 * 	 call shall be moved out of the if statement to right after the
 *	 setlocale().
 */
main(argc, argv)
int argc;
char **argv;
{
	register char *p1, *p2, *p3;

	(void) setlocale (LC_ALL, "");

	if (strcmp(argv[1], "-?") == 0) {
		catd = catopen(MF_BASENAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "Usage: basename string [suffix]\n"));
		exit(1);
	}

	if (strcmp(argv[1], "--") == 0) {
		argv++;
                argc--;
        }

	if (argc < 2 || argc > 3) {
		catd = catopen(MF_BASENAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "Usage: basename string [suffix]\n"));
		exit(1);
	}

	p1 = argv[1];
	p2 = p1;

	/**********
	  If there are trailing '/'s, remove them (POSIX 1003.2/D10)
	**********/
	while (*p1) {
		if (*p1 != '/')  {
			p2 = p1;
			while(*p1 && *p1 !='/')
				p1++;
			if (*p1)
				*p1++ = '\0';
		}
		else
			p1++;
	}

	/**********
	  if p2 is still equal to argv[1] then the string
	  could be all '/'s so output only one. (POSIX 1003.2/D10)
	**********/
	if ((p2 == argv[1]) && (*p2 == '/')) {
		p2[1] = '\0';
		goto output;
	}

	/**********
	  if there was a suffix, remove it unless it is the
	  entire remaing string, then leave it (POSIX 1003.2/D10)
	**********/
	if (argc>2) {
		for(p3=argv[2]; *p3; p3++) 
			;
		while(p1>p2 && p3>argv[2]) {
			if(*--p1 == '/')
				while(*p1 == '/')
					--p1;
			else
				*++p1; 	
			if(*--p3 != *--p1)
				goto output;
		}

		if (p1 != p2)
			*p1 = '\0';
	}
output:
	puts(p2);
	exit(0);
}
