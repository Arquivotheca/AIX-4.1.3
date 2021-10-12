static char sccsid[] = "@(#)29	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckkjtb.c, libKJI, bos411, 9428A410j 7/23/92 03:16:02";
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
 * MODULE NAME:       _Kckkjtb
 *
 * DESCRIPTIVE NAME:  MAKE TABLE OF CANDIDATES FOR KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x1104(JTEOVER): JTE overflow
 *                    0x1204(JKJOVER): JKJ overflow
 *                    0x7fff (UERROR): unpredictable error
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
short  _Kckkjtb(z_kcbptr)
struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   short   _Kckcanr();
   extern   short   _Kckcan1();
   extern   short   _Kckcan2();
   extern   short   _Kckcan3();

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_END        0x01ff
#define   Z_CONTINUE   0x02ff

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rc;

kcbptr1 = z_kcbptr;

/*----------------------------------------------------------------------*
 *      SIMPLE KANSUJI 
 *----------------------------------------------------------------------*/
   z_rc = _Kckcan1( z_kcbptr, 0, kcb.ychacyce );

   if ( z_rc == Z_END )
         return( SUCCESS );
   else
   {
      if ( z_rc != Z_CONTINUE)
         return( z_rc );
   }

/*----------------------------------------------------------------------*
 *      SIMPLE KANSUJI WITH UNIT
 *----------------------------------------------------------------------*/
   z_rc = _Kckcan2( z_kcbptr, 0, 0 );

   if ( z_rc  != SUCCESS)
      return( z_rc );

/*----------------------------------------------------------------------*
 *      COMPLEX KANSUJI 
 *----------------------------------------------------------------------*/
   z_rc = _Kckcan3( z_kcbptr, 0, 0 );

   if ( z_rc  != SUCCESS)
      return( z_rc );

/*----------------------------------------------------------------------*
 *      ROMAN NUMERIC
 *----------------------------------------------------------------------*/
   z_rc = _Kckcanr( z_kcbptr, 0, 0 );

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rc );
}
