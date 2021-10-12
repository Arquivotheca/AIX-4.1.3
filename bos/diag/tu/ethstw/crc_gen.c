static char sccsid[] = "@(#)crc_gen.c	1.5 7/9/91 15:06:23";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	unsigned short combine() 
 *		int crc_gen()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Function(s) Cyclic Redundancy Checksum (CRC) Algorithm

Module Name :  crc_gen.c

STATUS:    

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

   TYPE, VERSION: AIX C Compiler, version:

   OPTIONS:

NOTES: Nones.
*****************************************************************************/
/*** header files ***/
#include <stdio.h>
#include "tu_type.h"

/*** constant(s) use in this file ***/
#define CRC_MASK        0XFF07

/*****************************************************************************
*   Function name - combine
*
*   Purpose - This function can be used to swap the two bytes of a two byte
*             value, or to concatenate two separate bytes into a two byte
*             value.  The bytes are passed in the order they are to be
*             combined in.
*
*   Inputs - byte1:  most significant byte
*            byte2:  least significant byte
*
*   Return - 16 bit value make up of the passed bytes.
*
*   Author - Jim Darrah  7 June 1988
*
*   Mods -
*
*****************************************************************************/
ushort combine(uchar byte1, uchar byte2)
   		 /* byte1 msb */
                 /* byte2 lsb */
   {   /* start of combine() */
	static unsigned short ret_value;

	ret_value = (byte1 << 8) | byte2;
	return(ret_value);
   }  /* end of combine() */

/****************************************************************************
 * Function: int crc_gen()
 *
 * Function calculates CRC.  Based on modified algorithms by
 * listing from PC/RT Hardware Technical Reference Manual,
 * Volume I.
 *****************************************************************************/
int crc_gen(uchar *buf, int len)
   {
	union accum
	   {
	   ushort whole;	/* defines entire 16 bits */
	   struct bytes
	   	{			/* used to define 2 bytes */
		uchar msb;
		uchar lsb;
	   	} byte;
	   } avalue, dvalue;

	static ushort ret_crc;	/* value to be returned */
	uchar datav;
	int i;

	dvalue.whole = 0xffff;

/**************************************************************************
  Operate on each byte of data in the passed array, sending the data through
  the algorithm to generate a crc.  Then use the crc just generated as the
  base for the next pass of the data through the algorithm.
***************************************************************************/

	for (i = 0; len > 0; i++, len--)
	   {				/* step through the CRC area */
	   datav = *(buf + i);	/* GET BYTE */
	   avalue.byte.lsb = datav ^ dvalue.byte.lsb;
	   dvalue.byte.lsb = avalue.byte.lsb;
	   avalue.whole = (avalue.whole * 16) ^ dvalue.byte.lsb;
	   dvalue.byte.lsb = avalue.byte.lsb;
	   avalue.whole <<= 8;

	   avalue.whole >>= 1;
	   avalue.byte.lsb ^= dvalue.byte.lsb;

	   avalue.whole >>=4;

	   avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
	   avalue.whole = (avalue.whole & CRC_MASK) ^ dvalue.byte.lsb;
	   avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
	   avalue.byte.lsb ^= dvalue.byte.msb;
	   dvalue.whole = avalue.whole;
	   }				/* end step through the CRC area */
	ret_crc = dvalue.whole;
	return(ret_crc);
   } /* end of crc_gen */
