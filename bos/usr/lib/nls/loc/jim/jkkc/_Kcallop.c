static char sccsid[] = "@(#)03	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcallop.c, libKJI, bos411, 9428A410j 6/4/91 10:07:57";
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
 * MODULE NAME:       _Kcallop
 *
 * DESCRIPTIVE NAME:  OUTPUT THE CANDIDATE TO THE OUTPUT AREA
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0002(END_CAND):   end of All Candidtes Conversion
 *                    0x0004(NO_CAND):    no candidate exist
 *                    0x1104(JTEOVER):    JTE overflow
 *                    0x1204(JKJOVER):    JKJ overflow
 *                    0x1304(JLEOVER):    JLE overflow
 *                    0x1404(FTEOVER):    FTE overflow
 *                    0x1504(FWEOVER):    FWE overflow
 *                    0x1604(FKXOVER):    FKX overflow
 *                    0x1704(BTEOVER):    BTE overflow
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
short _Kcallop( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                              
 *----------------------------------------------------------------------*/
   extern short _Kcofkkc();             /* main of kkc                  */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rofkkc;               /* set return code              */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1          */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1          */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB
 *----------------------------------------------------------------------*/
   kcb.mode     = mcb.mode;             /* copy conversion mode        */
   kcb.func     = FUNALLOP;             /* all candidates open         */

   kcb.ymill1   = mcb.ymill1;           /* length of input yomi        */
   kcb.seill    = mcb.seill;            /* current seisho length       */
   kcb.semll    = mcb.semll;            /* current seisho map length   */
   kcb.grmll    = mcb.grmll;            /* current grammer map length  */

   kcb.reqcand  = mcb.reqcand;          /* No. of candidates to caller */

   if ( mcb.mode == MODALKAN )
      kcb.alpkan = mcb.alpkan;

/*----------------------------------------------------------------------* 
 *      MAKE THE TAVLE OF CANDIDATES FOR ALL CONVERSION
 *----------------------------------------------------------------------*/
   z_rofkkc = _Kcofkkc( kcbptr1 );      /* call KKCOFKKC()              */

   switch( z_rofkkc )
   {
      case SUCCESS:
      case END_CAND:
      case NO_CAND:

      case JTEOVER:
      case JKJOVER:
      case JLEOVER:
      case FTEOVER:
      case FWEOVER:
      case FKXOVER:
      case BTEOVER:
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
   mcb.ymill2   = kcb.ymill2;           /* offset to the J/F boundary  */
   mcb.seill    = kcb.seill;            /* current seisho length       */
   mcb.semll    = kcb.semll;            /* current seisho map length   */
   mcb.ymmll    = kcb.ymmll;            /* current yomi map length     */
   mcb.grmll    = kcb.grmll;            /* current grammer map length  */

   mcb.maxsei   = kcb.maxsei;           /* max length of seisho        */
   mcb.totcand  = kcb.totcand;          /* number of all candidates    */
   mcb.currfst  = kcb.currfst;          /* current fast cand number    */
   mcb.currlst  = kcb.currlst;          /* current last cand number    */
   mcb.outcand  = kcb.outcand;          /* number of candidates outbuf */

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rofkkc );
}
