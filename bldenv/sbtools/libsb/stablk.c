static char sccsid[] = "@(#)61  1.1  src/bldenv/sbtools/libsb/stablk.c, bldprocess, bos412, GOLDA411a 4/29/93 12:24:29";
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */
/*  stablk  --  string table lookup
 *
 *  Usage:  i = stablk (arg,table,quiet);
 *
 *	int i;
 *	char *arg,**table;
 *	int quiet;
 *
 *  Stablk looks for a string in "table" which matches
 *  "arg".  Table is declared like this:
 *    char *table[] = {"string1","string2",...,0};
 *  Each string in the table is checked via stablk() to determine
 *  if its initial characters match arg.  If exactly one such
 *  string matches arg, then the index of that string is returned.
 *  If none match arg, or if several match, then -1 (respectively -2)
 *  is returned.  Also, for either of these errors, if quiet is
 *  FALSE, the user will be asked if he wants a list of the possible
 *  strings.  In the case of multiple matches, the matching strings
 *  will be marked specially.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX.
 */

#include <string.h>
#include <ode/odedefs.h>

#define NOTFOUND -1
#define AMBIGUOUS -2
#define MAXSTRINGS 500

int stlmatch();
#ifndef STRRETINT
int strcmp();
#endif

int stablk (arg,table,quiet)
char *arg, **table;
int quiet;
{
	register int i,ix,count;
	int wantlist;
	char *matches[MAXSTRINGS];
	int exactmatch;

	count = 0;
	exactmatch = 0;
	for (i=0; table[i] != 0 && exactmatch == 0; i++) {
		if (stlmatch (table[i],arg)) {
			ix = i;		/* index of last match */
			matches[count++] = table[i];
			if (strcmp(table[i],arg) == 0)  exactmatch = 1;
		}
	}
	matches[count] = 0;

	if (exactmatch) {	/* i-th entry is exact match */
		--i;		/* (actually, i-1th entry) */
		matches[0] = table[i];
		count = 1;
		for (i=i+1; table[i] != 0; i++) {
			if (strcmp(table[i],arg) == 0)  {
				matches[count++] = table[i];
				ix = i;
			}
		}
		matches[count] = 0;
	}

	if (count == 1)  return (ix);

	if (!quiet) {
		if (strcmp(arg,"?") == 0) {
			wantlist = 1;
		}
		else {
			ui_print ( VALWAYS, "%s is %s.  ",
				   arg,(count ? "ambiguous" : "unknown"));
			wantlist = getbool ("Do you want a list?",1);
		}
		if (wantlist) {
			ui_print ( VALWAYS, "Must match one of these:\n");
			if (count)  fprstab (stderr,matches);
			else	    fprstab (stderr,table);
		}
	}
	return (count ? AMBIGUOUS : NOTFOUND);
}
