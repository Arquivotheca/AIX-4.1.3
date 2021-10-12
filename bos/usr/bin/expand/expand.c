static char sccsid[] = "@(#)14	1.12  src/bos/usr/bin/expand/expand.c, cmdfiles, bos412, 9446C 11/14/94 16:48:22";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: expand
 *
 * ORIGINS: 3, 26, 27
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "expand_msg.h" 

static nl_catd catd;
#define MSGSTR(num,str) catgets(catd, MS_EXPAND, num, str) 

#define	MAX_TABS 100

/* There are 2 usage statements now to support the old and the new syntax as */
/* defined in XPG4.  The old syntax will eventually be phased out.	     */
#define USAGE_STATEMENT	 \
	(void)fprintf(stderr,MSGSTR(NEWEXPUSAGE,\
		"usage: expand [-t tablist] [file...]\n")); \
	(void)fprintf(stderr, MSGSTR(EXPUSAGE, \
		"usage: expand [-tabstop][-tab1,tab2,...,tabn] [file ...]\n"))

#define COMMA_SEP 0	     /* Only a comma separated list allowed          */
#define BLANK_COMMA_SEP 1    /* Blank or comma separated list allowed        */

static void getstops(register char *cp, int format);
static void expand(void);
static void expand_mb(void);

static int	nstops = 0;
static int	tabstops[MAX_TABS];


/*
 * NAME: expand [-t tablist] [file...]
 *		 or
 *	 expand [ -tabstop ] [-tab1,tab2,..,tabn] [file...]
 *
 * FUNCTION: expand tabs to equivalent spaces
 * NOTE:  Default tab is 8.
 *        If -tabstop is given then the width of each tab is tabstop.
 *        If -t tablist or -tab1,tab2,..,tabn is given then tabs are 
 *	  set at those specific columns.
 */
main(int argc, char *argv[])
{
	register int fastpath;
	int option, dflag=0;

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_EXPAND, NL_CAT_LOCALE);
	fastpath = MB_CUR_MAX == 1;

	/* This code supports parsing of the new syntax in which tabstops    */
	/* can be specified using the -t tab1,tab2,..tabn parameters.        */
	opterr=0;
	if (argc == 1)  		/* No files or args given, use stdin */
		argc--, argv++;
	else { 
	while (dflag == 0 && ((option =  getopt(argc, argv, "t:")) != EOF)){
		switch(option) {

		case 't':
			/* Parse the tablist and store tabstop info in the   */
			/* tabstops[] array table.	                     */
			getstops(--optarg, BLANK_COMMA_SEP);
			break;
		default:
			if (argv[1][1] == '?') {
				USAGE_STATEMENT;
				exit(0);
			} else {
				/* This section of code supports the old     */
				/* syntax in which tabstops are specified as */
				/* -tabstop of as -tab1,tab2,..,tabn.	     */
               			argc--, argv++;
				while (argc > 0 && argv[0][0] == '-') {
               	       			getstops(argv[0], COMMA_SEP);
               				argc--, argv++;
               			}
				dflag=1;
			}
			break;
		}   /* end switch */
		}  	/* end while */

		if (!dflag) {
			argc -= optind;
		 	argv += optind;
		}

	}  /* end else */

	/* While there are more files to be expanded, open the file and */
	/* and expand it according to the tabstops[] array table. If no */
	/* tabstops are specified, the default spacing is 8 chars.      */
        do {
                if (argc > 0) {
                        if (freopen(argv[0], "r", stdin) == NULL) {
                                perror(argv[0]);
                                exit(1);
                        }
                        argc--, argv++;
                }
                if (fastpath)
                        expand();
                else
                        expand_mb();
        } while (argc > 0);
	exit(0);
}

/*
 * NAME: expand
 *
 * FUNCTION: Replace tabs with blanks.
 *	     Use ANSI-C single byte functions.
 */

