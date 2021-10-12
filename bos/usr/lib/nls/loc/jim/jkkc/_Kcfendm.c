static char sccsid[] = "@(#)58	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfendm.c, libKJI, bos411, 9428A410j 7/23/92 03:02:51";
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
 * MODULE NAME:       _Kcfendm
 *
 * DESCRIPTIVE NAME:  CHECK THE POSSIBILITY TO BE LAST MORA CODE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       POSSIBILITY YES/NO
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
short _Kcfendm(z_mcode)
uschar z_mcode;                         /* mora code                    */
{ 
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   static short z_emora[16] =	/* define possibility to be last	*/
   {                            /* char of this mora            	*/
    /* 0 */ 	0x0000,         /* 00000000 00000000    bit  00 -  15  	*/
    /* 1 */ 	0x0000,         /* 00000000 00000000    bit  16 -  31   */
    /* 2 */ 	0xFF7F,         /* 11111111 01111111    bit  32 -  47  	*/
    /* 3 */ 	0xB7DF,         /* 10110111 11011111    bit  48 -  63  	*/
    /* 4 */ 	0x6FFE,         /* 01101111 11111110    bit  64 -  79  	*/
    /* 5 */ 	0x4B0F,         /* 01001011 00001111    bit  80 -  95  	*/
    /* 6 */ 	0xA3F0,         /* 10100011 11110000    bit  96 - 111 	*/
    /* 7 */ 	0xC000,         /* 11000000 00000000    bit 112 - 127 	*/

    /* 8 */ 	0x0000,         /* 00000000 00000000    bit 128 - 143   */
    /* 9 */ 	0x0000,         /* 00000000 00000000    bit 144 - 159	*/
    /* A */ 	0x0000,         /* 00000000 00000000    bit 160 - 175	*/
    /* B */ 	0x0000,         /* 00000000 00000000    bit 176 - 191	*/
    /* C */ 	0x0001,         /* 00000000 00000001    bit 192 - 207	*/
    /* D */ 	0xF800,         /* 11111111 10000000    bit 208 - 223	*/
    /* E */ 	0x0000,         /* 00000000 00000000    bit 224 - 239	*/
    /* F */ 	0x0000          /* 00000000 00000000    bit 240 - 255 	*/
   }; 
   short z_ix,z_mx; 

/*----------------------------------------------------------------------*  
 *       CHECK THE POSSIBILITY TO BE LAST CHARACTER                        
 *----------------------------------------------------------------------*/ 
   z_ix = z_mcode / 16;                 /* calculate of subscript       */
   z_mx = z_mcode % 16;                 /* calculate bit position       */
   if ( z_emora[z_ix] & (0x8000 >> z_mx ) ) 
      return(YES);                      /* return with possible flag    */
   else 
      return(NO);                       /* return with impossible flag  */
}
