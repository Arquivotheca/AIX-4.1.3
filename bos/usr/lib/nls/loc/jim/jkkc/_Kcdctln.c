static char sccsid[] = "@(#)51	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcdctln.c, libKJI, bos411, 9428A410j 6/4/91 10:24:22";
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
 * MODULE NAME:       _Kcdctln
 *
 * DESCRIPTIVE NAME:  LEARNING OF JIRITU-GO AND FUZOKU-GO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0108(WSPOVER):    work space overflow
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
short _Kcdctln( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB              */
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern  short   _Kcofkkc();             /* main of kkc                 */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
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
   kcb.ymill1   = mcb.ymill1;           /* length of input yomi        */
   kcb.ymill2   = mcb.ymill2;           /* offset to the J/F boundary  */
   kcb.seill    = mcb.seill;            /* current seisho length       */
   kcb.semll    = mcb.semll;            /* current seisho map length   */
   kcb.ymmll    = mcb.ymmll;            /* current yomi map length     */
   kcb.grmll    = mcb.grmll;            /* current grammer map length  */

   kcb.func     = FUNLEARN;             /* set function No. as learning*/

/*----------------------------------------------------------------------* 
 *      LEARN JIRITSU-GO & FUZOKU-GO
 *----------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );      /* call KKCOFKKC()             */

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   switch( z_rofkkc )
   {
      case SUCCESS:
      case WSPOVER:
                      return( z_rofkkc );/* return to caller            */
      default:        return( UERROR );
   }
}
