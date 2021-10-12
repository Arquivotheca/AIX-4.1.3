/* @(#)89	1.1  src/htx/usr/lpp/htx/lib/hga/s3regs.h, tu_hga, htx410 6/2/94 11:37:31  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: S3ReadDP
 *		S3ReadData
 *		S3ReadRegister
 *		S3ReadRegisterShort
 *		S3WriteDP
 *		S3WriteData
 *		S3WriteIndex
 *		S3WriteRegister
 *		S3WriteRegisterSwappedLong
 *		S3WriteRegisterSwappedShort
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
#ifndef _S3_REGS_H_
#define _S3_REGS_H_
/*
 */
/*
 */

/*
 * ---------------------------------------------------------------------------
 * File: s3regs.h
 * 	- Contains the macros for accessing the S3's Vision928 a.k.a., 
 *	"homestake" graphics card's registers.
 * ---------------------------------------------------------------------------
 */

/*                    PCI Bus is big-endian                                  */
/*---------------------------------------------------------------------------*/
/*      define byte reversed inline code                                     */
/*---------------------------------------------------------------------------*/
void st_2r();                           /* prototype for 2-byte store        */
void st_4r();                           /* prototype for 4-byte store        */
#pragma mc_func st_2r { "7C60272C" }    /* sthbrx (vol_short_int)(gr4,0)=gr3 */
#pragma mc_func st_4r { "7C60252C" }    /* stbrx (vol_int)(gr4,0)=gr3        */
#pragma reg_killed_by st_2r             /* (none killed)                     */
#pragma reg_killed_by st_4r             /* (none killed)                     */
#pragma isolated_call (st_2r)
#pragma isolated_call (st_4r)

#define EIEIO     __iospace_eieio(); /* Enforced in-order execution of I/O. */

/*
 * ---------------------------------------------------------------------------
 *	macro: S3WriteRegister/S3ReadRegister
 *	description: This is the lowest level, most generic routine for 
 *		     accessing S3 registers.
 * ---------------------------------------------------------------------------
 */
#define S3WriteRegister(__register, __data)				\
{									\
    *(volatile unsigned char *)(ioBaseAddress | (__register)) = (__data);\
}

#define S3ReadRegister(__register, __data)				\
{									\
    __data = *(volatile unsigned char *)(ioBaseAddress | (__register));	\
}
#define S3ReadRegisterShort(__register, __data)				\
{									\
    __data = *(volatile unsigned short *)(ioBaseAddress | (__register));\
}


/*
 * ---------------------------------------------------------------------------
 *	macro: S3WriteRegisterSwappedShort/S3WriteRegisterSwappedLong
 *	description: This is the lowest level, most generic routine for 
 *		     writing swapped data to the S3 registers.
 * ---------------------------------------------------------------------------
 */
#define S3WriteRegisterSwappedShort(__register, __data)			\
{									\
    st_2r (__data, (ioBaseAddress | (__register)));			\
}
#define S3WriteRegisterSwappedLong(__register, __data)			\
{									\
    st_4r (__data, (ioBaseAddress | (__register)));			\
}

/*
 * ---------------------------------------------------------------------------
 * 	macro: S3WriteIndex
 *	description: The CRT Controller Index Register is loaded with a binary 
 *		     value that indexes the CRT Controller register where the 
 *		     data is to be accessed.
 * ---------------------------------------------------------------------------
 */
#define S3WriteIndex(__index)						\
{									\
    S3WriteRegister (S3CRTControlIndex, __index);			\
    EIEIO;								\
}

/*
 * ---------------------------------------------------------------------------
 *	macro: S3WriteData/S3ReadData
 *	description: This register is the data port for the CRT controller 
 *		     register indexed by the CRT Controller Address register.
 * ---------------------------------------------------------------------------
 */
#define S3WriteData(__data)						\
{									\
    S3WriteRegister (S3CRTControlData, __data);				\
    EIEIO;								\
}

#define S3ReadData(__data)						\
{									\
    S3ReadRegister (S3CRTControlData, __data);				\
}


/*
 * ---------------------------------------------------------------------------
 *	macro: S3WriteDP/S3ReadDP
 *	description: This macro should be used when writing to the data port.
 * ---------------------------------------------------------------------------
 */
#define S3WriteDP(__index, __data)					\
{									\
    S3WriteIndex (__index);						\
    S3WriteData  (__data);						\
}
#define S3ReadDP(__index, __data)					\
{									\
    S3WriteIndex (__index);						\
    S3ReadData  (__data);						\
}


/*
 * ---------------------------------------------------------------------------
 *	Controller Registers.
 * ---------------------------------------------------------------------------
 */
#define S3CRTControlIndex	0x3D4
#define S3CRTControlData	0x3D5

/*
 * ---------------------------------------------------------------------------
 *	Index offsets, this list should only contain indexes actually used. 
 * ---------------------------------------------------------------------------
 */
#define S3CursorFGColor	0x0E	/* When in 8bpp mode */
#define	S3CursorBGColor	0x0F	/* When in 8bpp mode */
#define S3RegisterLock2	0x39


/*
 * ---------------------------------------------------------------------------
 *	System control registers
 * ---------------------------------------------------------------------------
 */
