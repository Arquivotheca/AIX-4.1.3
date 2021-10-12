/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		stlmatch
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
 * $Log: stlmatch.c,v $
 * Revision 1.8.5.3  1993/04/29  15:45:20  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:27  damon]
 *
 * Revision 1.8.5.2  1993/04/27  22:33:09  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:32:59  damon]
 * 
 * Revision 1.8.2.3  1992/12/03  17:22:56  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:25  damon]
 * 
 * Revision 1.8.2.2  1992/12/02  20:27:11  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:51  damon]
 * 
 * Revision 1.8  1991/12/05  21:13:19  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:12:10  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:25:46  dwm]
 * 
 * Revision 1.6  90/10/07  20:04:56  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:11:24  gm]
 * 
 * Revision 1.5  90/08/09  14:24:07  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:55  gm]
 * 
 * Revision 1.4  90/07/17  12:37:13  devrcs
 * 	More changes for gcc.
 * 	[90/07/08  21:28:34  gm]
 * 
 * Revision 1.3  90/06/29  14:39:38  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:22:09  gm]
 * 
 * Revision 1.2  90/01/02  19:27:22  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:16:12  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Rewritten for VAX from Ken Greer's routine.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)27  1.1  src/bldenv/sbtools/libode/stlmatch.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:44";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: stlmatch.c,v $ $Revision: 1.8.5.3 $ (OSF) $Date: 1993/04/29 15:45:20 $";
#endif
/*  stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

#include <ode/util.h>

int
stlmatch ( const char *small, const char *big )
{
	register char *s, *b;
	s = (char *)small;
	b = (char *)big;
	do {
		if (*s == '\0')  return (1);
	} 
	while (*s++ == *b++);
	return (0);
}
