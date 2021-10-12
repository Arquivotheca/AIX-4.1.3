/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		path
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
 * $Log: path.c,v $
 * Revision 1.7.7.1  1993/11/08  17:58:54  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:41  damon]
 *
 * Revision 1.7.5.2  1993/04/27  21:27:03  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:26:41  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:21:28  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:34  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:27  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:25  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:32  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:44  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:24:46  dwm]
 * 
 * Revision 1.5  90/10/07  20:04:01  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:05  gm]
 * 
 * Revision 1.4  90/08/09  14:23:33  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:45  gm]
 * 
 * Revision 1.3  90/06/29  14:38:59  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:29  gm]
 * 
 * Revision 1.2  90/01/02  19:27:07  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:22  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Copied verbatim from PDP-11.  Still as messy as ever.
 * 	Some people have asked for a modification (I think that's a better
 * 	idea than a new routine) which will change the directory name
 * 	into an absolute pathname if it isn't one already.  The change
 * 	involves doing a getwd() and prepending that if appropriate, with
 * 	a "/" in between that and the directory part of the path.
 * 	If you want to be cute, you can also resolve ".."s at that time.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)00  1.1  src/bldenv/sbtools/libode/path.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:47";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: path.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/11/08 17:58:54 $";
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

#include <ode/util.h>

void
path ( const char *original, char *direc, char *file )
{
	register char *y;
	/* x is direc */
	register char *p;

	/* copy and note the end */
	p = (char *)original;
	y = direc;
	while ((*y++ = *p++)) ;		/* copy string */
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
