/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: fprstab
 *		prstab
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
 * $Log: prstab.c,v $
 * Revision 1.7.5.5  1993/04/29  17:35:33  damon
 * 	CR 463. Fixed proto of prstab
 * 	[1993/04/29  17:35:23  damon]
 *
 * Revision 1.7.5.4  1993/04/29  17:31:38  marty
 * 	Fix prototype of fprstab(). CR 463
 * 	[1993/04/29  17:31:21  marty]
 * 
 * Revision 1.7.5.3  1993/04/29  15:45:18  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:25  damon]
 * 
 * Revision 1.7.5.2  1993/04/27  21:08:39  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:08:30  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:22:11  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:54  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:31  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:27  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:36  devrcs
 * 	Correct copyright; clean up lint input.
 * 	[91/01/08  12:17:14  randyb]
 * 
 * Revision 1.5  90/10/07  20:04:10  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:17  gm]
 * 
 * Revision 1.4  90/08/09  14:23:38  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:56  gm]
 * 
 * Revision 1.3  90/06/29  14:39:04  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:33  gm]
 * 
 * Revision 1.2  90/01/02  19:27:09  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:27  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Modified for 4.2 BSD.  Also added fprstab() routine.
 * 	[85/04/28            sas]
 * 
 * 	Fixed two bugs:  printing extra spaces at end of last column, and
 * 	dividing by zero if ncol = 0 (i.e. very long string).
 * 	[83/03/15            sas]
 * 
 * 	Created.
 * 	[80/04/16            sas]
 * 
 * $EndLog$
 */
/*  prstab, fprstab  --  print list of strings
 *
 *  Usage:  prstab (table);
 *	    fprstab (file,table);
 *	char **table;
 *	FILE *file;
 *
 *  table is an array of pointers to strings, ending with a 0
 *  value.  This is the same format as "stablk" tables.
 *
 *  Prstab will attempt to print the strings in a concise format,
 *  using multiple columns if its heuristics indicate that this is
 *  desirable.
 *  Fprstab is the same, but you can specify the file instead of using
 *  stdout.
 *
 *  The heuristics are these:  assume that each column must be at
 *  least as wide as the longest string plus three blanks.  Figure
 *  out how many columns can fit on a line, and suppose that we use
 *  that many columns.  This represents the "widest" useable format.
 *  Now, see if this is too wide.  This means that there are just a
 *  few strings, and we would like them to be printed in fewer columns,
 *  with each column being a little bit longer.  The heuristic rule is
 *  that we will always use at least some minimum number of rows (8)
 *  if there are at least that many strings.
 */

#ifndef lint
static char sccsid[] = "@(#)13  1.1  src/bldenv/sbtools/libode/prstab.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:13";
#endif /* not lint */

#include <stdio.h>
#include <ode/util.h>

#define SPACE 5			/* min. space between columns */
#define MAXCOLS 71		/* max. cols on line */
#define MINROWS 8		/* min. rows to be printed */


void
prstab ( const char *list[] ) 
{
	fprstab (stdout,list);
}

void
fprstab ( FILE *file, const char *list[])
{
	register int nelem;	/* # elements in list */
	register int maxwidth;	/* widest element */
	register int i,l;	/* temps */
	register int row,col;	/* current position */
	register int nrow,ncol;	/* desired format */
	char format[20];	/* format for printing strings */

	maxwidth = 0;
	for (i=0; list[i]; i++) {
		l = strlen (list[i]);
		if (l > maxwidth)  maxwidth = l;
	}

	nelem = i;
	if (nelem <= 0)  return;

	ncol = MAXCOLS / (maxwidth + SPACE);
	if (ncol < 1)  ncol = 1;	/* for very long strings */
	if (ncol > (nelem + MINROWS - 1) / MINROWS)
		ncol = (nelem + MINROWS - 1) / MINROWS;
	nrow = (nelem + ncol - 1) / ncol;

	sprintf (format,"%%-%ds",maxwidth+SPACE);

	for (row=0; row<nrow; row++) {
		fprintf (file,"\t");
		for (col=0; col<ncol; col++) {
			i = row + (col * nrow);
			if (i < nelem) {
				if (col < ncol - 1) {
					fprintf (file,format,list[i]);
				}
				else {
					fprintf (file,"%s",list[i]);
				}
			}
		}
		fprintf (file,"\n");
	}
}
