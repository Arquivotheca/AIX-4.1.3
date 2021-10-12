/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: BufExpand
 *		Buf_AddBytes
 *		Buf_Destroy
 *		Buf_Discard
 *		Buf_GetByte
 *		Buf_GetBytes
 *		Buf_Init
 *		Buf_OvAddByte
 *		Buf_UngetByte
 *		Buf_UngetBytes
 *		max
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
 * $Log: buf.c,v $
 * Revision 1.2.2.3  1992/12/03  19:04:56  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:53  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:23:23  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:53:30  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:05  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:36:36  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  15:42:15  mckeen]
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
 */

#ifndef lint
static char sccsid[] = "@(#)32  1.4  src/bldenv/make/buf.c, bldprocess, bos412, GOLDA411a 1/19/94 16:26:23";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)buf.c	5.5 (Berkeley) 12/28/90";
#endif /* not lint */

/*-
 * buf.c --
 *	Functions for automatically-expanded buffers.
 */

#include    "make.h"
#include    "buf.h"

#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#endif

/*
 * BufExpand --
 * 	Expand the given buffer to hold the given number of additional
 *	bytes.
 *	Makes sure there's room for an extra NULL byte at the end of the
 *	buffer in case it holds a string.
 */
#define BufExpand(bp,nb) \
 	if (bp->left < (nb)+1) {\
	    int newSize = (bp)->size + max((nb)+1,BUF_ADD_INC); \
	    Byte  *newBuf = (Byte *) realloc((bp)->buffer, newSize); \
	    \
	    (bp)->inPtr = newBuf + ((bp)->inPtr - (bp)->buffer); \
	    (bp)->outPtr = newBuf + ((bp)->outPtr - (bp)->buffer);\
	    (bp)->buffer = newBuf;\
	    (bp)->size = newSize;\
	    (bp)->left = newSize - ((bp)->inPtr - (bp)->buffer);\
	}

#define BUF_DEF_SIZE	256 	/* Default buffer size */
#define BUF_ADD_INC	256 	/* Expansion increment when Adding */
#define BUF_UNGET_INC	16  	/* Expansion increment when Ungetting */

/*-
 *-----------------------------------------------------------------------
 * Buf_OvAddByte --
 *	Add a single byte to the buffer.  left is zero or negative.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The buffer may be expanded.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_OvAddByte (register Buffer bp, int byte)
{

    bp->left = 0;
    BufExpand (bp, 1);

    *bp->inPtr++ = byte;
    bp->left--;

    /*
     * Null-terminate
     */
    *bp->inPtr = 0;
}

/*-
 *-----------------------------------------------------------------------
 * Buf_AddBytes --
 *	Add a number of bytes to the buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Guess what?
 *
 *-----------------------------------------------------------------------
 */
void
Buf_AddBytes (register Buffer bp, int numBytes, Byte *bytesPtr)
{

    BufExpand (bp, numBytes);

    memcpy (bp->inPtr, bytesPtr, numBytes);
    bp->inPtr += numBytes;
    bp->left -= numBytes;

    /*
     * Null-terminate
     */
    *bp->inPtr = 0;
}

