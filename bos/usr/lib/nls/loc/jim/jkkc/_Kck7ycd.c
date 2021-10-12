static char sccsid[] = "@(#)20	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kck7ycd.c, libKJI, bos411, 9428A410j 6/4/91 12:57:27";
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
 * MODULE NAME:       _Kck7ycd
 *
 * DESCRIPTIVE NAME:  MAKE TABLE OF CANDIDATES OF ALPHANUMERIC CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       void
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/*----------------------------------------------------------------------*
 *      TRANSLATION TABLE
 *----------------------------------------------------------------------*/
static   uschar   y7bittb[] = { 
                                 0x2C,  /* CONMA                        */
                                 0x2E,  /* PIRIOD                       */
                                 0x00,  /* INVALID                      */
                                 0x30,  /* 0                            */
                                 0x31,  /* 1                            */
                                 0x32,  /* 2                            */
                                 0x33,  /* 3                            */
                                 0x34,  /* 4                            */
                                 0x35,  /* 5                            */
                                 0x36,  /* 6                            */
                                 0x37,  /* 7                            */
                                 0x38,  /* 8                            */
                                 0x39   /* 9                            */
                               };


/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kck7ycd( z_kcbptr )
struct  KCB  *z_kcbptr;
{
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
   short           z_i;


/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

 /*---------------------------------------------------------------------*  
  *      SET ESCAPE FOR ALPHANUMERIC
  *---------------------------------------------------------------------*/ 
   yceptr1 = kcb.ychyce;                /* set YCE address              */
   yce.yomi = ESC_ALPHA;                /* set Alphanumeric ESC on YCE  */
   mceptr1 = kcb.mchmce;
   mce.code = NULL;
   mce.yceaddr = yceptr1;

   yceptr1++;                           /* set yomi on YCE              */
   mceptr1++;                           /* set yomi on YCE              */

   for( z_i = 0; z_i < kcb.ymill1; z_i ++, yceptr1 ++, mceptr1++ )
   {
      yce.yomi = y7bittb[ (*kcb.ymiaddr).yomi[z_i] - Y_CNM ];
      mce.code = NULL;
      mce.yceaddr = yceptr1;
   }
   kcb.ychacyce = z_i + 1;

 /*---------------------------------------------------------------------*  
  *      SET  0 AT HEAD OF MORA
  *---------------------------------------------------------------------*/ 
   mceptr1 = kcb.mchmce;
   mce.code = NULL;
   mce.yceaddr = --yceptr1;
   mce.jdok = JD_READY;
   kcb.mchacmce = 1;
}
