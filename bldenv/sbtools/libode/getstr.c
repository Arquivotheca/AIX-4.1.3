/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		getstr
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
 * $Log: getstr.c,v $
 * Revision 1.7.7.1  1993/11/05  22:53:43  damon
 * 	CR 775. Added include of string.h
 * 	[1993/11/05  22:53:32  damon]
 *
 * Revision 1.7.5.2  1993/04/27  20:55:45  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:55:27  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:20:58  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:16  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:17  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:20  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:06  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:33  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:24:16  dwm]
 * 
 * Revision 1.5  90/10/07  20:03:32  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:23  gm]
 * 
 * Revision 1.4  90/08/09  14:23:15  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:09  gm]
 * 
 * Revision 1.3  90/06/29  14:38:46  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:16  gm]
 * 
 * Revision 1.2  90/01/02  19:27:00  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:14:41  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Modified for 4.2 BSD.  Now uses stderr for output.
 * 	[85/04/28            sas]
 * 
 * 	Added code to copy default to answer (in addition to Fil's code to
 * 	return NULL) on error or EOF in the standard input.
 * 	[82/10/23            sas]
 * 
 * 	Getstr() now percuolates any errors from gets() up to the calling
 * 	routine.
 * 	[80/10/21            faa]
 * 
 * 	Increased buffer size to 4000 characters.  Why not?
 * 	[80/05/19            sas]
 * 
 * 	Rewritten for VAX.  Mike thinks a 0 pointer for the default should
 * 	print no default (i.e. not even braces); I'm not sure I like the idea
 * 	of a routine that doesn't explicitly tell you what happens if you
 * 	just hit Carriage Return.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)85  1.1  src/bldenv/sbtools/libode/getstr.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:10";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getstr.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/11/05 22:53:43 $";
#endif
/*  getstr --  prompt user for a string
 *
 *  Usage:  p = getstr (prompt,defalt,answer);
 *	char *p,*prompt,*defalt,*answer;
 *
 *  Getstr prints this message:  prompt  [defalt]
 *  and accepts a line of input from the user.  This line is
 *  entered into "answer", which must be a big char array;
 *  if the user types just carriage return, then the string
 *  "defalt" is copied into answer.
 *  Value returned by getstr is just the same as answer,
 *  i.e. pointer to result string.
 *  The default value is used on error or EOF in the standard input.
 */

#include <stdio.h>
#include <string.h>
#include <ode/util.h>

char *
getstr ( const char *prompt, const char *defalt, char *answer )
{
	char defbuf[4000];
	register char *retval;

	fflush (stdout);
	fprintf (stderr,"%s  [%s]  ",prompt,defalt);
	fflush (stderr);
	strcpy (defbuf,defalt);
	retval = (char *) gets (answer);
	if (retval == NULL || *answer == '\0')  strcpy (answer,defbuf);
	if (retval == NULL)
	    return (retval);
	else
	    return (answer);
}
