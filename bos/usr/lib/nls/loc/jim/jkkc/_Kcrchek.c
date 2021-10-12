static char sccsid[] = "@(#)77	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrchek.c, libKJI, bos411, 9428A410j 6/4/91 15:20:57";
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
 * MODULE NAME:       _Kcrchek
 *
 * DESCRIPTIVE NAME:  JUDGE TO DO DECISION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       RETURN VALUE OF END POSITION
 *                    -1(Z_NOT_FIRM)  : not confirm
 *                     n              : mora position
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
short _Kcrchek(z_kcbptr,z_dctctl,z_mode)
struct KCB      *z_kcbptr;              /* get address of KCB           */
short            z_dctctl;              /* dict. looking up cntrl info. */
short            z_mode;
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCFPTH _Kccfpth();   /* Sellect Least Penalty Path   */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define    Z_NOT_FIRM   -1

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_ret;       /* return code                          */

/*----------------------------------------------------------------------*  
 *      JUDGE TO DO DECISION OR NOT TO DO
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */ 

   mceptr1 = kcb.mchmce ;               /* set MCE base pointer         */

   if(mce.maxln > gpw.pendpos+1)
      return(Z_NOT_FIRM);

   CMOVL(kcb.pthchh,pteptr1);
   if(pteptr1 == NULL)                  /* if PTE not exist             */
      return(Z_NOT_FIRM);

   if(z_mode == ABS)
   {
      z_ret = pte.endp - 3;             /* last PTE end position -3     */
   }
   else
   {
      if ((mce.jdok == JD_COMPLETE)&&      /* if looking up is completed   */
          ((z_dctctl > gpw.moraof2p+13)||
          (gpw.kakuflg != OFF )))
                                        /* dict control >= 13 mora      */
                                        /* seperate from longest 1st    */
                                        /* bunsetsu                     */
      {
         z_ret = pte.endp - 3;          /* last PTE end position -3     */
      }
      else
      {
         z_ret = Z_NOT_FIRM;            /* return with 'Do not proceed  */
      }
   }
   if (z_ret < 0)                       /* if end pos is negative       */
      z_ret = Z_NOT_FIRM;               /* not firm up                  */
   return(z_ret);
}                                       /* end program                  */
