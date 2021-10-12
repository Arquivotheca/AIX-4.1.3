/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: quit
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
 * $Log: quit.c,v $
 * Revision 1.7.7.1  1993/11/09  16:53:49  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:47  damon]
 *
 * Revision 1.7.5.2  1993/04/27  21:45:35  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:45:22  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:22:13  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:55  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:35  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:29  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:38  devrcs
 * 	Correct copyright; clean up lint input.
 * 	[91/01/08  12:17:31  randyb]
 * 
 * Revision 1.5  90/10/07  20:04:15  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:24  gm]
 * 
 * Revision 1.4  90/08/09  14:23:41  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:02  gm]
 * 
 * Revision 1.3  90/06/29  14:39:07  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:38  gm]
 * 
 * Revision 1.2  90/01/02  19:27:11  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:30  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * Revision 1.3  89/08/30  18:09:36  bww
 * 	Updated handling of variable argument lists.
 * 	[89/08/30  18:08:43  bww]
 * 
 * Revision 1.2  88/12/13  13:52:41  gm0w
 * 	Rewritten to use varargs.
 * 	[88/12/13            gm0w]
 * 
 * $EndLog$
 */
/*
 *  quit  --  print message and exit
 *
 *  Usage:  quit (status,format [,arg]...);
 *	int status;
 *	(... format and arg[s] make up a printf-arglist)
 *
 *  Quit is a way to easily print an arbitrary message and exit.
 *  It is most useful for error exits from a program:
 *	if (open (...) < 0) then quit (1,"Can't open...",file);
 */

#ifndef lint
static char sccsid[] = "@(#)14  1.1  src/bldenv/sbtools/libode/quit.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:15";
#endif /* not lint */

#include <stdio.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ode/util.h>

void
#if __STDC__
quit (int status, const char *fmt, ...)
#else
quit (va_alist)
va_dcl
#endif
{
#if !__STDC__
	int status;
	char *fmt;
#endif
	va_list args;

	fflush(stdout);
#if __STDC__
	va_start(args, fmt);
#else
	va_start(args);
	status = va_arg(args, int);
	fmt = va_arg(args, char *);
#endif
	(void) vfprintf(stderr, fmt, args);
	va_end(args);
	exit(status);
}
