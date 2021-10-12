static char sccsid[] = "@(#)52	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcdctlw.c, libKJI, bos411, 9428A410j 6/4/91 10:24:48";
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
 * MODULE NAME:       _Kcdctlw
 *
 * DESCRIPTIVE NAME:  LEARNING OF JIRITU-GO AND FUZOKU-GO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0a10(UPDATING):   being updated
 *                    0x0b10(RQ_RECOVER): request of recovery
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0782(USR_WRITE):  error of write()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0584(FZK_LSEEK):  error of lseek()
 *                    0x0684(FZK_READ):   error of read()
 *                    0x0784(FZK_WRITE):  error of write()
 *                    0x0884(FZK_LOCKF):  error of lockf()
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
short _Kcdctlw( z_mcbptr )
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
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rtofkkc;               /* set return code             */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* initialize mcbptr1          */
   kcbptr1 = mcb.kcbaddr;               /* initialize kcbptr1          */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB
 *----------------------------------------------------------------------*/
   kcb.func     = FUNLRNWR;             /* write the MRU data          */

/*----------------------------------------------------------------------* 
 *      WRITE LEARNED BUFFER TO USER DICT & FUZOKU-GO DICT
 *----------------------------------------------------------------------*/
   z_rtofkkc = _Kcofkkc( kcbptr1 );      /* call KKCOFKKC()             */

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   switch ( z_rtofkkc )
   {
      case SUCCESS:
      case UPDATING: 
      case RQ_RECOVER: 
      case USR_LSEEK:
      case USR_READ:
      case USR_WRITE:
      case USR_LOCKF:
      case FZK_LSEEK:
      case FZK_READ:
      case FZK_WRITE:
      case FZK_LOCKF:
                      return( z_rtofkkc );    /* return to caller            */
      default:        return( UERROR );
   }
}
