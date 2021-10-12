static char sccsid[] = "@(#)12	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbchkp.c, libKJI, bos411, 9428A410j 6/4/91 10:11:02";
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
 * MODULE NAME:       _Kcbchkp
 *
 * DESCRIPTIVE NAME:  JUDGEMENT HINSHI BY CONECTION OF JRTSU-GO AND FZK-GO
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       Return value is hinshi code
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
short _Kcbchkp(z_kcbptr,z_jhinpos,z_fsetsu)
struct KCB   *z_kcbptr;                 /* KCB pointer area             */
uschar       z_jhinpos;                 /* jiritsugo 24 bit map         */
uschar       *z_fsetsu;                 /* fuzoku-go 24 bit map         */
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjtx.h"   /* Jiritsugo Tag eXchange table (JTX)           */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short      z_i;                      /* define loop counter          */
   uschar     z_byte,z_bit;             /* define work area             */
   uschar     z_jhinsi[3];

/*----------------------------------------------------------------------*  
*       DISISION HINSHI FROM COMPARING JIRITSUGO AND FUZOKUGO BIT MAP   *  
*-----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;

   jtxptr1 = kcb.jtxjtx + z_jhinpos;

   for(z_i=0;z_i<3;z_i++)               /* loop about three bytes       */
   {
      if(z_byte=(jtx.pos[z_i] & z_fsetsu[z_i]))
                                        /* ATENTION!!! '=' is in this if*/
                                        /*statement but it isn't mistake*/
                                        /* do AND buerian operation     */
         break;                         /* if hit on any byte then break*/
   }
   if (z_i < 3)                         /* if somebyte was hit          */
   {
      for(z_bit=0x80,z_i*=8;z_bit!=0;z_bit>>=1,z_i++) 
                                        /* loop for checking bit position*/
      {
         if (z_byte & z_bit)            /* if found bit position        */
            return(z_i+1);              /* return with its bit position */
      }                                 /* end of loop                  */
      return(0);                        /* return with 0                */
   }                                    /* end if                       */
   else
      return(0);                        /* return with 0                */
}                                       /* end of program               */
