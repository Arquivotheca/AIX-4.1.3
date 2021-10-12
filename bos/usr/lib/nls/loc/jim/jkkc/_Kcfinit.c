static char sccsid[] = "@(#)62	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfinit.c, libKJI, bos411, 9428A410j 6/4/91 11:23:54";
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
 * MODULE NAME:       _Kcfinit
 *
 * DESCRIPTIVE NAME:  INITIALAZE FUZOKUGO WORKING AREA
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
#include   "_Kcrcb.h"                   /* Define Return Code Structure */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
void   _Kcfinit(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void            _Kccpurg();  /*fuzokugo wk.initialize routine */
   extern struct RETFSTMP _Kcfstmp();  /* making dummy fzokugo routine  */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcfkx.h"

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;

/*----------------------------------------------------------------------*
 *        INIT FUZOKUGO TABLE & CREATE DUMMY FUZOKUGO
 *----------------------------------------------------------------------*/
   _Kccpurg(&kcb.fthchh);               /* purge FTE                    */
   _Kccpurg(&kcb.fwhchh);               /* purge FWE                    */

   kcb.fkxacfkx = 0;                    /* reset no of acrive element   */
   kcb.fthfnum = 0;                     /* reset total of fuzokugo      */
}
