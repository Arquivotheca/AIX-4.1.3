/* @(#)33       1.3  src/bldenv/make/buf.h, bldprocess, bos412, GOLDA411a 1/19/94 16:26:28
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Buf_AddByte
 *		Buf_GetBase
 *		Buf_Size
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
 * $Log: buf.h,v $
 * Revision 1.2.2.3  1992/12/03  19:04:58  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:55  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:23:29  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:53:36  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:08  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  15:42:20  mckeen]
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
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)buf.h	5.4 (Berkeley) 12/28/90
 */

/*-
 * buf.h --
 *	Header for users of the buf library.
 */

#ifndef _BUF_H
#define _BUF_H

#include    "sprite.h"

typedef unsigned char Byte;

typedef struct Buffer {
    int	    size; 	/* Current size of the buffer */
    int     left;	/* Space left (== size - (inPtr - buffer)) */
    Byte    *buffer;	/* The buffer itself */
    Byte    *inPtr;	/* Place to write to */
    Byte    *outPtr;	/* Place to read from */
} *Buffer;

Buffer	Buf_Init(int);		/* Initialize a buffer */
void	Buf_Destroy(Buffer);	/* Destroy a buffer */
void	Buf_OvAddByte(Buffer, int);
				/* Add a byte to a buffer */
void	Buf_AddBytes(Buffer, int, Byte *);
				/* Add a range of bytes to a buffer */
int	Buf_GetByte(Buffer);    /* Get a byte from a buffer */
int	Buf_GetBytes(Buffer, int, Byte *);
				/* Get multiple bytes */
void	Buf_UngetByte(Buffer, int);
				/* Push a byte back into the buffer */
void	Buf_UngetBytes(Buffer, int, Byte *);
				/* Push many bytes back into the buf */
void	Buf_Discard(Buffer, int);
				/* Throw away some of the bytes */

/* Buf_AddByte adds a single byte to a buffer. */
#define	Buf_AddByte(bp, byte) { \
    if (--(bp)->left <= 0) \
	Buf_OvAddByte(bp, byte); \
    else { \
	*(bp)->inPtr++ = (byte); \
	*(bp)->inPtr = 0; \
    } \
}

#define Buf_Size(bp)	((bp)->inPtr - (bp)->outPtr)
#define Buf_GetBase(bp)	((bp)->outPtr)

#define BUF_ERROR 256

#endif /* _BUF_H */
