static char sccsid[] = "@(#)30	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kckmktb.c, libKJI, bos411, 9428A410j 6/4/91 12:58:58";
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
 * MODULE NAME:       _Kckmktb
 *
 * DESCRIPTIVE NAME:  MAKE TABLE OF CANDIDATES OF KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0004 (NO_CAND): No candidate
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x7fff (UERROR) : unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kckmktb(z_kcbptr)
struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */
   extern struct RETNCUTB _Kcncutb();   /* Delete Duplicate BTE         */
   extern void            _Kck7ycd();   /* Translate yomi codes         */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rkkjtb;    /* Define Area for Return of _Kckkjtb   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */
   struct RETNCUTB z_rncutb;    /* Define Area for Return of _Kcncutb   */
   short           z_i;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      7 BIT YOMI CODE -> ALPHABETIC 7 BIT YOMI CODE
 *----------------------------------------------------------------------*/
   _Kck7ycd( z_kcbptr );

/*----------------------------------------------------------------------*  
 *      GET ENTRIES
 *----------------------------------------------------------------------*/ 
   z_rkkjtb = _Kckkjtb( z_kcbptr );

   if ( z_rkkjtb != SUCCESS)
      return( z_rkkjtb );

   if( CACTV( kcb.jthchh ) == 0)
      return(NO_CAND);

 /*---------------------------------------------------------------------*  
  *      MAKE A TABLE OF CANDIDATE ON BTE
  *---------------------------------------------------------------------*/ 
   z_rbproc = _Kcbproc(z_kcbptr,(short)kcb.ychacyce-1,(uschar)0);

   if (z_rbproc != SUCCESS)
      return(z_rbproc);

 /*---------------------------------------------------------------------*  
  *      DELETE IDENTICAL CANDIDATE ON BTE
  *---------------------------------------------------------------------*/ 
   z_rncutb = _Kcncutb(z_kcbptr,NULL);

   return( z_rncutb.rc );
}
