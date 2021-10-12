static char sccsid[] = "@(#)40	1.4  src/bos/usr/bin/mh/sbr/brkstring.c, cmdmh, bos411, 9428A410j 10/10/90 09:14:43";
/* 
 * COMPONENT_NAME: CMDMH brkstring.c
 * 
 * FUNCTIONS: brkany, brkstring 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* brkstring.c - break string into an array of strings */

#include "mh.h"


static  brkany (register char, register char *);
static char *broken[MAXARGS+1];	/* array of substring start addresses */

char  **brkstring (strg, brksep, brkterm)
register char  *strg;
register char  *brksep,
               *brkterm;
{
    register int    bi;
    register char   c,
                   *sp;

    sp = strg;			/* scan string, replacing separators with
				   zeroes */

    for (bi = 0; bi < MAXARGS; bi++) {
				/* and entering start addrs in "broken" */
	while (brkany (c = *sp, brksep))
	    *sp++ = 0;
	if (!c || brkany (c, brkterm)) {
	    *sp = 0;
	    broken[bi] = 0;
	    return broken;	/* terminator found, finish up */
	}

	broken[bi] = sp;	/* set next start addr */
	while ((c = *++sp) && !brkany (c, brksep) && !brkany (c, brkterm))
		continue;
    }
    broken[MAXARGS] = 0;	/* reached limit of MAXARGS substrings */

    return broken;
}


static  brkany (register char chr,    
	        register char *strg )
				/* returns 1 if chr in strg, 0 otherwise  */
{
    register char  *sp;

    if (strg)
	for (sp = strg; *sp; sp++)
	    if (chr == *sp)
		return 1;
    return 0;
}
