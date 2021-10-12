static char sccsid[] = "@(#)84	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcicode.c, libKJI, bos411, 9428A410j 7/23/92 03:09:49";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcicode
 *
 * DESCRIPTIVE NAME:  CONVERSION TO MORA ABOUT ORDINARY CODE             *
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       mora code
 *                    0x00 (Z_INVALID): invalid 7 bit yomi code
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
unsigned char  _Kcicode( z_incode )

unsigned char  z_incode;                /* set 7 bits code              */
{
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_YOON      0x7E
#define   Z_INVALID   0x00
#define   Z_EM_A      0xE0		/* E.mora Alpha 'A'code         */
#define   Z_EM_Z      0xF9		/* E.mora Alpha 'Z'code         */
#define   Z_EM_a      0xB5		/* E.mora Alpha 'a'code         */
#define   Z_EM_z      0xCE		/* E.mora Alpha 'z'code         */
#define   Z_EM_OFFSET 0x2B		/* E.mora offset,lower to upper */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   static uschar z_table[128] = {
/* 0 */      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
      	     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
/* 1 */      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x00,
      	     0x00, 0x00, 0xB4, 0xB5, 0xB6, 0x00, 0x78, 0x1F,
/* 2 */      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
      	     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
/* 3 */      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
      	     0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
/* 4 */      0x36, 0x41, 0x42, 0x38, 0x44, 0x45, 0x46, 0x47,
      	     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
/* 5 */      0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
      	     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
/* 6 */      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
      	     0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
/* 7 */      0x70, 0x71, 0x00, 0x00, 0x74, 0xCF, 0xD0, 0xD1,
      	     0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9
   };

/*----------------------------------------------------------------------*
 *      NORMAL TRANSRASION FROM YOMI CODE TO MORA CODE
 *----------------------------------------------------------------------*/
   if( z_incode <= Z_YOON )
      return( z_table[ z_incode ] );    /* change 7bits to mora         */
   else { 
      if(( z_incode >= Z_EM_A ) && ( z_incode <= Z_EM_Z )) 
	  return( z_incode );
      else if(( z_incode >= Z_EM_a ) && ( z_incode <= Z_EM_z ))
	  return( z_incode + Z_EM_OFFSET );
      else	
          return( Z_INVALID );
   }
}
