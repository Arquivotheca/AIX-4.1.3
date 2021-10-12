/* @(#)05	1.1  src/bos/usr/ccs/bin/make/bit.h, cmdmake, bos411, 9428A410j  6/3/93  09:44:26 */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Bit_Alloc
 *		Bit_Clear
 *		Bit_Copy
 *		Bit_Free
 *		Bit_IsClear
 *		Bit_IsSet
 *		Bit_NumBytes
 *		Bit_NumInts
 *		Bit_Set
 *		Bit_Zero
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
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
 * @(#)$RCSfile: bit.h,v $ $Revision: 1.2.2.3 $ (OSF) $Date: 1992/03/23 22:36:31 $
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
 *	@(#)bit.h	5.3 (Berkeley) 6/1/90
 */

/*
 * bit.h --
 *
 *	Definition of macros for setting and clearing bits in an array
 *	of integers.
 *
 *	It is assumed that "int" is 32 bits wide.
 */

#ifndef _BIT
#define _BIT

#include "sprite.h"

#define BIT_NUM_BITS_PER_INT	32
#define BIT_NUM_BITS_PER_BYTE	8

#define Bit_NumInts(numBits)	\
	(((numBits)+BIT_NUM_BITS_PER_INT -1)/BIT_NUM_BITS_PER_INT)

#define Bit_NumBytes(numBits)	\
	(Bit_NumInts(numBits) * sizeof(int))

#define Bit_Alloc(numBits, bitArrayPtr)  	\
        bitArrayPtr = (int *)malloc((unsigned)Bit_NumBytes(numBits)); \
        Bit_Zero((numBits), (bitArrayPtr))

#define Bit_Free(bitArrayPtr)	\
        free((char *)bitArrayPtr)

#define Bit_Set(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] |= \
				(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_IsSet(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] & \
				(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_Clear(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] &= \
				~(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_IsClear(numBits, bitArrayPtr) \
	(!(Bit_IsSet((numBits), (bitArrayPtr))))

#define Bit_Copy(numBits, srcArrayPtr, destArrayPtr) \
	bcopy((char *)(srcArrayPtr), (char *)(destArrayPtr), \
		Bit_NumBytes(numBits))

#define Bit_Zero(numBits, bitArrayPtr) \
	bzero((char *)(bitArrayPtr), Bit_NumBytes(numBits))

extern int	  Bit_FindFirstSet();
extern int	  Bit_FindFirstClear();
extern Boolean	  Bit_Intersect();
extern Boolean 	  Bit_Union();
extern Boolean 	  Bit_AnySet();
extern int  	  *Bit_Expand();
	 
#endif /* _BIT */
