/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		nxtarg
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
 * $Log: nxtarg.c,v $
 * Revision 1.7.7.1  1993/11/10  19:18:05  damon
 * 	CR 463. Removed unsigned from call to skipto
 * 	[1993/11/10  19:17:55  damon]
 *
 * Revision 1.7.5.3  1993/04/28  14:35:56  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:43  damon]
 * 
 * Revision 1.7.5.2  1993/04/27  21:45:31  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:45:21  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:21:18  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:28  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:23  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:23  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:25  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:38  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:24:35  dwm]
 * 
 * Revision 1.5  90/10/07  20:03:46  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:43  gm]
 * 
 * Revision 1.4  90/08/09  14:23:24  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:27  gm]
 * 
 * Revision 1.3  90/06/29  14:38:55  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:25  gm]
 * 
 * Revision 1.2  90/01/02  19:27:05  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:05  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Bug fix: added check for "back >= front" in loop to chop trailing
 * 	white space.
 * 	[83/07/01            sas]
 * 
 * 	Rewritten for VAX.  By popular demand, a table of break characters
 * 	has been added (implemented as a string passed into nxtarg).
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: nxtarg.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/11/10 19:18:05 $";
#endif
/*
 *  nxtarg -- strip off arguments from a string
 *
 *  Usage:  p = nxtarg (&q,brk);
 *	char *p,*q,*brk;
 *	extern char _argbreak;
 *
 *	q is pointer to next argument in string
 *	after call, p points to string containing argument,
 *	q points to remainder of string
 *
 *  Leading blanks and tabs are skipped; the argument ends at the
 *  first occurence of one of the characters in the string "brk".
 *  When such a character is found, it is put into the external
 *  variable "_argbreak", and replaced by a null character; if the
 *  arg string ends before that, then the null character is
 *  placed into _argbreak;
 *  If "brk" is 0, then " " is substituted.
 *
 *  Originally	from klg (Ken Greer); IUS/SUS UNIX.
 */

#ifndef lint
static char sccsid[] = "@(#)94  1.1  src/bldenv/sbtools/libode/nxtarg.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:31";
#endif /* not lint */

#include <ode/util.h>

char _argbreak;

char *
nxtarg ( char **q, const char *brk )
{
	register char *front,*back;
        
	front = *q;			/* start of string */
	/* leading blanks and tabs */
	while (*front && (*front == ' ' || *front == '\t')) front++;
	/* find break character at end */
	if (brk == 0) brk = " ";
	back = skipto (front,brk);
	_argbreak = *back;
	*q = (*back ? back+1 : back);	/* next arg start loc */
	/* elim trailing blanks and tabs */
	back -= 1;
	while ((back >= front) && (*back == ' ' || *back == '\t')) back--;
	back++;
	if (*back)  *back = '\0';
	return (front);
}