/*-
 *-----------------------------------------------------------------------
 * Buf_UngetByte --
 *	Place the byte back at the beginning of the buffer.
 *
 * Results:
 *	SUCCESS if the byte was added ok. FAILURE if not.
 *
 * Side Effects:
 *	The byte is stuffed in the buffer and outPtr is decremented.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_UngetByte (register Buffer bp, int byte)
{

    if (bp->outPtr != bp->buffer) {
	bp->outPtr--;
	*bp->outPtr = byte;
    } else if (bp->outPtr == bp->inPtr) {
	*bp->inPtr = byte;
	bp->inPtr++;
	bp->left--;
	*bp->inPtr = 0;
    } else {
	/*
	 * Yech. have to expand the buffer to stuff this thing in.
	 * We use a different expansion constant because people don't
	 * usually push back many bytes when they're doing it a byte at
	 * a time...
	 */
	int 	  numBytes = bp->inPtr - bp->outPtr;
	Byte	  *newBuf;

	newBuf = (Byte *)emalloc(bp->size + BUF_UNGET_INC);
	memcpy ((char *)(newBuf+BUF_UNGET_INC), (char *)bp->outPtr, numBytes+1);
	bp->outPtr = newBuf + BUF_UNGET_INC;
	bp->inPtr = bp->outPtr + numBytes;
	free ((char *)bp->buffer);
	bp->buffer = newBuf;
	bp->size += BUF_UNGET_INC;
	bp->left = bp->size - (bp->inPtr - bp->buffer);
	bp->outPtr -= 1;
	*bp->outPtr = byte;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Buf_UngetBytes --
 *	Push back a series of bytes at the beginning of the buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	outPtr is decremented and the bytes copied into the buffer.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_UngetBytes (register Buffer bp, int numBytes, Byte *bytesPtr)
{

    if (bp->outPtr - bp->buffer >= numBytes) {
	bp->outPtr -= numBytes;
	memcpy ((void *)bp->outPtr, (void *)bytesPtr, numBytes);
    } else if (bp->outPtr == bp->inPtr) {
	Buf_AddBytes (bp, numBytes, bytesPtr);
    } else {
	int 	  curNumBytes = bp->inPtr - bp->outPtr;
	Byte	  *newBuf;
	int 	  newBytes = max(numBytes,BUF_UNGET_INC);

	newBuf = (Byte *)emalloc (bp->size + newBytes);
	memcpy((char *)(newBuf+newBytes), (char *)bp->outPtr, curNumBytes+1);
	bp->outPtr = newBuf + newBytes;
	bp->inPtr = bp->outPtr + curNumBytes;
	free ((char *)bp->buffer);
	bp->buffer = newBuf;
	bp->size += newBytes;
	bp->left = bp->size - (bp->inPtr - bp->buffer);
	bp->outPtr -= numBytes;
	memcpy ((char *)bp->outPtr, (char *)bytesPtr, numBytes);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Buf_GetByte --
 *	Return the next byte from the buffer. Actually returns an integer.
 *
 * Results:
 *	Returns BUF_ERROR if there's no byte in the buffer, or the byte
 *	itself if there is one.
 *
 * Side Effects:
 *	outPtr is incremented and both outPtr and inPtr will be reset if
 *	the buffer is emptied.
 *
 *-----------------------------------------------------------------------
 */
int
Buf_GetByte (register Buffer bp)
{
    int	    res;

    if (bp->inPtr == bp->outPtr) {
	return (BUF_ERROR);
    } else {
	res = (int) *bp->outPtr;
	bp->outPtr += 1;
	if (bp->outPtr == bp->inPtr) {
	    bp->outPtr = bp->inPtr = bp->buffer;
	    bp->left = bp->size;
	    *bp->inPtr = 0;
	}
	return (res);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Buf_GetBytes --
 *	Extract a number of bytes from the buffer.
 *
 * Results:
 *	The number of bytes gotten.
 *
 * Side Effects:
 *	The passed array is overwritten.
 *
 *-----------------------------------------------------------------------
 */
int
Buf_GetBytes (register Buffer bp, int numBytes, Byte *bytesPtr)
{
    
    if (bp->inPtr - bp->outPtr < numBytes) {
	numBytes = bp->inPtr - bp->outPtr;
    }
    memcpy (bytesPtr, bp->outPtr, numBytes);
    bp->outPtr += numBytes;

    if (bp->outPtr == bp->inPtr) {
	bp->outPtr = bp->inPtr = bp->buffer;
	bp->left = bp->size;
	*bp->inPtr = 0;
    }
    return (numBytes);
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Discard --
 *	Throw away bytes in a buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The bytes are discarded. 
 *
 *-----------------------------------------------------------------------
 */
void
Buf_Discard (register Buffer bp, int numBytes)
{

    if (bp->inPtr - bp->outPtr <= numBytes) {
	bp->inPtr = bp->outPtr = bp->buffer;
	bp->left = bp->size;
	*bp->inPtr = 0;
    } else {
	bp->outPtr += numBytes;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Init --
 *	Initialize a buffer. If no initial size is given, a reasonable
 *	default is used.
 *
 * Results:
 *	A buffer to be given to other functions in this library.
 *
 * Side Effects:
 *	The buffer is created, the space allocated and pointers
 *	initialized.
 *
 *-----------------------------------------------------------------------
 */
static Lst bufFreeList;
int nBufFree;
static Lst bigBufFreeList;
int nBigBufFree;
static Boolean inited = FALSE;

Buffer
Buf_Init (int size)	 	/* Initial size for the buffer */
{
    Buffer bp;			/* New Buffer */
    Lst bl;			/* Free Buffer list */
    int *nFree;

    if (!inited) {
	bufFreeList = Lst_Init();
	bigBufFreeList = Lst_Init();
	nBufFree = nBigBufFree = 0;
	inited = TRUE;
    }

    if (size <= 0) {
	size = BUF_DEF_SIZE;
	bl = bigBufFreeList;
	nFree = &nBigBufFree;
    } else if (size >= BUF_DEF_SIZE) {
	bl = bigBufFreeList;
	nFree = &nBigBufFree;
    } else {
	bl = bufFreeList;
	nFree = &nBufFree;
    }

    if (*nFree) {
	LstNode ln;

	ln = Lst_First(bl);
	bp = (Buffer) Lst_Datum(ln);
	Lst_Remove(bl, ln);
	if (bp == NULL || bp->buffer == NULL) {
	    printf("Buf_Init: NULL buffer\n");
	    bp = (Buffer)emalloc(sizeof(*bp));
	    bp->buffer = (Byte *)emalloc(size);
	    (*nFree)--;
	} else {
	    if (bp->size >= size)
		size = bp->size;
	    else {
		if (bp->size != 0)
		    free(bp->buffer);
		bp->buffer = (Byte *)emalloc(size);
	    }
	    (*nFree)--;
	}
    } else {
	bp = (Buffer)emalloc(sizeof(*bp));
	bp->buffer = (Byte *)emalloc(size);
    }

    bp->left = bp->size = size;
    bp->inPtr = bp->outPtr = bp->buffer;
    *bp->inPtr = 0;

    return (bp);
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Destroy --
 *	Nuke a buffer and all its resources.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The buffer is freed.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_Destroy (Buffer buf)  	/* Buffer to destroy */
{
    Lst bl;

    if (buf == NULL) {
	printf("Buf_Destroy: NULL\n");
	return;
    }
    if (buf->size >= BUF_DEF_SIZE) {
	bl = bigBufFreeList;
	nBigBufFree++;
    } else {
	bl = bufFreeList;
	nBufFree++;
    }
    Lst_AtFront(bl, (ClientData)buf);
}
