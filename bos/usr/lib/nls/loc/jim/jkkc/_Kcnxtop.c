static char sccsid[] = "@(#)56	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcnxtop.c, libKJI, bos411, 9428A410j 6/4/91 15:16:17";
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
 * MODULE NAME:       _Kcnxtop
 *
 * DESCRIPTIVE NAME:  OPEN ENVIRONMANT OF NEXT CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *                   0x0000( SUCCESS ):     success
 *                   0x0002( NOMORE_CAND ): no more next or prev candidate exist
 *                   0x0004( NO_CAND ):     no candidate exist
 *                   0x1104( JTEOVER ):     JTE overflow
 *                   0x1204( JKJOVER ):     JKJ overflow
 *                   0x1304( JLEOVER ):     JLE overflow
 *                   0x1404( FTEOVER ):     FTE overflow
 *                   0x1704( BTEOVER ):     BTE overflow
 *                   0x2104( SEIOVER ):     SEI overflow
 *                   0x2204( SEMOVER ):     SEM overflow
 *                   0x2304( YMMOVER ):     YMM overflow
 *                   0x2404( GRMOVER ):     GRM overflow
 *                   0x0108( WSPOVER ):     work space overflow
 *                   0x7fff( UERROR ):      unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */


/***********************************************************************
 *      START OF FUNCTION
 ***********************************************************************/
short _Kcnxtop( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB              */
{
/*---------------------------------------------------------------------*
 *      EXTERNAL REFERENCE                                             
 *---------------------------------------------------------------------*/
   extern  short   _Kcofkkc();         

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*---------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLE                                            
 *---------------------------------------------------------------------*/
   short        z_rofkkc;

/*---------------------------------------------------------------------*
 *      SET BASE POINTERS                                            
 *---------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1          */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1          */

/*---------------------------------------------------------------------*
 *      COPY PARAMETERS FROM MCB TO KCB        
 *---------------------------------------------------------------------*/
   kcb.mode     = mcb.mode;             /* copy conversion mode        */
   kcb.func     = FUNNXTCV;             /* next conversion             */
   kcb.env = ENVNONE;

   kcb.ymill1   = mcb.ymill1;           /* length of input yomi        */
   kcb.ymill2   = mcb.ymill2;           /* offset to the J/F boundary  */

   kcb.seill    = mcb.seill;            /* current seisho length       */
   kcb.semll    = mcb.semll;            /* current seisho map length   */
   kcb.grmll    = mcb.grmll;            /* current grammer map length  */

   if ( mcb.mode == MODALKAN )
      kcb.alpkan = mcb.alpkan;

/*---------------------------------------------------------------------*
 *      MAKE TABLE OF CANDIDATES        
 *---------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );

   switch( z_rofkkc )
   {
      case SUCCESS:
      case NOMORE_CAND:
      case NO_CAND:
      case JTEOVER:
      case JKJOVER:
      case JLEOVER:
      case FTEOVER:
      case BTEOVER:
      case SEIOVER:
      case SEMOVER:
      case YMMOVER:
      case GRMOVER:
      case WSPOVER:
                        break;

      default:          return( UERROR );
   }

/*---------------------------------------------------------------------*
 *      COPY PARAMETERS FROM KCB TO MCB                                
 *---------------------------------------------------------------------*/
   mcb.ymill2 = 0;
   mcb.seill = 0;
   mcb.semll = 0;
   mcb.ymmll = 0;
   mcb.grmll= 0;
   mcb.maxsei = kcb.maxsei;
   mcb.totcand = kcb.totcand;
   mcb.outcand = 0;
   mcb.reqcand = 0;
   mcb.currfst = 0;
   mcb.currlst = 0;

/*---------------------------------------------------------------------*
 *      RETURN                                
 *---------------------------------------------------------------------*/
   return( z_rofkkc );
}