static void
expand(void)
{
	register int column;
	register int c;
	register int n;

	column = 0;
	while ((c = getchar()) != EOF) {
		switch (c) {

		case '\t':
			if (nstops == 0) {
				do {
					putchar(' ');
					column++;
				} while (column & 07);
			} else if (nstops == 1) {
				do {
					putchar(' ');
					column++;
				} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
			} else {
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putchar(' ');
					column++;
					break;
				}
				while (column < tabstops[n]) {
					putchar(' ');
					column++;
				}
			}
			break;

		case '\b':
			if (column)
				column--;
			putchar('\b');
			break;

		default:
			putchar(c);
			column++;
			break;

		case '\n':
			putchar(c);
			column = 0;
			break;
		}
	}
	return;
}

/*
 * NAME: expand_mb
 *
 * FUNCTION: Replace tabs with blanks.
 *	     Use X/Open multibyte functions.
 */

static void
expand_mb(void)
{
	register int column;
	register wint_t c;
	register int n;

	column = 0;
	while ((c = getwchar()) != WEOF) {
		switch (c) {

		case '\t':
			if (nstops == 0) {
				do {
					putchar(' ');
					column++;
				} while (column & 07);
			} else if (nstops == 1) {
				do {
					putchar(' ');
					column++;
				} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
			} else {
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putchar(' ');
					column++;
					break;
				}
				while (column < tabstops[n]) {
					putchar(' ');
					column++;
				}
			}
			break;

		case '\b':
			if (column)
				column--;
			putchar('\b');
			break;

		default:
			(void)putwchar(c);
			column += (n = wcwidth((wchar_t)c)) == -1 ? 1 : n;
			break;

		case '\n':
			putchar(c);
			column = 0;
			break;
		}
	}
	return;
}

/*
 * NAME: getstops
 *
 * FUNCTION: checks for tabs that are smaller than previous or for
 *	     too many tab stops.
 */
static void
getstops(register char *cp, int format)
{
	register int i;

	/* In the original syntax, cp was sent in the format -tab1,tab2,.. */
	/* In the new syntax, cp is sent as a null character followed by   */
	/* tab1,tab2,.. or tab1 tab2 ...  Therefore, cp is incremented     */
	/* past the first char to get to the list of numbers.		   */ 
	cp++;

	/* In both the new and old syntax, cp is terminated by a '\0' char */
	for (;;) {

		/* Convert from string to integer */
		i = 0;
		while (*cp >= '0' && *cp <= '9')
			i = i * 10 + *cp++ - '0';

		/* Ensure that the tab stops are in ascending order.         */
		/* Issue both the old and new formats of the usage statement */
		if (i <= 0 || (nstops > 0 && i <= tabstops[nstops-1])) {
			(void)fprintf(stderr,"expand: %s", MSGSTR(TABORDER, "Specify tabs in increasing order.\n"));
			USAGE_STATEMENT;
	  		exit(1);

		/* You can currently only specify up to 100 tab stops       */
		} else if (nstops >= MAX_TABS) {
			(void)fprintf(stderr,"expand: %s", MSGSTR(TABOVERFLOW, "Too many tabs.\n"));
			USAGE_STATEMENT;
	  		exit(1);
		}

		/* Store the tab stop in the table 			    */
		tabstops[nstops++] = i;

		/* Break out of the for loop if there are no more tab stops */
		if (*cp == 0)
			break;

		/* Valid separators for the new syntax can be comma or blank.*/
		if (format == BLANK_COMMA_SEP  &&  *cp != ','  &&  *cp != ' '){
			(void)fprintf(stderr, "expand: %s", MSGSTR(NEWTABSEP, "Tabs must be separated with a comma or a blank.\n"));
			USAGE_STATEMENT;
	  		exit(1);
		/* Valid separators for the old syntax can only be the comma.*/
		} else if (format == COMMA_SEP  &&  *cp != ',') {
			(void)fprintf(stderr,MSGSTR(TABSEP, "expand: Tabs must be separated with a \",\".\n"));
			USAGE_STATEMENT;
	  		exit(1);
		}
		*cp++;
	}
}
