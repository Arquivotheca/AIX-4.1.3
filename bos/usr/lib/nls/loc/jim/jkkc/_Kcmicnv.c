static char sccsid[] = "@(#)49	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmicnv.c, libKJI, bos411, 9428A410j 6/4/91 15:14:34";
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
 * MODULE NAME:       _Kcmicnv
 *
 * DESCRIPTIVE NAME:  ALPHA/ALPHANUM AND KANSUJI INITIAL CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS) : success
 *                    0x0004(NO_CAND) : success
 *                    0x0108 (WSPOVER): internal workspace overflow
 *                    0x1104 (JTEOVER): JTE overflow
 *                    0x1204 (JKJOVER): JKJ overflow
 *                    0x1304 (JLEOVER): JLE overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x2104 (SEIOVER): seisho buffer overflow
 *                    0x2204 (SEMOVER): seisho map buffer overflow
 *                    0x2304 (YMMOVER): yomi map buffer overflow
 *                    0x2404 (GRMOVER): grammar map buffer overflow
 *                    0x7fff (UERROR):  unpredictable error
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
short  _Kcmicnv( z_kcbptr )
struct  KCB  *z_kcbptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern void            _Kcrinit();   /* Clear Output Buffers         */
   extern short           _Kcmmktb();   /* Make candidates table        */
   extern short           _Kcrsout();   /* Set BTE data on Output Buffer*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */
#include   "_Kcymm.h"   /* YoMi Map entry (YMM)                         */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rc;                /* Define Area for Return Code  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      SET ENVIRONMENT
 *----------------------------------------------------------------------*/
   kcb.env = ENVALKN;

/*----------------------------------------------------------------------*  
 *      INITIALIZE
 *----------------------------------------------------------------------*/ 
   _Kcxinia(z_kcbptr);                  /* initialize internal area     */

   _Kcrinit(z_kcbptr);                  /* initialize output buffer     */

/*----------------------------------------------------------------------*  
 *      CREATE CANDIDATES OF ALPHABETIC/ALPHANUMERIC CONVERSION
 *----------------------------------------------------------------------*/ 
   z_rc = _Kcmmktb( z_kcbptr );

   if ( z_rc != SUCCESS )
      return(z_rc);


/*----------------------------------------------------------------------*  
 *      GET THE TOP CANDIDATE OF ALPHANUMERIC CONVERSION
 *----------------------------------------------------------------------*/ 
   CMOVT( kcb.bthchh,bteptr1 );
   if ( bteptr1 == NULL )
      return( NO_CAND );

/*----------------------------------------------------------------------*  
 *      SET SEISHO AND OTHERS POINTER
 *----------------------------------------------------------------------*/ 
   seiptr1 = kcb.seiaddr;
   semptr1 = kcb.semaddr;
   grmptr1 = kcb.grmaddr;
   ymmptr1 = kcb.ymmaddr;

/*----------------------------------------------------------------------*  
 *      SET SEISHO ON OUTPUT BUFFER
 *----------------------------------------------------------------------*/ 
   z_rc = _Kcrsout(z_kcbptr,bteptr1,seiptr1,semptr1,grmptr1,ymmptr1);

   if (z_rc != SUCCESS)
      return(z_rc);

/*----------------------------------------------------------------------*  
 *      SET LENGTH
 *----------------------------------------------------------------------*/ 
   kcb.seill = CHPTTOSH( sei.ll );
   kcb.semll = CHPTTOSH( sem.ll );
   kcb.grmll = grm.ll;
   kcb.ymmll = ymm.ll;

   return( SUCCESS );
}
