static char sccsid[] = "@(#)44	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccpurg.c, libKJI, bos411, 9428A410j 6/4/91 10:21:41";
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
 * MODULE NAME:       _Kccpurg
 *
 * DESCRIPTIVE NAME:  PURGE ALL ACTIVE ELEMENTS
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


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
void   _Kccpurg(z_chhptr)

struct CHH      *z_chhptr;              /* address of CHH               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void   _Kccfree();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short   z_rc;                        /* return code save area        */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   chhptr1 = z_chhptr;                  /* set base address             */
                                        /*       of the CHain Header    */ 
/*----------------------------------------------------------------------*
 * SCAN THE ACTIVE CHAIN AND UNCHAIN ALL ACTIVE ELEM                    *
 *    FROM THE ACTIVE CHAIN AND RETURN THEM TO THE FREE CHAIN           *
 *----------------------------------------------------------------------*/
   for (;;)
   {
      if ( chh.actv == 0 )              /* break the loop               */
         break;                         /*    the elem is the last elem */

      _Kccfree( z_chhptr, CHEPTR(chh.act) );
   }
}
