static char sccsid[] = "@(#)14	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbhyki.c, libKJI, bos411, 9428A410j 6/4/91 10:12:09";
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
 * MODULE NAME:       _Kcbhyki
 *
 * DESCRIPTIVE NAME:  SET PARAMETER OF FUZOKUGO HYOKI ON BTE
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       VOID
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
void _Kcbhyki(z_kcbptr,z_bteptr,z_fteptr)
struct  KCB  *z_kcbptr;
struct  BTE  *z_bteptr;
struct  FTE  *z_fteptr;
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcfte.h"   /* Fuzokugo Table Entry (FTE)                   */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address    to KCB  */
   bteptr1 = z_bteptr;                  /* establish address    to BTE  */
   fteptr1 = z_fteptr;                  /* establish address    to FTE  */

/*----------------------------------------------------------------------*  
 *      WHEN FUZOKUGO HYOKI EXISTS
 *----------------------------------------------------------------------*/ 
   bte.fzkflg = NULL;                   /* Initialize fzkflg            */
   if( fte.fhtx != -1)                  /* look into if fuzokugo may be */
                                        /* written in kanji(-hyoki ari) */
   {
      fkxptr1 = kcb.fkxfkx + fte.fhtx;  /* point the corresp'ding FKX   */
                                        /* entry                        */
      bte.kjf1 = fkx.stap;              /* move mora pos in FKX to BTE  */

      if ( ( fkx.flag & FKXKJYN ) == FKXKJYN )
      {
         bte.fzkflg = F_FLG_USE1;
         bte.kjh1 = fkx.kjfp;           /* move FKJ ptr  in FKX to BTE  */
      }
      else
      {
         bte.fzkflg = NULL;
         bte.kjh1 = fkx.kjfp * (-1);    /* move FKJ ptr  in FKX to BTE  */
      }

      if( ( fkx.flag & FKXCONT ) == FKXCONT )
                                        /* FKX continue                 */
      {
         fkxptr1 ++;                    /* advance ptr to next FKX      */
         bte.kjf2 = fkx.stap;           /* move mora pos in FKX to BTE  */

         bte.fzkflg |= F_FLG_TWO;       /* 2nd HYOKI exists             */
         if ( ( fkx.flag & FKXKJYN ) == FKXKJYN )
         {
            bte.fzkflg |= F_FLG_USE2;   /* 2nd HYOKI will be used       */
            bte.kjh2 = fkx.kjfp;        /* move FKJ ptr in FKX to BTE   */
         }
         else
         {
            bte.kjh2 = fkx.kjfp * (-1); /* move FKJ ptr in FKX to BTE   */
         }
      }
      else
         bte.kjh2 = 0;
   }
   else
/*----------------------------------------------------------------------*  
 *      WHEN FUZOKUGO HYOKI DOES NOT EXIST
 *----------------------------------------------------------------------*/ 
   {
      bte.fzkflg = F_FLG_NOEXT;         /*  no kj-hyki for fzkg         */
      bte.kjh1 = 0;
      bte.kjh2 = 0;
   }
}
