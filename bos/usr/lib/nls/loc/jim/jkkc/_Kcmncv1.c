static char sccsid[] = "@(#)52	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmncv1.c, libKJI, bos411, 9428A410j 6/4/91 15:15:12";
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
 * MODULE NAME:       _Kcmncv1
 *
 * DESCRIPTIVE NAME:  MAKE AN ENVIRONMENT
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): Success
 *                    0x0004 (NO_CAND): No candidate
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x0108 (WSPOVER): work space overflow
 *                    0x7FFF (UERROR ): Fatal error
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
short   _Kcmncv1( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB              */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern short           _Kcmmktb();   /* Making Cand. Table for A/N   */
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_EXIST       0x00ff
#define   Z_NOT_EXIST   0x01ff

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rmmktb;    /* Define Area for Return of _Kcamktb   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */

   short           z_countr;    /* loop counter                         */
   short           z_cmpflg;    /* compare flag                         */
   short           z_seilen;    /* seisho length                        */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */
   yceptr1 = kcb.ychyce;                /* initialize of yceptr1        */
   ymiptr1 = kcb.ymiaddr;               /* initialize of ymiptr1        */

/*----------------------------------------------------------------------*  
 *      CHECK THAT CURRENT YOMI CODE IS SAME TO PREVIOUS YOMI CODE
 *----------------------------------------------------------------------*/ 
   if(kcb.ychacyce == kcb.ymill1+1)
   {
      z_cmpflg = ON;                    /* set compare flag ON          */
      for( z_countr = 0 ; z_countr < kcb.ymill1 ; z_countr++ )
      {
         yceptr1++;                     /* point next yce struct brock  */
         if( yceptr1->yomi != ymi.yomi[ z_countr ] )
         {
            z_cmpflg = OFF;             /* current YOMI isn't same prev */
         }
      }
   }
   else
   {
      z_cmpflg = OFF;                   /* current YOMI isn't same prev */
   }
/*----------------------------------------------------------------------*  
 *      CLEAR THE INTERNAL WORK AREA
 *----------------------------------------------------------------------*/ 
   if(( kcb.env != ENVALKN ) || ( z_cmpflg != ON ))
   {
      kcb.env = ENVALKN;
      _Kcxinia( kcbptr1 );

/*----------------------------------------------------------------------*  
 *      MAKE A BUNSETU TABLE
 *----------------------------------------------------------------------*/ 
      z_rmmktb = _Kcmmktb( kcbptr1 );
      if (z_rmmktb != SUCCESS )
        return(z_rmmktb);               /* _Kcamktb() error             */

      seiptr1 = kcb.seiaddr;
      z_seilen = CHPTTOSH( sei.ll ) - 2;
      seiptr1 = (struct SEI *)(( uschar *)seiptr1 + 2);
      z_rnmtch = _Kcnmtch( kcbptr1, seiptr1 , z_seilen );
      if( z_rnmtch.rc == Z_EXIST )      /* if specified Seisho exists   */
      {
         z_rcinsb = _Kccinsb( &kcb.bthchh, z_rnmtch.bteptr, NULL );
                                        /* then set it on top of table  */
         if ( z_rcinsb != SUCCESS )
            return( z_rcinsb );
      }
      else
      {
         if( z_rnmtch.rc != Z_NOT_EXIST)/* if specified Seisho exists   */
            return( z_rnmtch.rc );      /* _Kcnmtch() error. RC4,UERROR */
      }
   }

/*----------------------------------------------------------------------*  
 *      CHECK THE NUMBER OF ACTIVE BTE
 *----------------------------------------------------------------------*/ 
   if( CACTV( kcb.bthchh ) == 0 )
   {
      kcb.env = NULL;                   /* reset environment            */
      return( NO_CAND );                /* return with NO-CAND      4   */
   }

   kcb.totcand = CACTV( kcb.bthchh );   /* set total candidate          */

/*----------------------------------------------------------------------*  
 *      RETURN
 *----------------------------------------------------------------------*/ 
   return( SUCCESS );
}
