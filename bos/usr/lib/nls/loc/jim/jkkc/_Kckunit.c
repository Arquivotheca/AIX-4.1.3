static char sccsid[] = "@(#)34	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckunit.c, libKJI, bos411, 9428A410j 7/23/92 03:16:05";
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
 * MODULE NAME:       _Kckunit
 *
 * DESCRIPTIVE NAME:  TRANSLATE 4 DIGITS TO 1 NUMBER
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       LENGTH(DBCS) OF KANJI
 *
 ******************** END OF SPECIFICATIONS *****************************/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif

#include   "_Kcmap.h"                   /* Define Constant file         */

/*----------------------------------------------------------------------*
 *      STATIC TABLES
 *----------------------------------------------------------------------*/
struct KJTBL {
               uschar kj[2];
             };

static struct KJTBL kjn1[10] = {{0x81,0x5a},    /* 0                    */
                                {0x88,0xea},    /* 1                    */
                                {0x93,0xf1},    /* 2                    */
                                {0x8e,0x4f},    /* 3                    */
                                {0x8e,0x6c},    /* 4                    */
                                {0x8c,0xdc},    /* 5                    */
                                {0x98,0x5a},    /* 6                    */
                                {0x8e,0xb5},    /* 7                    */
                                {0x94,0xaa},    /* 8                    */
                                {0x8b,0xe3}};   /* 9                    */

static  struct KJTBL kjun[4] = {{0x81,0x40},    /* dummy                */
                                {0x8f,0x5c},    /* 10                   */
                                {0x95,0x53},    /* 100                  */
                                {0x90,0xe7}};   /* 1000                 */


/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short   _Kckunit( z_mceptr, z_outbuf, z_length )
struct MCE     *z_mceptr;
struct KJTBL   z_outbuf[];
short          z_length;
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)  		*/

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short            z_i;
   short            z_idx;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   for ( z_i = z_length, mceptr1 = z_mceptr, z_idx = 0;
	 z_i > 0; z_i--, mceptr1++ ) {
      /*------------------------  NUMBER  ------------------------------*/
      if ( ( ( z_i == 1 ) || ( z_i == 4 ) || ( mce.code != NUM_MORA_1 ) ) 
        && ( mce.code != NUM_MORA_0 ) ) {
         z_outbuf[z_idx  ].kj[0] = kjn1[ mce.code - NUM_MORA_0 ].kj[0];
         z_outbuf[z_idx++].kj[1] = kjn1[ mce.code - NUM_MORA_0 ].kj[1];
      }
      /*------------------------  Unit Kanji  --------------------------*/
      if ( ( z_i != 1 ) && ( mce.code != NUM_MORA_0 ) ) {
         z_outbuf[z_idx  ].kj[0] = kjun[z_i-1].kj[0];
         z_outbuf[z_idx++].kj[1] = kjun[z_i-1].kj[1];
      }
   }

/*----------------------------------------------------------------------*
 *      RETURN THE LENGTH(DBCS) OF KANJI
 *----------------------------------------------------------------------*/
   return( z_idx );
}
