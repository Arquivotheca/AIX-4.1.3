static char sccsid[] = "@(#)49  1.1  src/bldenv/sbtools/libsb/prstab.c, bldprocess, bos412, GOLDA411a 4/29/93 12:22:38";
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

#include <stdio.h>

#define SPACE 5			/* min. space between columns */
#define MAXCOLS 71		/* max. cols on line */
#define MINROWS 8		/* min. rows to be printed */

prstab (list)
char **list;
{
	fprstab (stdout,list);
}

fprstab (file,list)
FILE *file;
char **list;
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
