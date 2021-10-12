static char sccsid[] = "@(#)41	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcclose.c, libKJI, bos411, 9428A410j 7/23/92 02:58:21";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcclose
 *
 * DESCRIPTIVE NAME:  DEALLOCATION AND CLOSING DICTIONARY FILES
 *
 * INPUT:             
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x0281(SYS_CLOSE): error of close() with system dict
 *                    0x0282(USR_CLOSE): error of close() with user dict
 *                    0x0284(FZK_CLOSE): error of close() with fuzoku-go dict
 *                    0x7fff(UERROR):    unpredictable error
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
short _Kcclose( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE          
 *----------------------------------------------------------------------*/
#ifdef _AIX
   extern short          _Kccldc0();    /* Close Dictionaries      (TS) */
#endif

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcmcb.h"   /* Monitor Control Block (MCB)                  */
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*
 *      DEFINE LOCAL VARIABLES      
 *----------------------------------------------------------------------*/
#ifdef _AIX
   short        z_rcldc0;               /* return code of _Kccldc0()    */
#endif

/*----------------------------------------------------------------------*
 *      CLOSE DICTIONARY FILES AND FREE MEMORIES
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;

#ifdef _AIX
   z_rcldc0 = _Kccldc0( z_mcbptr, FZK_CLOSE  );

   switch ( z_rcldc0 )
   {
      case SUCCESS:   
      case SYS_CLOSE:
      case USR_CLOSE:
      case FZK_CLOSE:
         break;
      default:
         z_rcldc0 = UERROR;
         break;
   }

#endif
   free( (*mcb.kcbaddr).wsp );          /* free WSP area                */
   free( mcb.kcbaddr );                 /* free KCB area                */
   free( mcb.mcbaddr );                 /* free MCB area                */

#ifdef _AIX
   return( z_rcldc0 );
#else
#   if defined(_AGC) || defined(_GSW)
      return( SUCCESS );
#   endif
#endif
}
