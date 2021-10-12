static char sccsid[] = "@(#)95	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0lrnw.c, libKJI, bos411, 9428A410j 6/4/91 10:05:23";
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
 * MODULE NAME:       _Kc0lrnw
 *
 * DESCRIPTIVE NAME:  SAVE THE STUDIED DATA TO FILES
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0584(FZK_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0684(FZK_READ):   error of read()
 *                    0x0782(USR_WRITE):  error of write()
 *                    0x0784(FZK_WRITE):  error of write()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0884(FZK_LOCKF):  error of lockf()
 *                    0x0a10(UPDATING):   being updated
 *                    0x0b10(RQ_RECOVER): request of recovery
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
short   _Kc0lrnw( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB               */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kczwrit();   /* Write The Dictionaries   (TS)*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 

   short        z_rtcode;               /* set return code to caller    */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/

   kcbptr1 = z_kcbptr;                  /* initialize kcbptr1           */

/*----------------------------------------------------------------------*
 *      WRITE THE MRU BUFFER TO USER DICTIONARY
 *----------------------------------------------------------------------*/

   z_rtcode = _Kczwrit( kcbptr1, MRU, 0 , (short)EXCLUSIVE  );
   if ( z_rtcode != SUCCESS )
      return( z_rtcode );               /* write error                  */

/*----------------------------------------------------------------------*
 *      WRITE THE DFZ BUFFER TO FUZOKUGO DICTIONARY
 *----------------------------------------------------------------------*/
   z_rtcode = _Kczwrit( kcbptr1, FD, 0 , (short)EXCLUSIVE );
   if ( z_rtcode != SUCCESS )
      return( z_rtcode );               /* write error                  */
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/

   return( SUCCESS );
}
