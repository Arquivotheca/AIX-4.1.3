static char sccsid[] = "@(#)47	1.4.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kctmpry.c, libKJI, bos411, 9428A410j 7/23/92 03:17:05";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kctmpry
 *
 * DESCRIPTIVE NAME:  These Modules are tempolary before new monitor
 *                    will be made up.
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       Void
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
 *      1. Shift Yomi Code  1 culumn right and set mode on mcb.
 ************************************************************************/ 
void   _Kctalph( z_mcbptr )
struct  MCB     *z_mcbptr;              /* initialize of KCB            */
{

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kcmcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short        z_i;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   mcbptr1 = z_mcbptr;                  /* initialize of mcbptr1        */
   ymiptr1 = mcb.ymiaddr;               /* initialize of ymiptr1        */
   seiptr1 = mcb.seiaddr;               /* initialize of seiptr1        */

   if (ymi.yomi[0] == ESC_ALPHA )            /* if Alphanumeric mode         */
   {
   /*-------------------------------------------------------------------*
    *      CHECK THE SEISHO & YOMI  AND SHIFT YOMI BACKWARD
    *-------------------------------------------------------------------*/
      mcb.ymill1--;
      for(z_i = 0;z_i < mcb.ymill1; z_i++)
      {
         ymi.yomi[z_i] = ymi.yomi[z_i + 1];
      }
      mcb.mode = TSMALPHA;
   }
}

/************************************************************************
 *      START OF FUNCTION
 *      2. Change Yomi code from 7 bit yomi code to Alphanum 7 bit yomi
 ************************************************************************/
static   uschar   y7bittb[] = {
                                 Y_A_CNM,/* CONMA       0x72             */
                                 Y_A_PRD,/* PIRIOD      0x73             */
                                 0x00,   /* INVALID     0x74             */
                                 Y_A_0,  /* 0           0x75             */
                                 Y_A_1,  /* 1           0x76             */
                                 Y_A_2,  /* 2           0x77             */
                                 Y_A_3,  /* 3           0x78             */
                                 Y_A_4,  /* 4           0x79             */
                                 Y_A_5,  /* 5           0x7a             */
                                 Y_A_6,  /* 6           0x7b             */
                                 Y_A_7,  /* 7           0x7c             */
                                 Y_A_8,  /* 8           0x7d             */
                                 Y_A_9   /* 9           0x7e             */
                               };

void _Kctmix1(z_kcbptr)
struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcymi.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short               z_i;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */

/*----------------------------------------------------------------------*
 *      BRANCH EACH FUNCTION
 *----------------------------------------------------------------------*/
   if ( kcb.func != FUNINITW ) {
      if ( ( kcb.mode == MODALPHA) || ( kcb.mode == MODKANSU ) ) {
         if ( kcb.mode == MODALPHA)
            kcb.alpkan = ALPHMASK;
         else
            kcb.alpkan = (KANMASK | ALPHMASK);
         kcb.mode = MODALKAN;
      }
   }
}


static   uschar   a7bittb[] = {
                                 Y_CNM,  /* CONMA     2C                 */
                                 Y_CHO,  /* INVALID   --                 */
                                 Y_HDK,  /* PIRIOD    2E                 */
                                 Y_CHO,  /* INVALID   --                 */
                                 Y_0  ,  /* 0         30                 */
                                 Y_1  ,  /* 1         31                 */
                                 Y_2  ,  /* 2         32                 */
                                 Y_3  ,  /* 3         33                 */
                                 Y_4  ,  /* 4         34                 */
                                 Y_5  ,  /* 5         35                 */
                                 Y_6  ,  /* 6         36                 */
                                 Y_7  ,  /* 7         37                 */
                                 Y_8  ,  /* 8         38                 */
                                 Y_9     /* 9         39                 */
                               };

void _Kctmix2(z_kcbptr,z_modbak)
struct KCB      *z_kcbptr;              /* get address of KCB           */
short            z_modbak;              /* mode backup                  */
{
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcsem.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcymi.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short               z_i;
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
/*----------------------------------------------------------------------*
 *      SET SEICHO MAP
 *----------------------------------------------------------------------*/
                                        /* if alpha or kansu mode       */
   if ( ( z_modbak == MODALPHA) || ( z_modbak == MODKANSU ) )
   {                                    /* set seisho map pointer       */
      semptr1 = kcb.semaddr;
                                        /* if all conversion            */
      if ( (kcb.func == FUNALLOP)||
           (kcb.func == FUNALLFW)||
           (kcb.func == FUNALLBW) )
      {                                 /* change all seisho map        */
         for ( z_i = 0; z_i < kcb.totcand; z_i++ )
         {
            sem.code[0] = z_modbak;
            semptr1 = (struct SEM *)((uschar *)semptr1+CHPTTOSH(sem.ll));
         }
      }
      else                              /* change seisho map            */
      {
         sem.code[0] = z_modbak;
      }
      kcb.mode = z_modbak;              /* change last mode             */
   }
}
