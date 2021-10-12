/* @(#)11	1.2  src/bos/usr/include/sys/lc_layout.h, libi18n, bos411, 9428A410j 9/10/93 10:11:17 */
/*
 *   COMPONENT_NAME: LIBI18N
 *
 *   FUNCTIONS: None
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _Lc_Layout_
#define _Lc_Layout_

#include <sys/types.h>
 /**************************************************************************/
 /*                             CONSTANTS                                  */ 
 /**************************************************************************/

#define TRUE	1
#define FALSE	0

/* 
 * Define Layout Values type used for LayoutSet/GetValues()
  *  Types are defined as 32 bit int split into 2 parts
  *  Lower 16 bits - LayoutTextDescriptor
  *  Higher 16 bits - any other type
  */
#define AllTextDescptors	0x0000ffff
#define Orientation             0x00000001
#define TypeOfText              0x00000002
#define Swapping                0x00000004
#define Numerals                0x00000008
#define TextShaping             0x00000010
#define ArabicSpecialShaping    0x00000020
#define ArabicOneCellShaping    0x00000040
#define WordBreak               0x00000080
#define BidiType                0x00000100

#define ActiveShapeEditing	(0x01<<16)
#define ActiveBidirection	(0x02<<16)
#define ShapeCharset		(0x03<<16)
#define ShapeCharsetSize	(0x04<<16)
#define ShapeContextSize	(0x05<<16)
#define InputMode   		(0x07<<16)

 /* constant to select a specified bit in the BIDI Attribute */

#define TEXT_VISUAL     	0x00000000
#define TEXT_IMPLICIT   	0x01000000
#define TEXT_EXPLICIT   	0x02000000

#define BIDI_DEFAULT   		0x00000000
#define BIDI_UCS       		0x04000000

#define ORIENTATION_LTR      	0x00000000
#define ORIENTATION_RTL      	0x00010000
#define ORIENTATION_CONTEXT_LTR 0x00020000
#define ORIENTATION_CONTEXT_RTL 0x00030000

#define NUMERALS_NOMINAL      	0x00000000
#define NUMERALS_NATIONAL       0x00002000
#define NUMERALS_CONTEXTUAL     0x00003000

#define BREAK              	0x00000200
#define NO_BREAK             	0x00000000

#define SWAPPING              	0x00000100
#define NO_SWAPPING             0x00000000

#define ONECELL_SEEN       	0x00000080
#define TWOCELL_SEEN       	0x00000000

#define TEXT_STANDARD       	0x00000000
#define TEXT_SPECIAL       	0x00000040
#define TEXT_COMPOSED           0x00000020

#define TEXT_SHAPED   		0x00000000
#define TEXT_NOMINAL      	0x00000010
#define TEXT_INITIAL     	0x00000011
#define TEXT_MIDDLE      	0x00000012
#define TEXT_FINAL       	0x00000013
#define TEXT_ISOLATED    	0x00000014
 
#define EDITINPUT               0
#define EDITREPLACE             1

 /**************************************************************************/
 /*                            STRUCTURES                                  */ 
 /**************************************************************************/

typedef char *LayoutObject;
 
typedef char BooleanValue;

typedef struct {
		  int name;
		  caddr_t value;
		}LayoutValueRec, *LayoutValues;

typedef struct {
			int front;   /* previous chars */
			int back;    /* succeeding chars */
		}LayoutEditSizeRec, *LayoutEditSize;


typedef struct {
		  int in;	/* input buffer description */
		  int out;	/* output description */
	       }LayoutTextDescriptorRec, *LayoutTextDescriptor;

#endif _Lc_Layout_
