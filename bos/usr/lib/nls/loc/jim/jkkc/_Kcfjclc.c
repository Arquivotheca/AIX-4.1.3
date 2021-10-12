static char sccsid[] = "@(#)63	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfjclc.c, libKJI, bos411, 9428A410j 6/4/91 12:48:08";
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
 * MODULE NAME:       _Kcfjclc
 *
 * DESCRIPTIVE NAME:  CALCULATE PENALTY BETWEEN JIRITSUGO & FUZOKUGO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       PENALTY BETWEEN JIRITSUGO AND FUZOKUGO
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
short _Kcfjclc(z_kcbptr,z_jhin,z_jteptr,z_fteptr)
struct KCB      *z_kcbptr;              /* get address of KCB           */
short            z_jhin ;               /* get jiritsu setsuzoku hinshi */
struct JTE      *z_jteptr;              /* get address of JTE           */
struct FTE      *z_fteptr;              /* get address of FTE           */
  
{ 
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcfte.h"   /* Fuzokugo Table Entry (FTE)                   */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfle.h"   /* Fuzokugo Linkage table Entry (FLE) format    */
#include   "_Kcfpe.h"   /* Fuzokugo Penalty adjustment table Entry (FPE)*/
 
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_MAX_PEN    9999
#define    Z_SUB1   ((z_jhin-1) / 16)   /* use the upper 4 bits as indx */
#define    Z_SUB2   ((z_jhin-1) % 16)   /* use the lower 4 bits as indx */

/*----------------------------------------------------------------------*  
 *       THIS PROGRAM CHECK TEH CONECTABILITY OF BETWEEN
 *       2 FUZOKUGOS OR FUZOKUGO AND JIRITSUGO
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   fteptr1 = z_fteptr;                  /* set base address of FTE      */ 

   faeptr1 = kcb.faefae + fte.hinl;     /* set FAE address from hinl    */
   fleptr1 = kcb.flefle + fae.flendx;   /* set FLE address from flendx  */
   if( fle.lkvt[( Z_SUB1 )] & ( 0x8000 >> ( Z_SUB2 )))
                                        /* check conection penalty      */
   {                                    /* if it was                    */
      fpeptr1 = kcb.fpefpe + fle.fpendx;/* set FPE addr. from fpendx    */
      for (;fpe.hin < 0;fpeptr1++)      /* loop all penalty entry       */
      {
         if (abs(fpe.hin) == z_jhin)    /* if conection                 */
         { 
            if (fpe.pen == -1)          /* if no penalty                */
            { 
               return(0);               /* return with 0 as no penalty  */
            }  
            return(fpe.pen);            /* return with penalty          */
         }
      }                                 /* end of for loop              */
      return(0);                        /* return with no penalry       */
   }                                    /* endif of checking conection  */
   else 
      return(Z_MAX_PEN);                /* return with MAX value        */
}                                       /* end of program               */
