static char sccsid[] = "@(#)91	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0flrn.c, libKJI, bos411, 9428A410j 6/4/91 10:04:07";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kc0flrn
 *
 * DESCRIPTIVE NAME:  STUDY OF FUZOKUGO
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)     : success
 *                    0x7FFF(UERROR )     : fatal error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

short   _Kc0flrn( z_kcbptr )
struct  KCB     *z_kcbptr;              /* initialize of KCB           */
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short        z_fznumb;                /* set FUZOKUGO number         */
   short        z_count ;                /* counter of seisyo buffer    */
   short        z_hiraga;                /* set HIRAGANA flag           */

/*---------------------------------------------------------------------*
 *      START OF PROCESS
 *---------------------------------------------------------------------*/

   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1       */
   grmptr1 = kcb.grmaddr;               /* initialize of grmptr1       */
   seiptr1 = kcb.seiaddr;               /* initialize of seiptr1       */

/*---------------------------------------------------------------------*
 *      SET FUZOKUGO NUMBER BY USING GRAMMER MAP
 *---------------------------------------------------------------------*/

   if( kcb.grmll == 2 )                 /* if grammer map length = 2   */
   {
      z_fznumb = (short)grm.byte[0];    /* set FUZOKUGO number         */
   }
   else if( kcb.grmll == 3 )            /* if grammer map lingth = 3   */
   {
      z_fznumb = (short)(grm.byte[0] & 0x7f);
      z_fznumb <<= 8;
      z_fznumb += (short)grm.byte[1];
   }                                    /* caluculate & set FUZOKUGO No*/
   else
   {
      return( (short)UERROR );          /* grammer map isn't corect    */
   }

/*---------------------------------------------------------------------*
 *      STUDY THAT ALL CODE IN SEISYO BUFFER ARE HIRAGANA OR KANJI
 *---------------------------------------------------------------------*/

   if( z_fznumb > 0 )                   /* if there are KANJI HYOKI    */
   {
      z_hiraga = ON;                    /* set HIRAGANA flag ON        */

      for ( z_count = 0; z_count < (kcb.seill - 2) / 2; z_count++ )
      {
         if( sei.kj[ z_count ][ 0 ] != 0x82 )
         {
            z_hiraga = OFF;             /* there is KANJI              */
            break;
         }
         else
         {
            if((sei.kj[z_count][1] < 0x9f)||(sei.kj[z_count][1] > 0xf1))
            {
               z_hiraga = OFF;          /* there is KANJI              */
               break;
            }
         }
      }

      if( z_hiraga == ON )              /* all seisyo code are HIRAGANA*/
      {
         kcb.dfgdfg[ z_fznumb ] = 0x0f;
      }
      else                              /* there are KANJI             */
      {
         kcb.dfgdfg[ z_fznumb ] = 0xf0;
      }
   }
   else
      return( (short)UERROR );          /* FUZOKUGO number is'nt corect*/
/*---------------------------------------------------------------------*
 *      RETURN
 *---------------------------------------------------------------------*/

   return( SUCCESS );                   /* _Kc0flrn successful         */

}

