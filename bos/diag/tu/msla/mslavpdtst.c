static char sccsid[] = "@(#)24	1.3  src/bos/diag/tu/msla/mslavpdtst.c, tu_msla, bos411, 9428A410j 7/10/90 11:49:26";
/*
 * COMPONENT_NAME: ( mslavpdtest ) 
 *
 * FUNCTIONS: vpdtest, do_crc 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
 
#include "mslatu.h"
#include "mslafdef.h"
#include "mslaerrdef.h"

/* definitions for micro-channel adapter VPDROM data as architected */
#define CRC_LEN_DEF_OFFSET 4
#define CRC_OFFSET 6
#define CRC_START_OFFSET  8
#define CRC_MASK 0xff07
#define SUCCESS     0
 
#define COMBINE(x, y) (((x) << 8) | (y))

extern unsigned char  p_buf[200];

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  vpdtest.                                              */
/*                                                                           */
/*          PURPOSE :  Use the HTX interface to test MSLA  for the following */
/*                     vpdtest to test the VPD ROM on the microchannel       */
/*                     Interface Card.                                       */
/*                                                                           */
/*            INPUT :  Pointer to the mslatu structure.                      */
/*                                                                           */
/*           OUTPUT :  Returns the proper code.                              */
/*                                                                           */
/* FUNCTIONS CALLED :  mkerr                                                 */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int
vpdtest(gtu)
struct mslatu *gtu;
{
    short    crc_len;
    unsigned short crc_read, crc;
    unsigned short do_crc();
    int i, j, rc ;

    /*********************************************/
    /*    Read length of Vital Product Data      */
    /*********************************************/

    crc_len = p_buf[CRC_LEN_DEF_OFFSET]*256;       /* MSByte of length     */
    crc_len += p_buf[CRC_LEN_DEF_OFFSET+1];        /* Add LSbyte of length */
    crc_len *= 2;              /* vpd len is 1/2 num of bytes to be crc'ed */

    /**********************************************************************/
    /*    Save crc as read from ROM                                       */
    /**********************************************************************/
 
    crc_read =  (short) p_buf[CRC_OFFSET] * 256 ;       /* MSByte of CRC  */
    crc_read += (short) p_buf[CRC_OFFSET+1];            /* LSByte of CRC  */
 
    crc = do_crc( p_buf + CRC_START_OFFSET, crc_len, p_buf + CRC_OFFSET);
 
    if (crc==crc_read) 
       rc = SUCCESS ;
    else
    {
        rc =   1 ;
        mkerr(VPDTEST,1,rc,&rc);
    }
    return(rc);
}
 
/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  do_crc.                                               */
/*                                                                           */
/*          PURPOSE :  calculate crc of an char array                        */
/*                                                                           */
/*            INPUT :  p_buf, inbytes, crc_loc.                              */
/*                                                                           */
/*           OUTPUT :  Returns the sixteen_bit_crc.                          */
/*                                                                           */
/* FUNCTIONS CALLED :  COMBINE.                                              */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

unsigned short do_crc ( p_buf, inbytes, crc_loc )
uchar_t  *p_buf;
char  *crc_loc;
int   inbytes;
{
  struct bytes {
    uchar_t msb;
    uchar_t lsb;
  };
  union accum {
    short whole;
    struct bytes byte;
  } avalue, dvalue;
  unsigned char   datav;
  unsigned short  sixteen_bit_crc;
 
  /*************************************************/
  /* Translate each byte until end of file reached */
  /*************************************************/
  sixteen_bit_crc = 0;
  dvalue.whole = 0xffff;
 
  for ( ; inbytes > 0; inbytes--) {
    datav = *p_buf++;
    avalue.byte.lsb = (datav ^ dvalue.byte.lsb);
    dvalue.byte.lsb = avalue.byte.lsb;
    avalue.whole    = ((avalue.whole * 16) ^ dvalue.byte.lsb);
    dvalue.byte.lsb = avalue.byte.lsb;
    avalue.whole  <<= 8;
 
    avalue.whole >>= 1;
    avalue.byte.lsb ^= dvalue.byte.lsb;
 
    avalue.whole >>= 4;
 
    avalue.whole    = COMBINE(avalue.byte.lsb, avalue.byte.msb);
    avalue.whole    = ((avalue.whole & CRC_MASK) ^ dvalue.byte.lsb);
    avalue.whole    = COMBINE(avalue.byte.lsb, avalue.byte.msb);
    avalue.byte.lsb ^= dvalue.byte.msb;
    dvalue.whole    = avalue.whole;
  }
  sixteen_bit_crc = (dvalue.whole & 65535);
 
  return ( sixteen_bit_crc );
}
