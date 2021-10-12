static char sccsid[] = "@(#)29	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccfree.c, libKJI, bos411, 9428A410j 6/4/91 10:17:48";
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
 * MODULE NAME:       _Kccfree
 *
 * DESCRIPTIVE NAME:  FREE AN ACTIVE ELEMENT
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
void   _Kccfree(z_chhptr,z_cheptr)

struct CHH      *z_chhptr;              /* address of CHH to which      */
                                        /* the elem being freed belongs */
struct CHE      *z_cheptr;              /* address of the element       */
                                        /*                  being freed */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcche.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
   short   z_i;                         /* counter                      */
   short   z_chesub;
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   chhptr1 = z_chhptr;                  /* set base address of CHH      */
   cheptr1 = z_cheptr;                  /* set base address of CHE      */
   z_chesub = CHESUB(z_cheptr);

/*----------------------------------------------------------------------*
 * REFORMAT THE CHE FIELDS                                              
 *----------------------------------------------------------------------*/
   che.rsv01 = 0x00;                    /* reformat the CHE fields      */

/*----------------------------------------------------------------------*
 * UNCHAIN THE ELEM FROM THE ACTIVE CHAIN AND ADD IT TO THE FREE CHAIN  
 *----------------------------------------------------------------------*/

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE MIDDLE ELEM ON THE ACTIVE CHAIN                  
    *    AND SOME ELEMS EXIST ON THE FREE CHAIN                         
    *-------------------------------------------------------------------*/
   if ( (che.top == 0) && (che.last == 0) && (chh.fre != NEGA) )
   {                                    /*if the elem is the middle elem*/
      cheptr2 = CHEPTR(che.chef);       /* compleate the next elem      */
      che2.cheb = che.cheb;

      cheptr2 = CHEPTR(che.cheb);       /* compleate the previous elem  */
      che2.chef = che.chef;

      che.chef = chh.fre;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.cheb = NEGA;                  /* compleate the element        */
      che.actv = 0;
   }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE TOP ELEM ON THE ACTIVE CHAIN                    
    *    AND SOME ELEMS EXIST ON THE FREE CHAIN                       
    *-------------------------------------------------------------------*/
   else if ( (che.top == 1) && (che.last == 0) && (chh.fre != NEGA) )
   {                                    /* if the elem is the top elem  */
      cheptr2 = CHEPTR(che.chef);       /* compleate the next elem      */
      che2.top = 1;
      che2.cheb = che.cheb;             

      cheptr2 = CHEPTR(che.cheb);       /* compleate the previous elem  */
      che2.chef = che.chef;

      chh.act = che.chef;               /* set ptr of top elem          */
                                        /*           on active chain    */

      che.chef = chh.fre;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.actv = 0;
      che.top = 0;                      /* compleate the element        */
      che.cheb = NEGA;
   }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE LAST ELEM ON THE ACTIVE CHAIN            
    *    AND SOME ELEMS EXIST ON THE FREE CHAIN           
    *-------------------------------------------------------------------*/
   else if ( (che.top == 0) && (che.last == 1) && (chh.fre != NEGA) )
   {                                    /* if the elem is the last elem */
      cheptr2 = CHEPTR(che.chef);        /* compleate the next elem      */
      che2.cheb = che.cheb;

      cheptr2 = CHEPTR(che.cheb);        /* compleate the previous elem  */
      che2.last = 1;
      che2.chef = che.chef;

      che.chef = chh.fre;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.cheb = NEGA;                  /* compleate the element        */
      che.actv = 0;
      che.last = 0;
   }

   /*-------------------------------------------------------------------*
    * ONLY THIS ELEM EXISTS ON THE ACTIVE CHAIN                        
    *    AND SOME ELEMS EXIST ON THE FREE CHAIN                         
    *-------------------------------------------------------------------*/
   else if ( (che.top == 1) && (che.last == 1) && (chh.fre != NEGA) )
   {                                    

      che.chef = chh.fre;
      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;
      chh.act = NEGA;

      che.actv = 0;                     /* compleate the element        */
      che.top = 0;
      che.cheb = NEGA;
      che.last = 0;
   }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE MIDDLE ELEM ON THE ACTIVE CHAIN         
    *    AND NO ELEMS EXIST ON THE FREE CHAIN               
    *-------------------------------------------------------------------*/
   else if( (che.top == 0) && (che.last == 0) && (chh.fre == NEGA) )
   {                                    /*if the elem is the middle elem*/
      cheptr2 = CHEPTR(che.chef);       /* compleate the next elem      */
      che2.cheb = che.cheb;

      cheptr2 = CHEPTR(che.cheb);       /* compleate the previous elem  */
      che2.chef = che.chef;

      che.chef = NEGA;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.cheb = NEGA;                  /* compleate the element        */
      che.actv = 0;
      che.last = 1;
   }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE TOP ELEM ON THE ACTIVE CHAIN      
    *    AND NO ELEMS EXIST ON THE FREE CHAIN       
    *-------------------------------------------------------------------*/
   else if ( (che.top == 1) && (che.last == 0) && (chh.fre == NEGA) )
   {                                    /* if the elem is the top elem  */
      cheptr2 = CHEPTR(che.chef);       /* compleate the next elem      */
      che2.top = 1;
      che2.cheb = che.cheb;             

      cheptr2 = CHEPTR(che.cheb);       /* compleate the previous elem  */
      che2.chef = che.chef;

      chh.act = che.chef;               /* set ptr of top elem          */
                                        /*           on active chain    */

      che.chef = NEGA;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.actv = 0;
      che.top = 0;                      /* compleate the element        */
      che.cheb = NEGA;
      che.last = 1;
   }

   /*-------------------------------------------------------------------*
    * THIS ELEM IS THE LAST ELEM ON THE ACTIVE CHAIN
    *    AND NO ELEMS EXIST ON THE FREE CHAIN      
    *-------------------------------------------------------------------*/
   else if ( (che.top == 0) && (che.last == 1) && (chh.fre == NEGA) )
   {                                    /* if the elem is the last elem */
      cheptr2 = CHEPTR(che.chef);       /* compleate the next elem      */
      che2.cheb = che.cheb;

      cheptr2 = CHEPTR(che.cheb);       /* compleate the previous elem  */
      che2.last = 1;
      che2.chef = che.chef;

      che.chef = NEGA;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;

      che.cheb = NEGA;                  /* compleate the element        */
      che.actv = 0;
   }

   /*-------------------------------------------------------------------*
    * ONLY THIS ELEM EXISTS ON THE ACTIVE CHAIN  
    *    AND NO ELEMS EXIST ON THE FREE CHAIN 
    *-------------------------------------------------------------------*/
   else /*if ( (che.top == 1) && (che.last == 1) && (chh.fre == NEGA) ) */
   {                                    /* if the active chain has only */
                                        /*                  one element */
      che.chef = NEGA;

      chh.fre = z_chesub;               /* compleate the CHH            */
      chh.actv--;
      chh.act = NEGA;

      che.actv = 0;                     /* compleate the element        */
      che.top = 0;
      che.cheb = NEGA;
   }
}
