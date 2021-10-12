static char sccsid[] = "@(#)10  1.15  src/bos/usr/bin/unexpand/unexpand.c, cmdfiles, bos412, 9446C 11/14/94 16:47:11";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: unexpand
 *
 * ORIGINS: 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "expand_msg.h" 

static nl_catd catd;
#define MSGSTR(num,str) catgets(catd, MS_EXPAND, num, str) 

static char tabstops[LINE_MAX + 1];/* Array to hold tablist specified with -t option */
static int nstops = 0;

/*
 * NAME: unexpand [-a | -t tablist] [file ...]
 *
 * FUNCTION: Replace blanks with tabs. Leave trailing blanks/tabs
 *         at the end of a line.
 *
 * NOTE:   If -a is not used then only leading blanks are replaced
 *	   with tabs in each line.  If -t tablist is used, then blanks
 *	   in the specified columns will be replaced with tabs.
 */

/* Function prototypes */
static void		getstops(register char *cp);
static void 		tabify(int);
static void 		tabify_mb(int);
static void 		getstops(char *);

main(int argc, char *argv[])
{
	register int all = 0;
	int fastpath, option, errflag = 0;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_EXPAND, NL_CAT_LOCALE);
	fastpath = MB_CUR_MAX == 1;

	while ((option = getopt(argc, argv, "at:")) != -1) {
		switch (option) {
		case 'a':
			all = 1;
			break;
		case 't':
			/* Get the column positions in which blanks are to  */
			/* converted to tabs.				    */
			getstops(optarg);
			all = 1;
			break;		
		default:
			usage();
		}
	}

	argv += optind;
	argc -= optind;

	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				errflag = 1;
			}
			argc--, argv++;
		}
		if (fastpath)
			tabify(all);
		else
			tabify_mb(all);
	} while (argc > 0);
	exit(errflag);
}

/*
 * NAME: tabify
 *
 * FUNCTION:  read file, add tabs where needed, write file.
 *	      Process text with ANSI-C single byte functions.
 */
static void
tabify(all)
	int all;
{
	register int blanks;
	register size_t column;
	register int c;

	blanks = 0;
	column = 0;

	while ((c = getchar()) != EOF) {
		switch (c) {

		case ' ':
			blanks++;
			if (nstops) {
				if (!tabstops[++column]) {
					if (column > nstops) {
						while (blanks-- > 0)
							putchar(' ');
						while ((c = getchar()) != '\n' && c != EOF)
							putchar(c);
						blanks = column = 0;
						break;
					} else
						continue;
				}
			} else if ((++column & 7) != 0)
				continue;
			if ((blanks = nstops ? blanks : blanks - 1) > 0) {
				c = '\t';
				blanks = 0;
			}
			break;

		case '\t':
			column = (column + 8) & ~7;
			blanks = 0;
			break;

		case '\n':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = 0;
			break;

		case '\b':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = column ? column - 1 : 0;
			break;

		default:
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			if (all)
				column++;
			else {
				do
					putchar(c);
				while ((c = getchar()) != '\n' && c != EOF);
				column = 0;
			}
			break;
		}
		putchar(c);
	}
	return;
}

/*
 * NAME: tabify_mb
 *
 * FUNCTION:  read file, add tabs where needed, write file.
 *	      Process text with X/Open multibyte functions.
 */

static void
tabify_mb(all)
	int all;
{
	register int blanks, len;
	register size_t column;
	register wint_t c;

	blanks = 0;
	column = 0;

	while ((c = getwchar()) != WEOF) {
		switch (c) {

		case ' ':
			blanks++;
			if (nstops) {
				if (!tabstops[++column]) {
					if (column > nstops) {
						while (blanks-- > 0)
							putchar(' ');
						while ((c = getwchar()) != '\n' && c != EOF)
							putwchar(c);
						blanks = column = 0;
						break;
					} else
						continue;
				}
			} else if ((++column & 7) != 0)
				continue;
			if ((blanks = nstops ? blanks : blanks - 1) > 0) {
				c = '\t';
				blanks = 0;
			}
			break;

		case '\t':
			column = (column + 8) & ~7;
			blanks = 0;
			break;

		case '\n':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = 0;
			break;

		case '\b':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = column ? column - 1 : 0;
			break;

		default:
			while (blanks-- > 0)
				putchar(' ');
			if (all) {
				column += (len = wcwidth(c)) == -1 ? 1 : len;
				blanks = 0;
			} else {
				do
					putwchar(c);
				while ((c = getwchar()) != '\n' && c != WEOF);
				blanks = 0;
				column = 0;
			}
			break;
		}
		putwchar(c);
	}
	return;
}

/*
 * NAME: getstops
 *
 * FUNCTION: Reads the tablist delimited by blanks or commas and checks for 
 *	     tabs that are smaller than previous or for too many tab stops.
 *	     Valid tab stops are stored in the tabstops[] array.
 */
static void
getstops(char *cp)
{
	register int i;
	register single = 0;

	for (;;) {
		i = strtoul(cp, &cp, 10); /* Convert from string to integer */
		/* Ensure that the tab stops are in ascending order.         */
		if (i <= 0 || i <= nstops) {
			(void)fprintf(stderr,"unexpand: %s", MSGSTR(TABORDER, "Specify tabs as positive numbers in increasing order.\n"));
			usage();
		} else if (i > LINE_MAX) {
			(void)fprintf(stderr,"unexpand: %s", MSGSTR(TABOVERFLOW, "The tabs cannot be greater than LINE_MAX.\n"));
	  		usage();
		}

		/* Store the tab stop in the table */
		tabstops[i] = TRUE;
		nstops = i;
		single++;
		/* Break out of the for loop if there are no more tab stops */
		if (*cp == 0)
			break;

		/* Valid separators for the tablist can be comma or blank.*/
		if (*cp != ','  &&  *cp != ' '){
			(void)fprintf(stderr,"unexpand: %s", MSGSTR(NEWTABSEP, "Tabs must be separated with a comma or a blank.\n"));
			usage();
		}
		*cp++;
	}
	if (single == 1)
		while((nstops += i) <= (LINE_MAX + 1)) 
			tabstops[nstops] = TRUE;
}

static usage()
{
	fprintf(stderr, MSGSTR(USAGE,"usage: unexpand [-a | -t tablist] [file ...]\n"));
	exit(1);
}
