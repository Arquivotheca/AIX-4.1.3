static char sccsid[] = "@(#)61	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfhyki.c, libKJI, bos411, 9428A410j 6/4/91 10:28:10";
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
 * MODULE NAME:       _Kcfhyki
 *
 * DESCRIPTIVE NAME:  PROCESS OF FOZOKUGO LEARNING
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS) : Success
 *                    0x1604(FKXOVER) : FKX overflow
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
struct RETFHYKI _Kcfhyki(z_kcbptr,z_fweptr,z_joshi)
struct  KCB  *z_kcbptr;
struct  FWE  *z_fweptr;
uschar   z_joshi;
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcfwe.h"   /* Fuzokugo Work table Entry (FWE)              */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */
#include   "_Kcfkj.h"   /* Fuzokugo KanJi hyoki table (FKJ)             */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_H4BIT      0xF0
#define    Z_L4BIT      0x0F

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETFHYKI z_ret;       /* Define Area for Return of Own        */
   uschar z_hyki;               /* hyoki level                          */
   uschar z_dfg;                /* learning data                        */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address    to KCB  */

   fweptr1 = z_fweptr;                  /* set FWE   address            */

   fkjptr1 = kcb.fkjfkj + fwe.kjn;      /* set FKJ   address            */

   z_dfg   = *(kcb.dfgdfg+fwe.kjn);     /* get FUZOKUGO learning data   */

   gpwptr1 = kcb.gpwgpe;                /* establish address            */

   if( kcb.fkxacfkx == MAXFKX )         /* if beyoud max of FKX then    */
   {
      z_ret.rc = FKXOVER;               /* initialize return code       */
      return( z_ret );                  /* then FKX overflow            */
   }

   z_ret.rc = SUCCESS;                  /* initialize return code       */

   fkxptr1 = kcb.fkxfkx + kcb.fkxacfkx; /* set FKX address              */

/*----------------------------------------------------------------------*  
 *      LOOK INTO IF FKJ ENTRIES EXIST
 *----------------------------------------------------------------------*/ 
   if( fwe.kjn != NULL )                /* pointer to FKJ               */
   {
      fkx.stap = fwe.stap;              /* move mora position to FKX    */
      fkx.kjfp = fwe.kjn;               /* move FKJ pointer   to FKX    */

      /*----------------------------------------------------------------*  
       *      LOOK INTO POSITION IN THE PHRASE
       *----------------------------------------------------------------*/ 
      if( (z_joshi == 'J' ) || ( z_joshi == 'T'))
      {                                 /* J : joshi                    */
                                        /* T : the begin'g of phrase    */
         z_hyki =  ( fkj.hdr.hdr1 & Z_H4BIT) >> 4;
                                        /* point corres'nding FKJ entry */
      }                                 /*  by fkjptr + fwe.kjn         */
                                        /* keep begining of phrase lvl  */
      else
         z_hyki =  (fkj.hdr.hdr1 & Z_L4BIT);
                                        /* keep endof phrase lvl        */

      /*----------------------------------------------------------------*  
       *      LOOK INTO STATE OF LEARNING IN DFZ
       *----------------------------------------------------------------*/ 
      if( z_dfg == Z_H4BIT )
      {                                 /* check correspn'd DFZ entry   */
         z_hyki =  5;                   /* force to use kanji-hyki      */
      }

      if( z_dfg == Z_L4BIT )
      {                                  /* check correspn'd DFZ entry   */
         z_hyki =  0;                    /* force to use hiragana-hyki   */
      }

      if( z_hyki > gpw.hykilvl )         /* compare curr-hyki level      */
      {                                /* with temporary hyki(z_z.hyki)*/
         fkx.flag = FKXCONT | FKXKJYN;
      }
      else
         fkx.flag = FKXCONT;

      z_ret.fkxptr = fkxptr1;           /* return curr FKX entry pointer*/
      kcb.fkxacfkx ++;                  /* modified                     */

   }
   else
      z_ret.fkxptr = NULL;              /* return no FKX entry pointer  */

   return(z_ret);
}
