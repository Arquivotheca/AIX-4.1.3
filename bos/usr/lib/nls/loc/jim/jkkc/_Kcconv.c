static char sccsid[] = "@(#)42	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcconv.c, libKJI, bos411, 9428A410j 6/4/91 10:21:18";
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
 * MODULE NAME:       _Kcconv
 *
 * DESCRIPTIVE NAME:  OUTPUT THE BEST CANDIDET
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0006(DURING_CNV): during Chikuji Conversion
 *                    0x0004(NO_CAND):    no candidate exist
 *                    0x0104(NUM_BTE):    number of BTE is more than 6
 *                    0x1104(JTEOVER):    JTE overflow
 *                    0x1204(JKJOVER):    JKJ overflow
 *                    0x1304(JLEOVER):    JLE overflow
 *                    0x1404(FTEOVER):    FTE overflow
 *                    0x1504(FWEOVER):    FWE overflow
 *                    0x1604(FKXOVER):    FKX overflow
 *                    0x1704(BTEOVER):    BTE overflow
 *                    0x1804(PTEOVER):    PTE overflow
 *                    0x1904(YCEOVER):    YCE overflow
 *                    0x1a04(YPEOVER):    YPE overflow
 *                    0x1b04(MCEOVER):    MCE overflow
 *                    0x2104(SEIOVER):    SEI overflow
 *                    0x2204(SEMOVER):    SEM overflow
 *                    0x2304(YMMOVER):    YMM overflow
 *                    0x2404(GRMOVER):    GRM overflow
 *                    0x0108(WSPOVER):    work space overflow
 *                    0x0581(SYS_LSEEK):  error of lseek() with system dict.
 *                    0x0681(SYS_READ):   error of read() with system dict.
 *                    0x0881(SYS_LOCKF):  error of lockf() with system dict.
 *                    0x0582(USR_LSEEK):  error of lseek() with user dict.
 *                    0x0682(USR_READ):   error of read() with user dict.
 *                    0x0882(USR_LOCKF):  error of lockf() with user dict.
 *                    0x0584(FZK_LSEEK):  error of lseek() with adjunct dict.
 *                    0x0684(FZK_READ):   error of read() with adjunct dict.
 *                    0x0884(FZK_LOCKF):  error of lockf() with adjunct dict.
 *                    0x7fff(UERROR):     unpredictable error
 *                
 * CATEGORY:          Unique, Entry
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
short _Kcconv( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE                                             
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
   short        z_rofkkc;               /* return code from _Kcofkkc()  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS                                       
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* set base pointer of MCB      */
   kcbptr1 = mcb.kcbaddr;               /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *      SET INPUT PARAMETERS IN KCB                                      
 *----------------------------------------------------------------------*/
   kcb.mode     = mcb.mode;             /* copy conversion mode         */
   kcb.func     = FUNCONV;              /* set function No. as first    */

   kcb.cnvx     = mcb.cnvx;             /* explicit req for kakutei     */
   kcb.cnvi     = mcb.cnvi;             /* req for internal kakutei     */

   kcb.charl    = mcb.charl;            /* left to the current yomi     */
   kcb.charr    = mcb.charr;            /* right to the current yomi    */

   kcb.ymill1   = mcb.ymill1;           /* length of input yomi         */
   kcb.ymill2   = mcb.ymill2;           /* offset to the J/F boundary   */

   if ( mcb.mode == MODALKAN )
      kcb.alpkan = mcb.alpkan;

/*----------------------------------------------------------------------*
 *      FIRST CONVERSION OR RECONVERSION                             
 *----------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );

   switch( z_rofkkc )
   {
      case SUCCESS:
      case DURING_CNV:
      case NO_CAND:
      case NUM_BTE:

      case JTEOVER:
      case JKJOVER:
      case JLEOVER:
      case FTEOVER:
      case FWEOVER:
      case FKXOVER:
      case BTEOVER:
      case PTEOVER:
      case YCEOVER:
      case YPEOVER:
      case MCEOVER:

      case SEIOVER:
      case SEMOVER:
      case YMMOVER:
      case GRMOVER:

      case WSPOVER:

      case SYS_LSEEK:
      case SYS_READ:
      case SYS_LOCKF:

      case USR_LSEEK:
      case USR_READ:
      case USR_LOCKF:

      case FZK_LSEEK:
      case FZK_READ:
      case FZK_LOCKF:
                        break;

      default:          return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      SET OUTPUT PARAMETERS IN MCB                            
 *----------------------------------------------------------------------*/
   mcb.ymill2   = kcb.ymill2;           /* length of coverted yomi      */
   mcb.seill    = kcb.seill;            /* current seisho length        */
   mcb.semll    = kcb.semll;            /* current seisho map length    */
   mcb.ymmll    = kcb.ymmll;            /* current yomi map length      */
   mcb.grmll    = kcb.grmll;            /* current grammer map length   */

/*----------------------------------------------------------------------*
 *      RETURN                                       
 *----------------------------------------------------------------------*/
   return( z_rofkkc );
}
