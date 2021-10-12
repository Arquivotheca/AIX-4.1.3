static char sccsid[] = "@(#)46  1.6  src/bos/diag/tu/sun/tvpd.c, tu_sunrise, bos411, 9437A411a 7/8/94 17:39:34";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: Function
 *              check_vpd
 *              combine
 *              crc_gen
 *              test_base_vpd
 *              test_codec_vpd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************************
Function(s) - VPD Register Test

Module Name :  tvpd.c
*****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include "sun_tu_type.h"
#include "error.h"
#include "suntu.h"

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
*****************************************************************************/

unsigned short combine (byte1, byte2)
   unsigned char byte1,         /* msb */
                 byte2;         /* lsb */
   {                                               /* start of combine() */
        static unsigned short ret_value;

        ret_value = (byte1 << 8) | byte2;
        return(ret_value);
   }                                               /* end of combine() */

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
                unsigned short whole;   /* defines entire 16 bits */
                struct bytes
                   {                    /* used to define 2 bytes */
                        unsigned char msb;
                        unsigned char lsb;
                   } byte;
           } avalue, dvalue;

        static unsigned short ret_crc;  /* value to be returned */
        unsigned char datav;
        int i;

        dvalue.whole = 0xffff;

/**************************************************************************
  Operate on each byte of data in the passed array, sending the data through
  the algorithm to generate a crc.  Then use the crc just generated as the
  base for the next pass of the data through the algorithm.
***************************************************************************/

        for (i = 0; len > 0; i++, len--)
           {                            /* step through the CRC area */
                datav = *(buf + i);     /* GET BYTE */
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
           }                            /* end step through the CRC area */
        ret_crc = dvalue.whole;
        return(ret_crc);
   }                                    /* end of crc_chk */

/*****************************************************************************

check_vpd

Function reads the VPD into the array vpd_data.  Once read into the array,
function checks for 'VPD' characters, then calculate the Cyclic Redundancy
Checksum (CRC) on the VPD and compares it to the one written in the adapter.

*****************************************************************************/
int check_vpd (unsigned int startaddr)
{
        unsigned char vpd_data[VPDREGSMAX+1];
        int i,j;
        int rc, error_code;
        unsigned int data;     /* for data reading */
        unsigned int vpd_size, vpd_data_len, vpd_crc, calc_crc;

        /* Set appropriate error codes for which VPD test is running */
        if (startaddr == VPDBASE_MIN) {
          error_code = ERRORVPD_BASE-0x10;
        } else {
          error_code = ERRORVPD_CODEC-0x10;
        } /* endif */

        /* Read and store VPD in an array */
        for (i=0,j=startaddr;j<(startaddr+0x3ff) ;i++,j=j+4 ) {
           if (rc = readchip(VPD, j ,1, &data)) return (rc);
           vpd_data[i] = (unsigned char) data;
        } /* endfor */

        if ((vpd_data[1]!='V') | (vpd_data[2]!='P') | (vpd_data[3]!='D'))
        {
          DEBUG_3("vpd_data[1] = %c, vpd_data[2] = %c, vpd_data[3] = %c\n",
                   vpd_data[1], vpd_data[2], vpd_data[3]);
          return(error_code+1);
        }

        /*
         * form CRC by combining MSB and LSB from VPD.
         */
        vpd_crc = (0x100 * vpd_data[6]) + vpd_data[7];

        /*
         * get the length specified in the VPD.  Note that
         * the designers of VPD placed the length divided by 2
         * (thus the number of "words"), so we multiply by 2
         * to get the total number of bytes.  Also note that
         * we do NOT need to look at the MSB of the length field
         * (vpd_data[4]) for the calculation of the length (i.e.
         * only vpd_data[5] - the LSB) since it is known that it
         * will be zero since the length is rather short.
         */

        vpd_data_len = 2 * vpd_data[5];
        DEBUG_1("The vpd_data_len=%d%d\n\r", vpd_data_len);

        calc_crc = crc_gen(&vpd_data[8], vpd_data_len);

        if (vpd_crc != calc_crc)
        {
          DEBUG_1("\nVPD  crc is %08X\n\r",vpd_crc);
          DEBUG_1("Calc crc is %08X\n\r",calc_crc);
          error.crc.crc_expected = vpd_crc;
          error.crc.crc_actual = calc_crc;
          return(error_code+2);
        }

        return(OK);
}

/*****************************************************************************

test_base_vpd

Function test the VPD in the BASE card.

*****************************************************************************/
int test_base_vpd ()
{
        int rc;

        /* Test VPD in BASE card */
        if (rc = check_vpd(VPDBASE_MIN))
        {
          error.crc.error_code = ERRORVPD_BASE;
          error.crc.sub_error_code = rc;
          log_error(error);
          return(ERRORVPD_BASE);
        }

        return(OK);
}

/*****************************************************************************

test_codec_vpd

Function test the VPD in the CODEC daughter card.

*****************************************************************************/
int test_codec_vpd ()
{
        int rc;

        /* Test VPD in CODEC card */
        if (rc = check_vpd(VPDCODEC_MIN))
        {
          error.crc.error_code = ERRORVPD_CODEC;
          error.crc.sub_error_code = rc;
          log_error(error);
          return(ERRORVPD_CODEC);
        }

        return(OK);
}
