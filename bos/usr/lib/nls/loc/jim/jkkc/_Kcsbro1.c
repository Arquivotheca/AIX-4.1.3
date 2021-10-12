static char sccsid[] = "@(#)88	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcsbro1.c, libKJI, bos411, 9428A410j 7/23/92 03:16:55";
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
 * MODULE NAME:       _Kcsbro1
 *
 * DESCRIPTIVE NAME:  MAKE A SINGLE KANJI ENVIRONMENT
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x0004(NO_CAND): no candidate
 *                    0x1104(JTEOVER): JTE table overflow
 *                    0x1204(JKJOVER): JKJ table overflow
 *                    0x1704(BTEOVER): BTE overflow
 *                    0x7FFF(UERROR ): Fatal error
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
short  _Kcsbro1(z_kcbptr,z_file)

struct  KCB  *z_kcbptr;
short         z_file;                   /* System File Number           */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcimora();   /* Set 7bit Yomi & Mora Code    */
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */
   extern short           _Kcjsdct();   /* Looking up on System Dict.   */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_NEW        4               /*                              */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rimora;    /* Define Area for Return of _Kcimora   */
   short           z_rjsdct;    /* Define Area for Return of _Kcjsdct   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */
   short           z_i;         /* Loop counter                         */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */

/*----------------------------------------------------------------------*  
 *      SET MORA ON YCE
 *----------------------------------------------------------------------*/ 
   z_rimora = _Kcimora(z_kcbptr,(short)Z_NEW);
                                        /* set mora code on mce         */
   mceptr2 = kcb.mchmce + kcb.mchacmce;
   mceptr1 = mceptr2 - 1;
   for(z_i = 0;z_i < kcb.mchacmce; z_i++,mceptr1--,mceptr2--)
   {
       mce2.code    = mce.code;
       mce2.yceaddr = mce.yceaddr;
       mce2.jdok    = NULL;
       mce2.maxln   = NULL;
   }
   mce2.code    = 0xFA;                 /* set mora code of tankan shift*/
   mce2.yceaddr = NULL;
   mce2.jdok    = 1;
   kcb.mchacmce ++;

/*----------------------------------------------------------------------*  
 *      LOOK UP SINGLE KANJI ENTRY
 *----------------------------------------------------------------------*/ 
   z_rjsdct = _Kcjsdct(z_kcbptr,(short)0,kcb.mchacmce,(short)SPECIFIC,z_file);
   if ( z_rjsdct != SUCCESS )
      return(z_rjsdct);

/*----------------------------------------------------------------------*  
 *      CREATE BTE
 *----------------------------------------------------------------------*/ 
   z_rbproc=_Kcbproc(z_kcbptr,(short)(kcb.mchacmce-1),(uschar)0);
   if ( z_rbproc != SUCCESS )
      return(z_rbproc);

   kcb.totcand = CACTV( kcb.bthchh );
   if ( kcb.totcand  == 0)
   {
      kcb.outcand = 0;
      kcb.currfst = 0;
      kcb.currlst = 0;
      return( NO_CAND );
   }
   kcb.maxsei = 2;

/*----------------------------------------------------------------------*  
 *      END AND RETURN                                                  *
 *----------------------------------------------------------------------*/ 
   return( SUCCESS );

}
