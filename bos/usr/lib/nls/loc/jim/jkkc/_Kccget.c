static char sccsid[] = "@(#)30	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccget.c, libKJI, bos411, 9428A410j 6/4/91 12:47:54";
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
 * MODULE NAME:       _Kccget
 *
 * DESCRIPTIVE NAME:  GET FREE ELEMENT
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:   0x00ff(GET_TOP_MID): elem isn't the last elem in free chain
 *                0x01ff(GET_LAST):    elem is the last element in free chain
 *                0x02ff(GET_EMPTY):   no element exists in free chain.
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *       INCLUDE FILES    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"
#include   "_Kcrcb.h"


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETCGET _Kccget(z_chhptr)
struct CHH *z_chhptr;                   /* address of CHH on which an   */
                                        /*      elem is to be obtained  */
{
/*----------------------------------------------------------------------* 
 *       INCLUDE FILES    
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"
#include   "_Kcche.h"
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLE 
 *----------------------------------------------------------------------*/
   struct RETCGET   z_rcb;              /* return code block            */
   short  z_chesub;
/*----------------------------------------------------------------------*
 *      CHECK IF ANY FREE ELEMENT IS AVAILABLE 
 *----------------------------------------------------------------------*/
   chhptr1 = z_chhptr;                  /* set base addr of CHH         */
   if ( chh.actv >= chh.mxele )         /* check                        */
      z_rcb.rc = GET_EMPTY;             /*   if free chain is available */

   else
   {

   /*-------------------------------------------------------------------*
    * UNCHAIN THE TOP ELEMENT OF THE FREE CHAIN    
    *-------------------------------------------------------------------*/
      z_chesub = chh.fre;               /* return addr of obtained elem */

      cheptr1 = CHEPTR(chh.fre);        /* set base addr of CHE         */
      if ( che.last == 1 )              /* if the elem is the last elem */
      {                                 /* on the free chain            */
         chh.fre = NEGA;
         z_rcb.rc = GET_LAST;
      }
      else                              /* if the elem is the top elem  */
      {                                 /* or middle elem on free chain */
         chh.fre = che.chef;
         z_rcb.rc = GET_TOP_MID;
      }

   /*-------------------------------------------------------------------*
    *   ADD THE ELEMENT TO THE BOTTOM OF THE ACTIVE CHAIN   
    *-------------------------------------------------------------------*/
      che.actv = 1;                     /* set the active-flag on       */

      if ( chh.actv == 0 )              /* add this elem as an only     */
      {                                 /*    elem on the active chain  */
         chh.act = z_chesub;        /* if no active elem exists     */
         che.chef = z_chesub;
         che.cheb = z_chesub;
         che.top = 1;
         che.last = 1;
      }

      else                              /* add this elem as the new     */
      {                                 /*       last-in-chain elem     */
         cheptr2 = CHEPTR(chh.act);     /*       on the active chain    */
                                        /* if some active elems exist   */
         che.chef = chh.act;
         che.cheb = che2.cheb;
         che.top = 0;
         che.last = 1;

         che2.cheb = z_chesub;
         
         cheptr2 = CHEPTR(che.cheb);
         che2.chef = z_chesub;
         che2.last = 0;
      }

      chh.actv++;                       /* increase the active elem cnt */
   }

/*----------------------------------------------------------------------*
 *      RETURN         
 *----------------------------------------------------------------------*/
   z_rcb.cheptr = CHEPTR(z_chesub);
   return(z_rcb);
}
