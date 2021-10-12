static char sccsid[] = "@(#)44	1.1.1.1  src/bos/diag/tu/ethsal/crcgen.c, tu_ethsal, bos411, 9428A410j 3/6/92 15:08:12";
/*
 * COMPONENT NAME : tu_ethsal
 *
 * FUNCTIONS      : crc_gen
 *
 * ORIGINS        : 27
 * 
 * IBM CONFIDENTIAL -- 
 *
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/

/*
 * Cyclic Redundancy Checksum (CRC) Algorithm
 * Calculates CRC based on modified algorithms by
 * listing from PC/RT Hardware Technical Reference Manual,
 * Volume I.
*/

#include <ctype.h>

/* 
 * NAME : crc_gen
 *
 * DESCRIPTION :
 *
 * Generates CRC of input buffer.
 *     
 *
 * INPUT :
 *
 *   1. Data buffer.
 *   2. Buffer size.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   CRC.
 *
*/

#define CRC_MASK          0xFF07
#define TO_SHORT(msb,lsb) ((msb << 8) | lsb)

ushort_t crc_gen(uchar_t buf[], int len)
{
  int i;

  union accum
  {
    ushort_t whole;

    struct bytes
    {			
      uchar_t msb;
      uchar_t lsb;
    } byte;

  } avalue, dvalue;

/**************************************************************************
  Operate on each byte of data in the passed array, sending the data through
  the algorithm to generate a crc.  Then use the crc just generated as the
  base for the next pass of the data through the algorithm.
***************************************************************************/

  dvalue.whole = 0xffff;  /* required initialization */

  for (i = 0; i < len; i++)
  {
    dvalue.byte.lsb = avalue.byte.lsb = buf[i] ^ dvalue.byte.lsb;
    avalue.whole = (avalue.whole * 16) ^ dvalue.byte.lsb;
    dvalue.byte.lsb = avalue.byte.lsb;
    avalue.whole <<= 8;
    avalue.whole >>= 1;
    avalue.byte.lsb ^= dvalue.byte.lsb;
    avalue.whole >>=4;
    avalue.whole = TO_SHORT(avalue.byte.lsb, avalue.byte.msb);
    avalue.whole = (avalue.whole & CRC_MASK) ^ dvalue.byte.lsb;
    avalue.whole = TO_SHORT(avalue.byte.lsb, avalue.byte.msb);
    avalue.byte.lsb ^= dvalue.byte.msb;
    dvalue.whole = avalue.whole;
  }
  
  return(dvalue.whole);
}
