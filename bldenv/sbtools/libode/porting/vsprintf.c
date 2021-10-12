/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		vsnprintf
 *		vsprintf
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
 */
/*
 * HISTORY
 * $Log: vsprintf.c,v $
 * Revision 2.8.2.3  1992/12/03  17:22:05  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:50  damon]
 *
 * Revision 2.8.2.2  1992/09/01  18:29:22  damon
 * 	CR 267. Moved vfprintf and vsprintf here.
 * 	[1992/08/07  15:49:56  damon]
 * 
 * Revision 2.8  1991/12/05  21:13:34  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:12:16  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:25:59  dwm]
 * 
 * Revision 2.6  90/10/07  20:05:09  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:11:44  gm]
 * 
 * Revision 2.5  90/08/09  14:24:16  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:49:13  gm]
 * 
 * Revision 2.4  90/06/29  14:39:45  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:22:18  gm]
 * 
 * Revision 2.3  90/01/02  18:54:48  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 2.2  89/12/26  11:28:09  gm
 * 	Added _doprnt #define when _DOPRNT_IS_VISIBLE is not set.
 * 	[89/12/22            gm]
 * 
 * 	Added vsnprintf().
 * 	[89/08/14  14:16:49  bww]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)10  1.1  src/bldenv/sbtools/libode/porting/vsprintf.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:08";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: vsprintf.c,v $ $Revision: 2.8.2.3 $ (OSF) $Date: 1992/12/03 17:22:05 $";
#endif
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/***
 ***  "vsprintf.c	5.2 (Berkeley) 6/27/88";
 ***/

#include <stdio.h>
#include <varargs.h>

#if	!_DOPRNT_IS_VISIBLE
/* 
 *  No new architectures make _doprnt() visible.
 */
#define	_doprnt	_doprnt_va
#endif

int
vsprintf(str, fmt, ap)
	char *str, *fmt;
	va_list ap;
{
	FILE f;
	int len;

	f._flag = _IOWRT+_IOSTRG;
	f._ptr = str;
	f._cnt = 32767;
	len = _doprnt(fmt, ap, &f);
	*f._ptr = 0;
	return (len);
}

int
vsnprintf(str, n, fmt, ap)
	char *str, *fmt;
	va_list ap;
{
	FILE f;
	int len;

	f._flag = _IOWRT+_IOSTRG;
	f._ptr = str;
	f._cnt = n-1;
	len = _doprnt(fmt, ap, &f);
	*f._ptr = 0;
	return (len);
}
