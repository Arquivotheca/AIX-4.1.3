static char sccsid[] = "@(#)80  1.14  src/bos/usr/bin/colrm/colrm.c, cmdfiles, bos411, 9428A410j 11/16/93 08:53:24";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: colrm
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
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
COLRM removes unwanted columns from a file
*/

#include <stdio.h>
#include <sys/limits.h>
#include <locale.h>
#include <stdlib.h>
#include "colrm_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_COLRM, n, s) 

#include <sys/types.h>

/*
 * NAME: colrm [startcol [endcol]]
 *
 * FUNCTION: COLRM removes unwanted columns from a file
 * NOTE:  If called with only one number then colrm removes columns startcol 
 *        to the end of the line.
 *        If called with two numbers then colrm removes columns startcol to 
 *        endcol.
 */
main(argc,argv)
char **argv;
{
	register int ct = 0, first = 0, last = 0,i = 0,tmpct;
	wint_t wc;
        wchar_t lc, buf[MAX_INPUT];
        nl_catd catd;

	(void) setlocale(LC_ALL,"");
        catd = catopen(MF_COLRM, NL_CAT_LOCALE);
    	if (argc < 2 || argc > 3) {
        	fprintf(stderr,MSGSTR(USAGE,"Usage: colrm first [last]\n"));
		exit(1);
	}
	if (argc > 1)
		first = atoi(*++argv);
	if ( first < 1) {
		fprintf(stderr,MSGSTR(ERROR1,"colrm: Start column is less than 1.\n"));	
		exit(1);
	}
	if (argc > 2) {
		last = atoi(*++argv);
		if (first > last ) {
		  fprintf(stderr,MSGSTR(ERROR2,
			"colrm: Start column is greater then end column.\n")); 
		  exit(1);
		}
	}

	while ((wc = getwc(stdin)) != WEOF)
        {
		if (wc == '\n') {            /* if newline restart counter */
			buf[i] = '\0';
			putws(buf);
			ct = i = 0;
			continue;
		}
		if (wc == '\b' && ct > 0) {
			ct--;
			i--;
			continue;
		}	
		ct++;
		if (last != 0 && ct > last) {
			buf[i++] = wc;
			continue;
		}
		if (wc == '\t') {
			tmpct = ct;
			ct = ct + 8 & ~ 7;
			if ((last > 0 && tmpct <= first && ct >= first) || 
                                         (last > 0 && ct > first && ct >= last)) {
				while (tmpct <= ct) {
					tmpct++;
					if ((tmpct-1 >= first) && (tmpct-1 <= last))
						continue;
					buf[i++] = ' ';
				}
				continue;
			}	
		}
		if (ct >= first && (last == 0 || ct <= last))
                  continue;
		buf[i++] = wc;
	} 
	exit(0);
} 
