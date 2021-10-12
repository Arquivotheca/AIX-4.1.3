static char sccsid[] = "@(#)74	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcprvcv.c, libKJI, bos411, 9428A410j 6/4/91 15:20:09";
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
 * MODULE NAME:       _Kcprvcv
 *
 * DESCRIPTIVE NAME:  OUTPUT THE PREVIOUS CANDIDET TO THE OUTPUT AREA
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE: 0x0000( SUCCESS ):     success
 *              0x0002( NOMORE_CAND ): no more next or prev candidates exist
 *              0x2104( SEIOVER ):     SEI overflow
 *              0x2204( SEMOVER ):     SEM overflow
 *              0x2304( YMMOVER ):     YMM overflow
 *              0x2404( GRMOVER ):     GRM overflow
 *              0x0108( WSPOVER ):     work space overflow
 *              0x7fff( UERROR ):      unpredictable error
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
short _Kcprvcv( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE                                         
 *----------------------------------------------------------------------*/
extern  short   _Kcofkkc();             /* main of kkc                  */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINE LOCAL VARIABLES      
 *----------------------------------------------------------------------*/
   short        z_rtofkkc;              /* set return code              */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS      
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1           */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1           */

/*----------------------------------------------------------------------*
 *      SET INPUT PARAMETERS IN KCB      
 *----------------------------------------------------------------------*/
   kcb.mode     = mcb.mode;             /* copy conversion mode         */
   kcb.func     = FUNPRVCV;             /* previous conversion          */

   kcb.ymill1   = mcb.ymill1;           /* length of input yomi         */
   kcb.ymill2   = mcb.ymill2;           /* offset to the J/F boundary   */
   kcb.seill    = mcb.seill;            /* current seisho length        */
   kcb.semll    = mcb.semll;            /* current seisho map length    */
   kcb.grmll    = mcb.grmll;            /* current grammer map length   */

/*----------------------------------------------------------------------*
 *      GET PREVIOUS CANDIDATE
 *----------------------------------------------------------------------*/
   z_rtofkkc = _Kcofkkc( kcbptr1 );     /* call KKCOFKKC()              */

   switch( z_rtofkkc )
   {
      case SUCCESS:
      case NOMORE_CAND:

      case SEIOVER:
      case SEMOVER:
      case YMMOVER:
      case GRMOVER:

      case WSPOVER:
                        break;

      default:          return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      SET OUTPUT PARAMETERS IN MCB      
 *----------------------------------------------------------------------*/
   mcb.ymill2 = kcb.ymill2;
   mcb.seill = kcb.seill;
   mcb.semll = kcb.semll;
   mcb.ymmll = kcb.ymmll;
   mcb.grmll = kcb.grmll;

/*----------------------------------------------------------------------*
 *      RETURN      
 *----------------------------------------------------------------------*/
   return( z_rtofkkc );                 /* return to caller             */
}
