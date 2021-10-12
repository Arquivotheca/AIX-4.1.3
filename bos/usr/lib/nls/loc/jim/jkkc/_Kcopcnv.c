static char sccsid[] = "@(#)69	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopcnv.c, libKJI, bos411, 9428A410j 6/4/91 15:19:00";
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
 * MODULE NAME:       _Kcopcnv
 *
 * DESCRIPTIVE NAME:  PREVIOUS CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0002 (NOMORE_CAND): No more candidate
 *                    0x0004 (NO_CAND): No candidate
 *
 *                    0x0108 (WSPOVER): work space overflow
 *
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
 *
 *                    0x7fff (UERROR) :  unpredictable error
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
short _Kcopcnv(z_kcbptr)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kcobro1();
   extern short             _Kconcv2();
   extern short             _Kccinsb();
   extern struct RETNMTCH   _Kcnmtch();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcsem.h"
#include   "_Kcsei.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_EXIST       0x00ff
#define   Z_NOT_EXIST   0x01ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_rc;              /* return code save area        */
   short             z_len1;
   struct RETNMTCH   z_ret;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   semptr1 = kcb.semaddr;               /* set base pointer of SEM      */

/*----------------------------------------------------------------------*
 *       MAKE ENVIRONMENT FOR NEXT CONVERSION 
 *----------------------------------------------------------------------*/
   if ( kcb.env != ENVNEXT )
   {
      /*----------------   SET ENVIRONMENT   ---------------------------*/
      kcb.env = ENVNEXT;

      /*----------------   GET CANDIDATES   ----------------------------*/
      z_rc = _Kcobro1( z_kcbptr );
   
      if ( z_rc != SUCCESS)
         return( z_rc );

      /*----------------   SET ENVIRONMENT   ---------------------------*/
      gpwptr1 = kcb.gpwgpe;
      gpw.pkakutei = PKAKALL;

      /*--------   INSERT THE CANDIDATE FOR FIRST CONVERSION 
                      INTO THE TOP OF THE TABLE OF CANDIDATES   --------*/
      seiptr1 = kcb.seiaddr;
      z_len1 = sei.ll[0] * 256 + sei.ll[1] - 2;

      z_ret = _Kcnmtch(z_kcbptr, &sei.kj[0][0], z_len1);

      if ( z_ret.rc != Z_EXIST )
      {
         if ( z_ret.rc == Z_NOT_EXIST)
             return( UERROR );
         else
             return( z_ret.rc );
      }

      z_rc = _Kccinsb(&kcb.bthchh,z_ret.bteptr,NULL);

      if ( z_rc != SUCCESS )
      {
         return( z_rc );
      }
   }

/*----------------------------------------------------------------------*
 *       SELECT NEXT CANDIDATE
 *----------------------------------------------------------------------*/
   z_rc = _Kconcv2(z_kcbptr,(short)PREVCNV);

   return( z_rc );
}
