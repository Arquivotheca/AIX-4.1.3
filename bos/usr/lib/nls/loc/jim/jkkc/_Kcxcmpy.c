static char sccsid[] = "@(#)04	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxcmpy.c, libKJI, bos411, 9428A410j 7/23/92 03:17:08";
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
 * MODULE NAME:       _Kcxcmpy
 *
 * DESCRIPTIVE NAME:  COMPARE STRING WITH 7BIT YOMI CODE IN YCE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:        1 (EQ_SHORT): equal but string1 is equal or shorter
 *                     2 (LONG):     equal but string1 is longer
 *                    -1 (NOT_EQU):  not equal
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETXCMPY _Kcxcmpy(z_kcbptr,z_str1,z_len1,z_off2,z_len2)
struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   *z_str1;                /* pointer of string1           */
short           z_len1;                 /* length of string1            */
unsigned char   z_off2;                 /* offset of string2            */
unsigned char   z_len2;                 /* length of string2            */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcyce.h"
#include   "_Kcmce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETXCMPY   z_ret;             /* rerative record number       */
   uschar            *z_ptr1;           /* pointer of string1           */
                                        /*   which moves in buffer      */
   struct YCE        *z_ptr2;           /* pointer of string2           */
                                        /*   which moves in buffer      */
   struct YCE        *z_stptr2;         /* save area of str2 pointer    */
   short             z_stlen2;          /* save area of str2 length     */
   short             z_i;               /* counter                      */
   short             z_flg;
   uschar	     z_offset;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   z_ptr1  = z_str1;                    /* set pointer of string1       */

/*----------------------------------------------------------------------*
 *      SET START POINTER OF STR2(YCEs) AND LENGTH
 *----------------------------------------------------------------------*/
   if ( z_off2 != 0 ) {                 /* if 1st MCE is the top of tb  */
      mceptr1 = kcb.mchmce + z_off2 - 1;
      z_ptr2 = mce.yceaddr + 1;         
   }
   else                                 /* if 1st MCE is middle entry   */
      z_ptr2 = kcb.ychyce;

   z_stptr2 = z_ptr2;                   /* set static pointer of str2   */
   mceptr1  = kcb.mchmce  + z_off2 + (short)z_len2 - 1;
   z_stlen2 = mce.yceaddr - z_stptr2 + 1;
   
/*----------------------------------------------------------------------*
 *       COMPARE STRING1 WITH STRING2  
 *----------------------------------------------------------------------*/
   if(( *z_ptr1 & M_E_MASK ) == ESC_ALPHA ) {
      if( !( *z_ptr1 & M_E_SHIFT )) {
         *z_ptr1 |= M_E_SHIFT;
         z_ptr1++;
         for( z_i = 0; z_i < z_len1 - 1 ; z_i++ ) 
	    *(z_ptr1+z_i) = OLDtoEMORA[*(z_ptr1+z_i)];
      }
      else
         z_ptr1++;
   }

   while ( 1 )
   {
      if ( z_ptr1 >= z_str1 + z_len1 ) { 
         z_ret.res = EQ_SHORT;
         break;
      }
      else if ( z_ptr2 >= z_stptr2 + z_stlen2 ) {
         z_ret.res = LONG;
         break;
      }
      else {
         yceptr1 = z_ptr2;
	 z_offset = 0;
	 if(( yce.yomi >= M_E_ALPHA_a ) && ( yce.yomi <= M_E_ALPHA_z ))
	      z_offset = M_E_OFFSET;
         if( *z_ptr1 != ( yce.yomi + z_offset )) {
            z_ret.res = NOT_EQU;
            break;
         }
         else { 
            z_ptr1++;
            z_ptr2++;
         }
      }
   }
   z_flg = 0;
   if ( z_ret.res == EQ_SHORT ) {
      for ( z_i = 0; z_i < z_len2; z_i++ ) {
         mceptr1 = kcb.mchmce + z_off2 + z_i;
         if ( ( z_ptr2 - (short)1 ) == mce.yceaddr ) {
            z_flg = 1;
            break;
         }
      }
      if ( z_flg != 1 ) {
         z_ret.len = 0;
         z_ret.res = NOT_EQU;
      }
      else
         z_ret.len = z_i + (short)1;
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
    return(z_ret);
}
