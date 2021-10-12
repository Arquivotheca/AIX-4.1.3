/* @(#)51       1.3  src/bldenv/make/sprite.h, bldprocess, bos412, GOLDA411a 1/19/94 16:31:32
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
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
 * $Log: sprite.h,v $
 * Revision 1.2.5.2  1993/05/04  18:21:55  damon
 * 	CR 435. Fixed definition of NIL. Was broken by 64 bits
 * 	[1993/05/04  18:21:45  damon]
 *
 * Revision 1.2.5.1  1993/05/04  18:21:54  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.2.2.3  1992/12/03  19:07:18  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:33  damon]
 * 
 * Revision 1.2.2.2  1992/09/24  19:27:09  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:34  gm]
 * 
 * Revision 1.2  1991/12/05  20:45:13  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  17:19:35  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:10:22  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
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
 *
 *	@(#)sprite.h	5.3 (Berkeley) 6/1/90
 */

/*
 * sprite.h --
 *
 * Common constants and type declarations for Sprite.
 */

#ifndef _SPRITE
#define _SPRITE


/*
 * A boolean type is defined as an integer, not an enum. This allows a
 * boolean argument to be an expression that isn't strictly 0 or 1 valued.
 */

typedef int Boolean;
#ifndef TRUE
#define TRUE	1
#endif /* TRUE */
#ifndef FALSE
#define FALSE	0
#endif /* FALSE */

/*
 * Functions that must return a status can return a ReturnStatus to
 * indicate success or type of failure.
 */

typedef int  ReturnStatus;

/*
 * The following statuses overlap with the first 2 generic statuses 
 * defined in status.h:
 *
 * SUCCESS			There was no error.
 * FAILURE			There was a general error.
 */

#define	SUCCESS			0x00000000
#define	FAILURE			0x00000001


/*
 * A nil pointer must be something that will cause an exception if 
 * referenced.  There are two nils: the kernels nil and the nil used
 * by user processes.
 */

/* It is important that NIL be -1 so that it will work on */
/* 32 or 64 bit architectures */
#define NIL 		-1
#define USER_NIL 	0
#ifndef NULL
#define NULL	 	0
#endif /* NULL */

/*
 * An address is just a pointer in C.  It is defined as a character pointer
 * so that address arithmetic will work properly, a byte at a time.
 */

typedef char *Address;

/*
 * ClientData is an uninterpreted word.  Unlike an "Address",
 * client data will generally not be used in arithmetic.
 */

typedef void *ClientData;

#endif /* _SPRITE */
