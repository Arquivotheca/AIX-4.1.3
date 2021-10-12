static char sccsid[] = "@(#)67	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kconcnv.c, libKJI, bos411, 9428A410j 6/4/91 15:18:38";
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
 * MODULE NAME:       _Kconcnv
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0002 (NOMORE_CAND): No more candidate
 *                    0x0004 (NO_CAND): No candidate
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
 *                    0x0108 (WSPOVER): work space overflow
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
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kconcnv(z_kcbptr)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcobro1();   /* Making All Cand. Environment */
   extern short           _Kconcv2();   /* Set Candidate for Next Conv. */
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_EXIST     (0x0000 | LOCAL_RC) /* exist in candidates          */
#define Z_NOT_EXIST (0x0100 | LOCAL_RC) /* not exist in candidates      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_robro1;    /* Define Area for Return of _Kcobro1   */
   short           z_roncv2;    /* Define Area for Return of _Kconcv2   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */
   short           z_len1;      /* length save area                     */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   semptr1 = kcb.semaddr;               /* set base pointer of SEM      */

/*----------------------------------------------------------------------*
 *      MAKE ENVIRONMENT FOR NEXT CONVERSION
 *----------------------------------------------------------------------*/
   if ( kcb.env != ENVNEXT )
   {
      kcb.env = ENVNEXT;

      z_robro1 = _Kcobro1( z_kcbptr );
   
      gpwptr1 = kcb.gpwgpe;
      gpw.pkakutei = PKAKALL;

      if ( z_robro1 != SUCCESS )
         return( z_robro1 );

      seiptr1 = kcb.seiaddr;

      z_len1 = CHPTTOSH( sei.ll ) - 2;

      z_rnmtch = _Kcnmtch(z_kcbptr, sei.kj, z_len1);
      if( z_rnmtch.rc == Z_EXIST )
      {
         z_rcinsb = _Kccinsb(&kcb.bthchh,z_rnmtch.bteptr, NULL);
         if(z_rcinsb != SUCCESS )
            return( z_rcinsb );
      }
      else
      {
         if( z_rnmtch.rc != Z_NOT_EXIST )
            return( z_rnmtch.rc );
      }
   }

/*----------------------------------------------------------------------*
 *       SELECT NEXT CANDIDATE
 *----------------------------------------------------------------------*/
   z_roncv2 = _Kconcv2(z_kcbptr,(short)NEXTCNV);
   if( z_roncv2 != SUCCESS )
      return( z_roncv2 );

/*----------------------------------------------------------------------*
 *       RETURN NORMAL
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
