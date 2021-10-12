/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: concat
 *		defined
 *		vconcat
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
 * $Log: concat.c,v $
 * Revision 1.7.7.1  1993/08/19  18:26:41  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:53  damon]
 *
 * Revision 1.7.5.2  1993/04/27  20:43:19  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:43:07  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:20:30  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:07:55  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:25:46  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:07  damon]
 * 
 * Revision 1.7  1991/12/05  21:04:28  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:10:53  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:05  dwm]
 * 
 * Revision 1.5  90/10/07  20:02:49  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:18  gm]
 * 
 * Revision 1.4  90/08/09  14:22:46  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:13  gm]
 * 
 * Revision 1.3  90/06/29  14:38:10  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:31  gm]
 * 
 * Revision 1.2  90/01/02  19:26:35  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:09  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * Revision 1.3  89/08/30  18:09:29  bww
 * 	Updated handling of variable argument lists.
 * 	[89/08/30  18:08:22  bww]
 * 
 * Revision 2.2  88/12/13  13:51:06  gm0w
 * 	Created.
 * 	[88/12/04            gm0w]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)74  1.1  src/bldenv/sbtools/libode/concat.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:47";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: concat.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/08/19 18:26:41 $";
#endif
/*
 * subroutine for concatenating strings into a buffer
 *
 * char *concat(buf, buflen, ptr1, ptr2, ..., NULL) { char *ep; return(ep); }
 * char *buf, *ptr1, *ptr2, ...;
 * int buflen;
 *
 * "buflen" should be sizeof("buf")
 * "buf" will be terminated by a null byte
 * "concat" will return a pointer to the null byte, if return is non-null
 *
 * concat will return null(0) under any of the following conditions:
 *    1) buf is null
 *    2) buflen <= 0
 *    3) buf was not large enough to hold the contents of all the ptrs.
 */

#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ode/util.h>

char *
vconcat( char *buf, int buflen, va_list ap );

/*VARARGS2*/
char *
#ifdef __STDC__
concat(char *buf, int buflen, ...)
#else
concat(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *buf;
    int buflen;
#endif
    va_list ap;
    char *ptr;

#ifdef __STDC__
    va_start(ap, buflen);
#else
    va_start(ap);
    buf = va_arg(ap, char *);
    buflen = va_arg(ap, int);
#endif
    ptr = vconcat(buf, buflen, ap);
    va_end(ap);
    return(ptr);
}

char *
vconcat( char *buf, int buflen, va_list ap )
{
    register char *arg, *ptr, *ep;

    if (buf == NULL)
	return(NULL);
    if (buflen <= 0)
	return(NULL);
    ptr = buf;
    *ptr = '\0';
    ep = buf + buflen;
    while (ptr != NULL && (arg = va_arg(ap, char *)) != NULL)
	while ((*ptr = *arg++))
	    if (++ptr == ep) {
		ptr = NULL;
		break;
	    }
    return(ptr);
}
