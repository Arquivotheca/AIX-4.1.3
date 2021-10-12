static char sccsid[] = "@(#)08	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcapcnv.c, libKJI, bos411, 9428A410j 6/4/91 10:09:59";
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
 * MODULE NAME:       _Kcapcnv
 *
 * DESCRIPTIVE NAME : TRY TO PREVIOUS CONVERT
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x0002(NOMORE_CAND): No more candidate
 *                    0x0004(NO_CAND)    : No candidate
 *                    0x1104(JTEOVER)    : JTE table overflow
 *                    0x1204(JKJOVER)    : JKJ table overflow
 *                    0x1304(JLEOVER)    : JLE table overflow
 *                    0x1404(FTEOVER)    : FTE overflow
 *                    0x1704(BTEOVER)    : BTE overflow
 *                    0x1804(PTEOVER)    : PTE overflow
 *                    0x2104(SEIOVER)    : seisho buffer overflow
 *                    0x2204(SEMOVER)    : seisho map buffer overflow
 *                    0x2304(YMMOVER)    : yomi map buffer overflow
 *                    0x2404(GRMOVER)    : grammar map buffer overflow
 *                    0x0108(WSPOVER)    : work space overflow
 *                    0x7FFF(UERROR )    : Fatal error
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
short   _Kcapcnv( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB              */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcancv1();   /* Making Next Conv Env. for A/N*/
   extern short           _Kconcv2();   /* Set Candidate for Next Conv. */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rancv1;    /* Define Area for Return of _Kcancv1   */
   short           z_roncv2;    /* Define Area for Return of _Kconcv2   */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcb pointer    */

/*----------------------------------------------------------------------*
 *      MAKE AN ENVIRONMENT
 *----------------------------------------------------------------------*/
   z_rancv1 = _Kcancv1( kcbptr1 );

   if ( z_rancv1 != SUCCESS )
      return( z_rancv1 );

/*----------------------------------------------------------------------*
 *      MAKE A CANDIDATE AND OUTPUT IT TO THE OUTPUT AREA
 *----------------------------------------------------------------------*/
   z_roncv2 = _Kconcv2( kcbptr1, (short)PREVCNV );

   if ( z_roncv2 != SUCCESS )
      return(z_roncv2);

   kcb.ymill2 = kcb.ymill1;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
