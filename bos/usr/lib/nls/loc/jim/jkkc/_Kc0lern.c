static char sccsid[] = "@(#)94	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0lern.c, libKJI, bos411, 9428A410j 6/4/91 10:05:04";
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
 * MODULE NAME:       _Kc0lern
 *
 * DESCRIPTIVE NAME:  STUDY OF JIRITUGO OR FUZOKUGO
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS) : success
 *                    0x7fff(UERROR ) : fatal error
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
short   _Kc0lern( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB               */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kc0flrn();   /* Learning FUZOKUGO HYOKI      */
   extern void            _Kc0jlrn();   /* Leraning JIRITSUGO           */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short        z_rtcode;               /* set return code to caller    */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */
   semptr1 = kcb.semaddr;               /* initialize of semptr1        */
   gpwptr1 = kcb.gpwgpe;                /* initialize of gpwprt1        */

/*----------------------------------------------------------------------*
 *      CHANGE THE FUZOKUGO HYOKI LEVEL
 *----------------------------------------------------------------------*/

   if( sem.code[0] <= 0x07 )
   {
      gpw.hykilvl = sem.code[0];        /* change FUZOKUGO HYOKI level  */
   }

/*----------------------------------------------------------------------*
 *      STUDY OF FUZOKUGO HYOKI LEVEL
 *----------------------------------------------------------------------*/

   else if(( sem.code[0] == FZK_HOMO ) || ( sem.code[0] == ALLFZK_HOMO ))
   {
      if(( z_rtcode = _Kc0flrn( kcbptr1 )) != 0 )
      {
         return( z_rtcode );            /* return to caller with        */
      }                                 /*      _Kc0flrn()'s error code */
   }

/*----------------------------------------------------------------------*
 *      STUDY OF JIRITUGO
 *----------------------------------------------------------------------*/

   else
      if(( sem.code[0] == ALPHANUM   )||
         ( sem.code[0] == JRT        )||
         ( sem.code[0] == PREFIX     )||
         ( sem.code[0] == SUFFIX     )||
         ( sem.code[0] == NUM_PREFIX )||
         ( sem.code[0] == NUM_SUFFIX )||
         ( sem.code[0] == PRO_PREFIX )||
         ( sem.code[0] == PRO_SUFFIX ))
   {
      _Kc0jlrn( kcbptr1 );
   }

/*----------------------------------------------------------------------*
 *      SEISYO ATTRIBUTE ISN'T CORECT
 *----------------------------------------------------------------------*/

   else
   {
      return( UERROR );                 /* invarid SEISYO attribute     */
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/

   return( SUCCESS );

}
