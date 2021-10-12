/* @(#)07	1.2  src/bos/usr/ccs/bin/make/buf.h, cmdmake, bos411, 9428A410j  6/20/94  10:48:47 */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * @(#)$RCSfile: buf.h,v $ $Revision: 1.2.2.3 $ (OSF) $Date: 1992/03/23 22:36:34 $
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
 *	@(#)buf.h	5.3 (Berkeley) 6/1/90
 */

/*-
 * buf.h --
 *	Header for users of the buf library.
 */

#ifndef _BUF_H
#define _BUF_H

#include    "sprite.h"

typedef struct Buffer *Buffer;
typedef unsigned char Byte;

typedef struct {
    int      size;   /* Current size of the buffer */
    Byte    *buffer; /* The buffer itself */
    Byte    *inPtr;  /* Place to write to */
    Byte    *outPtr; /* Place to read from */
} Buf, *BufPtr;

				/* Add a single byte to a buffer */
void	Buf_AddByte(const Buffer, const Byte);
				/* Add a range of bytes to a buffer */
void	Buf_AddBytes(const Buffer, const int, const Byte *);
				/* Get all bytes from buffer */
Byte	*Buf_GetAll(const Buffer, int *);
				/* Throw away some of the bytes */
void	Buf_Discard(Buffer, int);

/*-
 *-----------------------------------------------------------------------
 * Buf_Size --
 *      Returns the number of bytes in the given buffer. Doesn't include
 *      the null-terminating byte.
 *
 * Results:
 *      The number of bytes.
 *
 * Side Effects:
 *      None.
 *
 *-----------------------------------------------------------------------
 */
#define Buf_Size(buf) (((BufPtr)(buf))->inPtr - ((BufPtr)(buf))->outPtr)

				/* Initialize a buffer */
Buffer	Buf_Init(int);
				/* Destroy a buffer */
void	Buf_Destroy(const Buffer, const Boolean);

#define BUF_ERROR 256

#endif /* _BUF_H */