#define S3SystemConfiguration	0x40
#define S3CursorMode		0x45
#define S3CursorOriginXHigh	0x46	/* High order 3 bits */
#define S3CursorOriginXLow	0x47	/* Low order 8 bits */
#define S3CursorOriginYHigh	0x48
#define S3CursorOriginYLow	0x49
#define S3CursorAddrStartHigh	0x4C	/* High order 4 bits */
#define S3CursorAddrStartLow	0x4D	/* Low order 8 bits */
#define S3CursorHotSpotX	0x4E
#define S3CursorHotSpotY	0x4F


/*
 * ---------------------------------------------------------------------------
 * 	System Extension Registers.
 * ---------------------------------------------------------------------------
 */
#define S3ExtSystemControl1	0x50
#define S3ExtSystemControl2	0x51
#define S3ExtBIOS1		0x52
#define S3ExtMemoryControl1	0x53
#define S3ExtMemoryControl2	0x54
#define S3ExtDACControl		0x55
#define S3ExternalSync1		0x56
#define S3ExternalSync2		0x57
#define S3LinearAddrWindowControl	0x58
#define S3LinearAddrWindowPositionHigh	0x59
#define S3LinearAddrWindowPositionLow	0x5A
#define S3ExtBIOS2		0x5B
#define S3GeneralOutPort	0x5C
#define S3ExtendedHorizontalOverflow	0x5D
#define S3ExtendedVerticalOverflow	0x5E
#define S3BusGrantTerminationPosition	0x5F





/*
 * ---------------------------------------------------------------------------
 *	Enhanced Command Register Descriptions
 * ---------------------------------------------------------------------------
#ifdef LITTLE_ENDIAN
 */
#if 1
#define S3SubsystemStatus		0x42E8
#define S3SubsystemControl		0x42E8
#define S3AdvancedFunctionControl	0x4AE8
#define S3CurrentY			0x82E8
#define S3CurrentX			0x86E8
#define S3DestinationY			0x8AE8
#define S3AxialStepConst		0x8AE8
#define S3DestinationX			0x8EE8
#define S3DiagonalStepConst		0x8EE8
#define S3ErrorTerm			0x92E8
#define S3MajorAxisPixelCount		0x96E8
#define S3ProcessorStatus		0x9AE8
#define S3DrawingCommand		0x9AE8
#define S3ShortStrokeVectorTransfer	0x9EE8
#define S3BackgroundColor		0xA2E8
#define S3ForegroundColor		0xA6E8
#define S3WriteMask			0xAAE8
#define S3ReadMask			0xAEE8
#define S3ColorCompare			0xB2E8
#define S3BackgroundMix			0xB6E8
#define S3ForegroundMix			0xBAE8
#define S3MultiFuncControl		0xBEE8	/* Upper 4 bits used as index */
#define S3PixelDataTransfer		0xE2E8
#define S3PixelDataTransfer_Ext		0xE2EA
#define S3MinorAxisPixelCount		0xBEE8
#else
#define S3SubsystemStatus		0xE842
#define S3SubsystemControl		0xE842
#define S3AdvancedFunctionControl	0xE84A
#define S3CurrentY			0xE882
#define S3CurrentX			0xE886
#define S3DestinationY			0xE88A
#define S3AxialStepConst		0xE88A
#define S3DestinationX			0xE88E
#define S3DiagonalStepConst		0xE88E
#define S3ErrorTerm			0xE892
#define S3MajorAxisPixelCount		0xE896
#define S3ProcessorStatus		0xE89A
#define S3DrawingCommand		0xE89A
#define S3ShortStrokeVectorTransfer	0xE89E
#define S3BackgroundColor		0xE8A2
#define S3ForegroundColor		0xE8A6
#define S3WriteMask			0xE8AA
#define S3ReadMask			0xE8AE
#define S3ColorCompare			0xE8B2
#define S3BackgroundMix			0xE8B6
#define S3ForegroundMix			0xE8BA
#define S3MultiFuncControl		0xE8BE	/* Upper 4 bits used as index */
#define S3PixelDataTransfer		0xE8E2
#define S3PixelDataTransfer_Ext		0xEAE2
#endif 


/*
 * ---------------------------------------------------------------------------
 * 	S3 VGA Registers
 * ---------------------------------------------------------------------------
 */

#define S3ConfigReg1	0x36

/* 
	S3 DAC Control Register 
*/
#define S3DACStatusRegister	0x3C7
#define S3DACCursorData		0x3C7
#define S3DACWriteIndexRegister	0x3C8
#define S3DACDataRegister	0x3C9

#define DAC_X_LOW		0x3C8
#define DAC_X_HIGH		0x3C9
#define DAC_Y_LOW		0x3C6
#define DAC_Y_HIGH		0X3C7

/*
	S3 Attribute Controller Registers 
*/

#define S3AttrControllerIndex	0x3C0


#define MF_MIN_AXIS_PCNT 0x0000
#define MF_SCISSORS_T    0x1000
#define MF_SCISSORS_L    0x2000
#define MF_SCISSORS_B    0x3000
#define MF_SCISSORS_R    0x4000
#define MF_PIX_CNTL      0xA000
#define MF_MULT_MISC     0xE000
#define MF_READ_SEL      0xF000


#endif  /* _S3_REGS_H_ */
