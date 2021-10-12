static char sccsid[] = "@(#)59	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcffcon.c, libKJI, bos411, 9428A410j 6/4/91 10:27:19";
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
 * MODULE NAME:       _Kcffcon
 *
 * DESCRIPTIVE NAME:  CHECK THE CONNECTIVITY BETWEEN TWO FUZUKOGO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       PENALTY
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
struct RETFFCON _Kcffcon(z_kcbptr,z_fzkno1,z_fzkno2)

struct KCB *z_kcbptr;
short      z_fzkno1;
short      z_fzkno2;
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfle.h"   /* Fuzokugo Linkage table Entry (FLE) format    */
#include   "_Kcfpe.h"   /* Fuzokugo Penalty adjustment table Entry (FPE)*/

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_SUB1   ((fae.hin-1) / 16)  /* use the upper 4 bits as indx */
#define    Z_SUB2   ((fae.hin-1) % 16)  /* use the lower 4 bits as indx */

#define    Z_NOT_CON  -128

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETFFCON z_ret;       /* Define Area for Return of Own        */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address to kcb     */

   faeptr1  = kcb.faefae + z_fzkno1;    /* point correpn'd FAE entry    */
                                        /*  of fuzokugo no.1            */
   faeptr2  = kcb.faefae + z_fzkno2;    /* point correpn'd FAE entry    */
                                        /*  of fuzokugo no.2            */
   fleptr1 = kcb.flefle + fae2.flendx;  /* point corrent FLE entry      */

   /*-------------------------------------------------------------------*  
    *      LOOK INTO THE LINKAGE VECTOR TABLE OF FUZOKUGO 2
    *-------------------------------------------------------------------*/ 
   if( fle.lkvt[( Z_SUB1 )] & ( 0x8000 >> ( Z_SUB2 )))
   {
      z_ret.rc    = 0  ;                /* set penalty                  */
      z_ret.oflag = OFF;                /* set oflag (return code) off  */
      fpeptr1 = kcb.fpefpe + fle.fpendx - 1;
                                        /* point corrent FPE entry      */
      /*----------------------------------------------------------------*  
       *      UNTIL FPE ENTRIES EXIST
       *----------------------------------------------------------------*/ 
      do
      {
         fpeptr1++;
         if( abs( fpe.hin ) == fae.hin )
         {                              /* compare with absolute value  */
            if( fpe.pen == -1 )
            {
               z_ret.oflag = ON;        /* set O flag                   */
               z_ret.rc = 0;            /* set penalty as return value  */
               break;
            }
            z_ret.rc = fpe.pen;         /* return penalty value         */
            break;
         }

      }while( fpe.hin < 0 );            /* if value of fpe.hin is minus */
                                        /* next entry exists            */
   }
   else
   {
      z_ret.rc = Z_NOT_CON;
   }
   return(z_ret);                       /* return to caller with having */
}
