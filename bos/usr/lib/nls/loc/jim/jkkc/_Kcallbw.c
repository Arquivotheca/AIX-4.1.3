static char sccsid[] = "@(#)01	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcallbw.c, libKJI, bos411, 9428A410j 6/4/91 10:06:59";
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
 * MODULE NAME:       _Kcallbw
 *
 * DESCRIPTIVE NAME:  RE-OUTPUT THE CANDIDETS TO THE OUTPUT AREA
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0002(END_CAND):   end of All Candidtes Conversion
 *                    0x7fff(UERROR):     unpredictable error
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
short _Kcallbw( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                    
 *----------------------------------------------------------------------*/
   extern short           _Kcofkkc();   /* Root of KKC Common Portion   */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rofkkc;    /* Define Area for Return of _Kcofkkc   */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1          */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1          */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB
 *----------------------------------------------------------------------*/
   kcb.reqcand = mcb.reqcand;           /* set request count            */
   kcb.func     = FUNALLBW;             /* all candidate backward      */

/*----------------------------------------------------------------------* 
 *      GET REQUESTED NUMBER OF CANDIDATES
 *----------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );      /* call KKCOFKKC()             */

   switch( z_rofkkc )
   {
      case SUCCESS:
      case END_CAND:
                        break;

      default:          return( UERROR );
   }

/*----------------------------------------------------------------------* 
 *      SET OUTPUT PARAMETERS IN MCB
 *----------------------------------------------------------------------*/
   mcb.ymill2   = kcb.ymill2;           /* offset to the J/F boundary  */
   mcb.seill    = kcb.seill;            /* current seisho length       */
   mcb.semll    = kcb.semll;            /* current seisho map length   */
   mcb.ymmll    = kcb.ymmll;            /* current yomi map length     */
   mcb.grmll    = kcb.grmll;            /* current grammer map length  */

   mcb.currfst  = kcb.currfst;          /* current fast cand number    */
   mcb.currlst  = kcb.currlst;          /* current last cand number    */
   mcb.outcand  = kcb.outcand;          /* number of candidates outbuf */

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rofkkc );                  /* return to caller            */
}
