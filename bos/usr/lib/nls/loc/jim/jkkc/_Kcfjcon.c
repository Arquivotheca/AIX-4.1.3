static char sccsid[] = "@(#)64	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfjcon.c, libKJI, bos411, 9428A410j 6/4/91 12:48:16";
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
 * MODULE NAME:       _Kcfjcon
 *
 * DESCRIPTIVE NAME:  CHECK THE CONNECTIVITY JIRITSUGO AND FUZUKOGO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       Possibility (YES 1 / NO 0)
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
short _Kcfjcon(z_kcbptr,z_fzkgno)

struct KCB *z_kcbptr;
short  z_fzkgno;                        /* FAE entry no (subscript type)*/
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfle.h"   /* Fuzokugo Linkage table Entry (FLE) format    */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */

   faeptr1 = kcb.faefae+ z_fzkgno;      /* establish addr'blity to FAE  */
   fleptr1 = kcb.flefle + fae.flendx;   /* establish addr'blity to FLE  */

   if(( fle.lkvt[0] & 0xffff)  ||       /* the 1st 24 bits represents   */
      ( fle.lkvt[1] & 0xff00))          /* poss'ty to connect jiritsugo */
      return( ON );                     /* set possibility to conj      */
   else                                 /* set no possibility           */
      return( OFF );
}
