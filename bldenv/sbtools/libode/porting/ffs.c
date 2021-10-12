/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		ffs
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
 * $Log: ffs.c,v $
 * Revision 1.2.2.2  1992/12/03  17:21:35  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:38  damon]
 *
 * Revision 1.2  1991/12/05  20:44:53  devrcs
 * 	Added ffs.c as porting option
 * 	[91/04/25  14:19:51  mckeen]
 * 
 * $EndLog$
 */
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)04  1.1  src/bldenv/sbtools/libode/porting/ffs.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:55";
#endif /* not lint */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "@(#)ffs.c	5.4 (Berkeley) 5/17/90";
#endif /* LIBC_SCCS and not lint */

#include <string.h>

/*
 * ffs -- vax ffs instruction
 */
ffs(mask)
	register int mask;
{
	register int bit;

	if (mask == 0)
		return(0);
	for (bit = 1; !(mask & 1); bit++)
		mask >>= 1;
	return(bit);
}
