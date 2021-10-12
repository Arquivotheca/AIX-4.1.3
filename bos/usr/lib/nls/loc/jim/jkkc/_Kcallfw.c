static char sccsid[] = "@(#)02	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcallfw.c, libKJI, bos411, 9428A410j 6/4/91 10:07:32";
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
 * MODULE NAME:       _Kcallfw
 *
 * DESCRIPTIVE NAME: OUTPUT THE NEXT CANDIDATE TO THE OUTPUT AREA
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
short _Kcallfw( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                   
 *----------------------------------------------------------------------*/
   extern       _Kcofkkc();             /* main of kkc                 */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rofkkc;                /* set return code             */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1          */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1          */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB
 *----------------------------------------------------------------------*/
   kcb.reqcand  = mcb.reqcand;          /* No. of candidates to caller */

   kcb.func     = FUNALLFW;             /* all candidate forward       */

/*----------------------------------------------------------------------* 
 *      GET REQUESTED NUMBER OF CANDIDATES 
 *----------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );

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
   return( z_rofkkc );
}
