static char sccsid[] = "@(#)38	1.11  src/bos/usr/bin/rev/rev.c, cmdfiles, bos411, 9428A410j 11/16/93 09:50:41";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: rev
 *
 * ORIGINS: 18, 26, 27
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
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#include <stdio.h>
#include <locale.h>

/* reverse lines of a file */

#define N 256
FILE *input;
#include "rev_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_REV, n, s) 

#include <sys/types.h>
nl_catd catd;
wchar_t line[N];

/*
 * NAME: rev [file] ...
 *                                                                    
 * FUNCTION:  Reverse lines of a file
 */  

main(argc,argv)
int argc;
char *argv[];
{
	register int i, eof = 0;
        wint_t wc;
	
	(void) setlocale(LC_ALL,"");
        catd = catopen(MF_REV, NL_CAT_LOCALE);
	input = stdin;
	do {
		eof = 0;
		if(argc>1) {
			if((input=fopen(argv[1],"r"))==NULL) {
				fprintf(stderr,MSGSTR(OPENFAIL, "rev: cannot open %s\n"),
					argv[1]);
				exit(1);
			}
		}
		for(;;){
			for(i=0;i<N;i++) {
				line[i] = wc = getwc(input);
				switch(wc) {
				case WEOF:
					eof++;
					break;
				default:
					continue;
				case '\n':
					break;
				}
				break;
			}
			if (eof > 0 ) break;
			while(--i>=0)
				putwc(line[i],stdout);
			putwc('\n',stdout);
		}
		fclose(input);
		argc--;
		argv++;
	} while(argc>1);
	exit(0);
}
