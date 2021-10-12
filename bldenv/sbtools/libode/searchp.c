/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		searchp
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
 * $Log: searchp.c,v $
 * Revision 1.7.7.1  1993/11/03  20:41:11  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:37  damon]
 *
 * Revision 1.7.5.2  1993/04/27  22:41:37  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:41:14  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:22:48  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:18  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:56  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:45  damon]
 * 
 * Revision 1.7  1991/12/05  21:13:06  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:54  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:25:23  dwm]
 * 
 * Revision 1.5  90/10/07  20:04:38  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:59  gm]
 * 
 * Revision 1.4  90/08/09  14:23:56  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:33  gm]
 * 
 * Revision 1.3  90/06/29  14:39:24  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:51  gm]
 * 
 * Revision 1.2  90/01/02  19:27:14  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:44  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	4.1BSD system ignores trailing slashes. 4.2BSD does not.
 * 	Therefore don't add a seperating slash if there is a null
 * 	filename.
 * 	[86/04/01            ern]
 * 
 * 	Fixed two bugs: (1) calling function as "func" instead of
 * 	"(*func)", (2) omitting trailing null name implied by trailing
 * 	colon in path.  Latter bug fixed by introducing "lastchar" and
 * 	changing final loop test to look for "*lastchar" instead of
 * 	"*nextpath".
 * 	[82/10/23            sas]
 * 
 * 	Created for VAX.  If you're thinking of using this, you probably
 * 	should look at openp() and fopenp() (or the "want..." routines)
 * 	instead.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)22  1.1  src/bldenv/sbtools/libode/searchp.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:34";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: searchp.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/11/03 20:41:11 $";
#endif
/*  searchp  --  search through pathlist for file
 *
 *  Usage:  p = searchp (path,file,fullname,func);
 *	char *p, *path, *file, *fullname;
 *	int (*func)();
 *
 *  Searchp will parse "path", a list of pathnames separated
 *  by colons, prepending each pathname to "file".  The resulting
 *  filename will be passed to "func", a function provided by the
 *  user.  This function must return zero if the search is
 *  successful (i.e. ended), and non-zero if the search must
 *  continue.  If the function returns zero (success), then
 *  searching stops, the full filename is placed into "fullname",
 *  and searchp returns 0.  If the pathnames are all unsuccessfully
 *  examined, then searchp returns -1.
 *  If "file" begins with a slash, it is assumed to be an
 *  absolute pathname and the "path" list is not used.  Note
 *  that this rule is used by Bell's cc also; whereas Bell's
 *  sh uses the rule that any filename which CONTAINS a slash
 *  is assumed to be absolute.  The execlp and execvp procedures
 *  also use this latter rule.  In my opinion, this is bogosity.
 */

#include <ode/util.h>

int
searchp ( const char *spath, const char *file, const char *fullname,
          int (*func)( const char * ) )
{
	const char *nextpath, *nextchar,*lastchar;
        char *fname;
	int failure;

	nextpath = ((*file == '/') ? "" : spath);
	do {
		fname = (char *)fullname;
		nextchar = nextpath;
		while (*nextchar && (*nextchar != ':'))
			*fname++ = *nextchar++;
		if (nextchar != nextpath && *file) *fname++ = '/';
		lastchar = nextchar;
		nextpath = ((*nextchar) ? nextchar + 1 : nextchar);
		nextchar = file;	/* append file */
		while (*nextchar)  *fname++ = *nextchar++;
		*fname = '\0';
		failure = (*func) (fullname);
	} 
	while (failure && (*lastchar));
	return (failure ? -1 : 0);
}
