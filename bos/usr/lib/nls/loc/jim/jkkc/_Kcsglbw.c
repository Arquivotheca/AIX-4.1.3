static char sccsid[] = "@(#)94	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcsglbw.c, libKJI, bos411, 9428A410j 6/4/91 15:25:00";
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
 * MODULE NAME:       _Kcsglbw
 *
 * DESCRIPTIVE NAME:
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
short  _Kcsglbw(z_mcbptr)
struct  MCB  *z_mcbptr;
{
/*----------------------------------------------------------------------* 
 *      EXTERN REFERENCE                                                   
 *----------------------------------------------------------------------*/
   extern   short   _Kcofkkc();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
  short   z_rtofkkc;

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
  mcbptr1 = z_mcbptr;                   /* establish address'ty to mcb  */
  kcbptr1 = mcb.kcbaddr;                /* establish address'ty to kcb  */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB 
 *----------------------------------------------------------------------*/
   kcb.reqcand = mcb.reqcand;
  kcb.func    = FUNSGLBW;

/*----------------------------------------------------------------------* 
 *      GET REQUESTED NUMBER OF SINGLE KANJI CANDIDATES 
 *----------------------------------------------------------------------*/
  z_rtofkkc = _Kcofkkc(kcbptr1);

   switch( z_rtofkkc )
   {
      case SUCCESS:
      case END_CAND:
                        break;

      default:          return( UERROR );
   }

/*----------------------------------------------------------------------* 
 *      SET OUTPUT PARAMETERS IN MCB 
 *----------------------------------------------------------------------*/
   mcb.seill   = kcb.seill;
   mcb.outcand = kcb.outcand;
   mcb.currfst = kcb.currfst;
   mcb.currlst = kcb.currlst;

/*----------------------------------------------------------------------* 
 *      RETURN  
 *----------------------------------------------------------------------*/
   return(z_rtofkkc);
}
