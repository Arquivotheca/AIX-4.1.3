static char sccsid[] = "src/bos/diag/tu/370pca/crc_gen.c, tu_370pca, bos411, 9428A410j 8/8/91 15:28:44";
#include "pscatu.h"
#include "psca.h"		/* for NOSWAP */

/*****************************************************************************

Function(s) Cyclic Redundancy Checksum (CRC) Algorithm

Module Name :  crc_gen.c
SCCS ID	    :  1.1

Current Date:  8/29/89, 14:05:11
Newest Delta:  8/13/89, 11:33:48

Function computes CRC on specified buf of len bytes.

*****************************************************************************/

/*
 *
 * crc_gen
 *
 *
 * Function calculates CRC.  Based on modified algorithms by
 * Jim Darrah and listing from PC/RT Hardware Technical Reference Manual,
 * Volume I.
 *
 */
#include <stdio.h>
#define CRC_MASK	0XFF07

/*****************************************************************************
*   Function name - combine
*
*   Purpose - This function can be used to swap the two bytes of a two byte
*	      value, or to concatenate two separate bytes into a two byte
*	      value.  The bytes are passed in the order they are to be
*	      combined in.
*
*   Inputs - byte1:  most significant byte
*	     byte2:  least significant byte
*
*   Return - 16 bit value make up of the passed bytes.
*
*   Author - Jim Darrah	 7 June 1988
*
*   Mods -
*
*****************************************************************************/

unsigned short combine (byte1, byte2)
   unsigned char byte1,		/* msb */
		 byte2;		/* lsb */
   {						   /* start of combine() */
	static unsigned short ret_value;

	ret_value = (byte1 << 8) | byte2;
	return(ret_value);
   }						   /* end of combine() */

/*****************************************************************************

crc_gen

Function that does actual computation of CRC.

*****************************************************************************/

int crc_gen (buf, len)
   unsigned char *buf;
   int len;
   {
	union accum
	   {
		unsigned short whole;	/* defines entire 16 bits */
		struct bytes
		   {			/* used to define 2 bytes */
#ifdef NOSWAP
			unsigned char lsb;
			unsigned char msb;
#else
			unsigned char msb;
			unsigned char lsb;
#endif
		   } byte;
	   } avalue, dvalue;

	static unsigned short ret_crc;	/* value to be returned */
	unsigned char datav;
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
   }					/* end of crc_chk */
