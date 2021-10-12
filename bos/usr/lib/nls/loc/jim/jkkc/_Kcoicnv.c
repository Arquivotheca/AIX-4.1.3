static char sccsid[] = "@(#)64	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcoicnv.c, libKJI, bos411, 9428A410j 6/4/91 15:18:08";
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
 * MODULE NAME:       _Kcoicnv
 *
 * DESCRIPTIVE NAME:  PASS THROUGH THIS FUNCTION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):    success
 *                    0x0004 (NO_CAND):    no candidate exist
 *                    0x0006 (DURING_CNV): no candidate exist
 *                    0x1104 (JTEOVER):    JTE table overflow
 *                    0x1204 (JKJOVER):    JKJ overflow
 *                    0x1304 (JLEOVER):    JLE overflow
 *                    0x1404 (FTEOVER):    FTE table overflow
 *                    0x1704 (BTEOVER):    BTE table overflow
 *                    0x1804 (PTEOVER):    PTE table overflow
 *                    0x2104 (SEIOVER):    seisho buffer overflow
 *                    0x2204 (SEMOVER):    seisho map buffer overflow
 *                    0x2304 (YMMOVER):    ymi map buffer overflow
 *                    0x2404 (YMMOVER):    grammar map buffer overflow
 *                    0x7fff (UERROR) :     unpredictable error
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
short _Kcoicnv(z_kcbptr)
struct KCB      *z_kcbptr;              /* get address of KCB           */
  
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcoicv1();   /* Initial Conversion           */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_roicv1;    /* Define Area for Return of _Kcoicv1   */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;
   gpwptr1 = kcb.gpwgpe;

   z_roicv1 = _Kcoicv1(z_kcbptr);

   gpw.pmode = kcb.mode;                /* save current mode            */
   if(( z_roicv1 != SUCCESS )&&
      ( z_roicv1 != DURING_CNV ))
      gpw.pkakutei = PKAKALL;        /* kakutei ON for next time     */
   return(z_roicv1);
}
