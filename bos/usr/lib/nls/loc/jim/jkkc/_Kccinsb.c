static char sccsid[] = "@(#)35	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccinsb.c, libKJI, bos411, 9428A410j 6/4/91 10:18:53";
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
 * MODULE NAME:       _Kccinsb
 *
 * DESCRIPTIVE NAME:  INSERT ELEMENT BEFORE THE TARGET
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x7fff(UERROR):  unpredictable error
 *
 ************************************************************************/
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
short _Kccinsb(z_chhptr,z_chep1,z_chep2)

struct CHH      *z_chhptr;              /* address of CHH               */
                                        /*             to be acted upon */
struct CHE      *z_chep1;               /* addr of CHE to be moved      */
struct CHE      *z_chep2;               /* addr of the target elem      */
{

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcche.h"                   /* CHain Header map (CHH)       */

   short   z_chesb1;
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   chhptr1 = z_chhptr;                  /* set base address             */
                                        /*       of the CHain Header    */ 
   z_chesb1 = CHESUB(z_chep1);          /* set subscript 1              */

/*----------------------------------------------------------------------*
 * CHECK THE ACTIVE ELEM COUNT               
 *----------------------------------------------------------------------*/
   if ( chh.actv == 0 )                 /* return                       */
      return( UERROR );                 /* if the active chain is empty */
                                        
/*----------------------------------------------------------------------*
 * CHECK IF TARGET ELEM IS AS SAME AS THIS ELEM  
 *----------------------------------------------------------------------*/
   else if ( z_chep1 == z_chep2 )
      return( SUCCESS );

/*----------------------------------------------------------------------*
 * UNCHAIN THE ELEM SPECIFIED           
 *----------------------------------------------------------------------*/
   else 
   {
      cheptr1 = z_chep1;                /* set base addr of CHE         */

   /*-------------------------------------------------------------------*
    * THE ELEM IS THE TOP ELEM ON THE ACTIVE CHAIN  
    *-------------------------------------------------------------------*/
      if ( ( che.top == 1 ) && ( che.last == 1 ) )
         return( SUCCESS );

      else if ( che.top == 1 )
      {
         chh.act = che.chef;            /* complete the CHH             */

         cheptr2 = CHEPTR(che.chef);    /* complete the next CHE        */
         che2.cheb = che.cheb;
         che2.top = 1;

         cheptr2 = CHEPTR(che.cheb);    /* complete the previous CHE    */
         che2.chef = che.chef;
      }

   /*-------------------------------------------------------------------*
    * THE ELEM IS THE LAST ELEM ON THE ACTIVE CHAIN 
    *-------------------------------------------------------------------*/
      else if ( che.last == 1 )
      {
         cheptr2 = CHEPTR(che.chef);    /* complete the next CHE        */
         che2.cheb = che.cheb;

         cheptr2 = CHEPTR(che.cheb);    /* complete the previous CHE    */
         che2.chef = che.chef;
         che2.last = 1;
      }

   /*-------------------------------------------------------------------*
    * THE ELEM IS THE MIDDLE ELEM ON THE ACTIVE CHAIN 
    *-------------------------------------------------------------------*/
      else
      {
         cheptr2 = CHEPTR(che.chef);    /* complete the next CHE        */
         che2.cheb = che.cheb;

         cheptr2 = CHEPTR(che.cheb);    /* complete the previous CHE    */
         che2.chef = che.chef;
      }
         
/*----------------------------------------------------------------------*
 * INSERT THE UNCHAINED ELEM INTO NEW POTISION       
 *----------------------------------------------------------------------*/
      cheptr2 = z_chep2;                /* set base addr of CHE         */

   /*-------------------------------------------------------------------*
    * THIS ELEM IS INSERTED AT THE TOP OF THE ACTIVE CHAIN 
    *-------------------------------------------------------------------*/
      if ( z_chep2 == NULL )            /* if the target elem is NULL   */
      {
         che.top = 1;
         che.last = 0;

         cheptr2 = CHEPTR(chh.act);
         che.chef = chh.act;
         che.cheb = che2.cheb;
         che2.top = 0;
         che2.cheb = z_chesb1;

         cheptr2 = CHEPTR(che.cheb);
         che.chef = che2.chef;
         che2.chef = z_chesb1;

         chh.act = z_chesb1;
      }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS INSERTED AT THE TOP OF THE ACTIVE CHAIN
    *-------------------------------------------------------------------*/
      else if ( che2.top == 1 )         /* if the target elem           */
      {                                 /*      is the top elem         */
         che.top = 1;
         che.last = 0;

         che.cheb = che2.cheb;
         che2.cheb = z_chesb1;
         che2.top = 0;

         cheptr2 = CHEPTR(che.cheb);
         che.chef = che2.chef;
         che2.chef = z_chesb1;

         chh.act = z_chesb1;
      }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS INSERTED AT THE BEFORE OF THE TARGET ELEM 
    *-------------------------------------------------------------------*/
      else                              /* if the target elem           */
      {                                 /*   is the last or middle elem */
         che.top = 0;
         che.last = 0;

         che.cheb = che2.cheb;
         che2.cheb = z_chesb1;

         cheptr2 = CHEPTR(che.cheb);
         che.chef = che2.chef;
         che2.chef = z_chesb1;
      }

   /*-------------------------------------------------------------------*
    * RETURN                                         
    *-------------------------------------------------------------------*/
      return( SUCCESS );
   }
}
