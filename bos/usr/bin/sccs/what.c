static char sccsid[] = "@(#)03  1.9  src/bos/usr/bin/sccs/what.c, cmdsccs, bos41B, 9504A 12/9/94 10:00:17";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: dowhat, main
 *
 * ORIGINS: 3, 10, 27
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
 */


#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include "defines.h"

static int found;
static int silent;

#define pat0    '@'
static char pat1[]  =  "(#)";

main(argc,argv)
int argc;
register char **argv;
{
	register done = FALSE;
	register int c;

	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	while ((c = getopt(argc, argv, "s")) != EOF)
	{
		switch (c)
		{
			case 's':
				silent = TRUE;
				break;
			default:
				fprintf(stderr,MSGCM(USAGEWHAT, 
				   "Usage: what [ -s ] [ File... ]\n"));
				exit(1);
		}
	} 

	for ( ; optind < argc; optind++)
	{
		if (!(freopen(argv[optind],"r",stdin)))
			perror(argv[optind]);
		else {
			printf("%s:\n",argv[optind]);
			dowhat();
		}
		done = TRUE;
	}
	if (!done)
		dowhat();
	exit(!found);				/* shell return code */
}


static dowhat()
{
	register int c;
	register char *p;

	while ((c = getchar()) != EOF)
		while (c == pat0)
			for (p = pat1;;) {
				if ((c = getchar()) == EOF) return;
				if (c != *p) break;
				if (!*++p) {
					putchar('\t');
					while ((c = getchar()) != EOF && c &&
					    !any(c,"\"\\>\n"))
						putchar(c);
					putchar('\n');
					found = TRUE;
					if (silent) return;
					break;
				}
			}
}
