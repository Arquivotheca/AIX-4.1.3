static char sccsid[] = "@(#)96	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcabcnv.c, libKJI, bos411, 9428A410j 6/4/91 10:05:41";
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
 * MODULE NAME:       _Kcabcnv
 *
 * DESCRIPTIVE NAME:  TOP OF RYAKUSHO HENKAN
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kcabcnv(z_kcbptr)

struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern void            _Kcrinit();   /* Clear Output Buffers         */
   extern short           _Kcjudct();   /* Looking up on User Dict.     */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */
#include   "_Kcymm.h"   /* YoMi Map entry (YMM)                         */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rjudct;    /* Define Area for Return of _Kcjudct   */
   short           z_i;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                   /* establish address to kcb    */
   ymiptr1 = kcb.ymiaddr;                /* set address of YMI          */

/*----------------------------------------------------------------------*  
 *      INITIALIZE INTERNAL WORK AREA
 *----------------------------------------------------------------------*/ 
   _Kcxinia(z_kcbptr);

/*----------------------------------------------------------------------*  
 *      INITIALIZE OUTPUT BUFFER
 *----------------------------------------------------------------------*/ 
   _Kcrinit(z_kcbptr);

/*----------------------------------------------------------------------*  
 *      SET INPUT YOMI in YCE
 *----------------------------------------------------------------------*/ 
   for(z_i = 0,yceptr1 = kcb.ychyce;z_i < kcb.ymill1;z_i ++,yceptr1 ++)
      yce.yomi = ymi.yomi[z_i];
   kcb.ychacyce = z_i ;

/*----------------------------------------------------------------------*  
 *      SET 0 at HEAD of MORA
 *----------------------------------------------------------------------*/ 
   mceptr1 = kcb.mchmce;
   mce.code = NULL;
   mce.yceaddr = --yceptr1;
   mce.jdok = JD_READY;
   kcb.mchacmce = 1;

/*----------------------------------------------------------------------*  
 *      LOOK UP THE RYAKU SHO ON USER DICTIONARY
 *----------------------------------------------------------------------*/ 
   z_rjudct = _Kcjudct(z_kcbptr,(uschar)0,(uschar)1);
   if ( z_rjudct != SUCCESS )
      return(z_rjudct);

   if ( mce.jdok == JD_READY )
      return( NO_CAND );

/*----------------------------------------------------------------------*  
 *      SET LENGTH
 *----------------------------------------------------------------------*/ 
   seiptr1 = kcb.seiaddr;               /* set seisho address           */
   semptr1 = kcb.semaddr;               /* set seisho map address       */
   grmptr1 = kcb.grmaddr;               /* set gremer map address       */
   ymmptr1 = kcb.ymmaddr;               /* set yomi   map address       */
   kcb.seill = CHTODBCS(sei.ll);        /* set seisho length            */
   sem.code[0] = RYAKUSHO;              /* set seisho attribute code    */
   SHTOCHPT(sem.ll,(short)3);           /* set seisho map length        */
   kcb.semll = 3;                       /* set seisho map length        */
   grm.byte[0] = 0x00;                  /* set gremer map               */
   kcb.grmll = 2;                       /* set gremer map length        */
   grm.ll    = kcb.grmll;               /* set gremer map length        */
   ymm.byte[0] = 0x80;                  /* set yomi map                 */
   kcb.ymmll = ( kcb.ymill1 / 8 ) + 2 ; /* set yomi map length          */
   ymm.ll  = kcb.ymmll;                 /* set yomi map length          */

/*----------------------------------------------------------------------*  
 *      END AND RETURN
 *----------------------------------------------------------------------*/ 

   return( SUCCESS );
 }
