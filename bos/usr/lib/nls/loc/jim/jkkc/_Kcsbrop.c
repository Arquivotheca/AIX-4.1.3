static char sccsid[] = "@(#)89	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcsbrop.c, libKJI, bos411, 9428A410j 7/23/92 03:16:59";
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
 * MODULE NAME:       _Kcsbrop
 *
 * DESCRIPTIVE NAME:  Single Kanji Open
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS) : Success
 *                    0x0002 (END_CAND): End of candidate
 *                    0x0004 (NO_CAND) : No candidate
 *                    0x1104 (JTEOVER) : JTE table overflow
 *                    0x1204 (JKJOVER) : JKJ table overflow
 *                    0x1704 (BTEOVER) : BTE overflow
 *                    0x7fff (UERROR)  : Unpredictable error
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
short  _Kcsbrop(z_kcbptr)

struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern short           _Kcsbro1();   /* Make Env. for Single Kanji   */
   extern short           _Kcobro2();   /* Set Candidates on Seisho Buf.*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcmcb.h"   /* Monitor control block (MCB)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rsbro1;    /* Define Area for Return of _Kcsbro1   */
   short           z_file;      /* System File Number                   */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;
   mcbptr1 = ( struct MCB *)kcb.myarea; /* Get MCB pointer              */

   kcb.env = ENVTAN;                    /* set environment flag         */
   gpwptr1 = kcb.gpwgpe;
   gpw.pkakutei = PKAKALL;              /* set confirm flag for next    */

   _Kcxinia(kcbptr1);                   /* init. all ctl blocks         */

   for( z_file = 0; z_file < mcb.mul_file; z_file++ ) {
      if( mcb.mulsys[z_file].single_kanji ) {	/* Single-Kanji Exist   */
	  z_rsbro1 = _Kcsbro1(z_kcbptr,z_file);
          if( z_rsbro1 != SUCCESS )
             return( z_rsbro1 );
      }
   }

/*----------------------------------------------------------------------*  
 *      END AND RETURN
 *----------------------------------------------------------------------*/ 
   kcb.posend = 0;                      /* zero on end point            */

   return( _Kcobro2( z_kcbptr,(short)ALLFW ) );

}
