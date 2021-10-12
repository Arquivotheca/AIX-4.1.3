/* @(#)87	1.1  src/htx/usr/lpp/htx/lib/hga/hgautils.h, tu_hga, htx410 6/2/94 11:37:27  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: HGAFillBlockSolid
 *		HGAHandleRasterAttrs
 *		HGAProcessorIdle
 *		HGAWaitForFiFo
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _HGAUTILS_H
#define _HGAUTILS_H

#define HGAHandleRasterAttrs(pGC)					\
{									\
    S3WriteRegisterSwappedShort (S3ForegroundMix,  			\
				(short)(0x00000020 | (0x000000ff & 0x7)));\
    S3WriteRegisterSwappedShort (S3WriteMask, (short)pGC->planemask);	\
    S3WriteRegisterSwappedShort (S3ForegroundColor, (short)pGC->fgPixel);\
    S3WriteRegister (S3MultiFuncControl, (char)(0xE));			\
    S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)(0xA000));	\
}

#define ZeroSlots	0xFF
#define OneSlots	0x7F
#define TwoSlots	0x3F
#define ThreeSlots 	0x1F
#define FourSlots	0x0F
#define FiveSlots	0x07
#define SixSlots	0x03
#define SevenSlots	0x01
#define EightSlots	0x00

/*
 * ---------------------------------------------------------------------------
 *	macro: HGAWaitForFiFo
 *	description:
 *		- Wait for the fifo to have at least "slotsNeeded", slots
 *		open.  This loop is counter-intuitive because the greater
 *		the number returned from the processor query the fewer
 *		slots there are availiable.
 * ---------------------------------------------------------------------------
 */
#define HGAWaitForFiFo(slotsNeeded)					\
{									\
    short slotsOpen=ZeroSlots;						\
    EIEIO;								\
    do									\
    {									\
	S3ReadRegister (S3ProcessorStatus, slotsOpen);			\
    }									\
    while (slotsOpen > slotsNeeded);					\
}

#define HGAFillBlockSolid(x, y, w, h)					\
{									\
    S3WriteRegisterSwappedShort (S3CurrentX, (short) x);		\
    S3WriteRegisterSwappedShort (S3CurrentY, (short) y);		\
    S3WriteRegisterSwappedShort (S3MajorAxisPixelCount, (short)(w-1));	\
    S3WriteRegister 		(S3MultiFuncControl, (short)(0x0));	\
    S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)(h-1));	\
    EIEIO;								\
    S3WriteRegisterSwappedShort (S3DrawingCommand, (short)(0x40F3));	\
}

/*
 * ---------------------------------------------------------------------------
 *	macro: HGAProcessorIdle()
 *	description:
 *		- Waits for the S3 graphics processor to be idle.
 * ---------------------------------------------------------------------------
 */
#define HGAProcessorIdle()						\
{									\
    short ProcessorStatus = 0;						\
    do									\
    {									\
	S3ReadRegisterShort (S3ProcessorStatus, ProcessorStatus);	\
    }									\
    while ((ProcessorStatus & 0x02));					\
}

#endif /* _HGAUTILS_H */
