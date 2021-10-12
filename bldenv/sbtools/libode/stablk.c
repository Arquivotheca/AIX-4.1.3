/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: stablk
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: stablk.c,v $
 * Revision 1.8.5.4  1993/04/29  15:45:22  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:28  damon]
 *
 * Revision 1.8.5.3  1993/04/27  20:55:38  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:55:24  damon]
 * 
 * Revision 1.8.5.2  1993/04/08  21:03:51  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:03:16  damon]
 * 
 * Revision 1.8.2.4  1992/12/03  17:22:53  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:23  damon]
 * 
 * Revision 1.8.2.3  1992/12/02  20:27:07  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:50  damon]
 * 
 * Revision 1.8.2.2  1992/06/15  18:10:52  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:45  damon]
 * 
 * Revision 1.8.5.2  1992/06/15  16:34:10  damon
 * 	Taken from 2.1.1
 * 
 * Revision 1.8.2.2  1992/03/24  19:28:03  damon
 * 	Changed strings.h to string.h
 * 	[1992/03/24  19:27:30  damon]
 * 
 * Revision 1.8  1991/12/05  21:13:16  devrcs
 * 	Merged some includes into include of ode/odedefs.h
 * 	[91/09/04  13:11:37  damon]
 * 
 * 	Added changes to support RIOS and aix
 * 	[91/01/22  13:00:36  mckeen]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:47:05  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:22:18  randyb]
 * 
 * Revision 1.6  90/10/07  20:04:51  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:11:18  gm]
 * 
 * Revision 1.5  90/08/09  14:24:04  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:50  gm]
 * 
 * Revision 1.4  90/07/17  12:37:10  devrcs
 * 	More changes for gcc.
 * 	[90/07/08  21:28:28  gm]
 * 
 * Revision 1.3  90/06/29  14:39:35  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:22:04  gm]
 * 
 * Revision 1.2  90/01/02  19:27:20  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:16:05  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Modified for 4.2 BSD.  Now puts output on std. error using fprintf and
 * 	fprstab.
 * 	[85/04/28            sas]
 * 
 * 	Now handles case of multiple exact matches just like case of
 * 	multiple initial-substring matches:  returns -2 if "quiet", else
 * 	asks user which one (as if it matters).
 * 	[81/09/08            sas]
 * 
 * 	Added exactmatch and code to recognize exact match in case of
 * 	ambiguity from initial prefix matching.
 * 	[80/05/19            sas]
 * 
 * 	Changed listing code to use prstab() instead of just printing
 * 	table -- this uses multiple columns when appropriate.  To do this,
 * 	it was necessary to add the "matches" array.  Too bad!
 * 	[80/04/16            sas]
 * 
 * 	Rewritten for VAX from Ken Greer's routine.  The error messages are
 * 	different now.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)26  1.1  src/bldenv/sbtools/libode/stablk.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:42";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/util.h>

#define NOTFOUND -1
#define AMBIGUOUS -2
#define MAXSTRINGS 500

int
stablk ( const char *arg, const char *table[], int quiet )
{
	register int i,ix,count;
	int wantlist;
	const char *matches[MAXSTRINGS];
	int exactmatch;

        ix = -1;
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
