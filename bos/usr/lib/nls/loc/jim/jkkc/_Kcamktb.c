static char sccsid[] = "@(#)05	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcamktb.c, libKJI, bos411, 9428A410j 6/4/91 10:08:46";
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
 * MODULE NAME:       _Kcamktb
 *
 * DESCRIPTIVE NAME:  MAKE TABLE OF CANDIDATES OF ALPHANUMERIC CONVERSION
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0004 (NO_CAND): No candidate
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x0108 (WSPOVER): work space overflow
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
short  _Kcamktb(z_kcbptr)
struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcjmdct();   /* Looking up MRU Entry         */
   extern short           _Kcjudct();   /* Looking up on User Dict.     */
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */
   extern struct RETNCUTB _Kcncutb();   /* Delete Duplicate BTE         */

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
   short           z_rjmdct;    /* Define Area for Return of _Kcjmdct   */
   short           z_rjudct;    /* Define Area for Return of _Kcjudct   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */
   struct RETNCUTB z_rncutb;    /* Define Area for Return of _Kcncutb   */
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
   yceptr1++;                           /* set yomi on YCE              */
   for(z_i = 0;z_i < kcb.ymill1;z_i ++,yceptr1 ++)
      yce.yomi = (*kcb.ymiaddr).yomi[z_i];
   kcb.ychacyce = z_i + 1;

 /*---------------------------------------------------------------------*  
  *      SET  0 AT HEAD OF MORA
  *---------------------------------------------------------------------*/ 
   mceptr1 = kcb.mchmce;
   mce.code = NULL;
   mce.yceaddr = --yceptr1;
   mce.jdok = JD_READY;
   kcb.mchacmce = 1;

 /*---------------------------------------------------------------------*  
  *      GET ENTRY IN USER DICTIONARY
  *---------------------------------------------------------------------*/ 
   z_rjudct = _Kcjudct(z_kcbptr,(uschar)0,(uschar)1);
   if ( z_rjudct != SUCCESS )
      return(z_rjudct);

   if( CACTV( kcb.jthchh ) == 0)
      return(NO_CAND);

 /*---------------------------------------------------------------------*
  *     GET ENTRY IN MRU
  *---------------------------------------------------------------------*/
   z_rjmdct = _Kcjmdct(z_kcbptr,(uschar)0,(uschar)1);
   if ( z_rjmdct != SUCCESS )
      return(z_rjmdct);

 /*---------------------------------------------------------------------*  
  *      MAKE A TABLE OF CANDIDATE ON BTE
  *---------------------------------------------------------------------*/ 
   z_rbproc = _Kcbproc(z_kcbptr,(short)kcb.mchacmce-1,(uschar)0);
   if ( z_rbproc != SUCCESS )
      return(z_rbproc);

 /*---------------------------------------------------------------------*  
  *      DELETE IDENTICAL CANDIDATE ON BTE
  *---------------------------------------------------------------------*/ 
   z_rncutb = _Kcncutb(z_kcbptr,NULL);
   if ( z_rncutb.rc != SUCCESS )
      return(z_rncutb.rc);

 /*---------------------------------------------------------------------*  
  *      RETURN
  *---------------------------------------------------------------------*/ 
   return( SUCCESS );
}
