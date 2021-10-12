static char sccsid[] = "@(#)48  1.1  src/bldenv/sbtools/libsb/path.c, bldprocess, bos412, GOLDA411a 4/29/93 12:22:30";
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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: path.c,v $ $Revision: 1.7 $ (OSF) $Date: 91/12/05 21:05:32 $";
#endif
/*  path  --  break filename into directory and file
 *
 *  path (filename,direc,file);
 *  char *filename,*direc,*file;
 *  filename is input; direc and file are output (user-supplied).
 *  file will not have any trailing /; direc might.
 *
 *  Note these rules:
 *  1.  trailing / are ignored (except as first character)
 *  2.  x/y is x;y where y contains no / (x may contain /)
 *  3.  /y  is /;y where y contains no /
 *  4.  y   is .;y where y contains no /
 *  5.      is .;. (null filename)
 *  6.  /   is /;. (the root directory)
 *
 * Algorithm is this:
 *  1.  delete trailing / except in first position
 *  2.  if any /, find last one; change to null; y++
 *      else y = x;		(x is direc; y is file)
 *  3.  if y is null, y = .
 *  4.  if x equals y, x = .
 *      else if x is null, x = /
 */

path (original,direc,file)
char *original,*direc,*file;
{
	register char *y;
	/* x is direc */
	register char *p;

	/* copy and note the end */
	p = original;
	y = direc;
	while (*y++ = *p++) ;		/* copy string */
	/* y now points to first char after null */
	--y;	/* y now points to null */
	--y;	/* y now points to last char of string before null */

	/* chop off trailing / except as first character */
	while (y>direc && *y == '/') --y;	/* backpedal past / */
	/* y now points to char before first trailing / or null */
	*(++y) = 0;				/* chop off end of string */
	/* y now points to null */

	/* find last /, if any.  If found, change to null and bump y */
	while (y>direc && *y != '/') --y;
	/* y now points to / or direc.  Note *direc may be / */
	if (*y == '/') {
		*y++ = 0;
	}

	/* find file name part */
	if (*y)  strcpy (file,y);
	else     strcpy (file,".");

	/* find directory part */
	if (direc == y)        strcpy (direc,".");
	else if (*direc == 0)  strcpy (direc,"/");
	/* else direc already has proper value */
}
