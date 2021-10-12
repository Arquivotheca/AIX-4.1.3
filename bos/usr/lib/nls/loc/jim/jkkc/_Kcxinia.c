static char sccsid[] = "@(#)05	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxinia.c, libKJI, bos411, 9428A410j 6/4/91 15:29:05";
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
 * MODULE NAME:       _Kcxinia
 *
 * DESCRIPTIVE NAME:  INITIALAZE ALL CONTROL BLOCKS
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
void   _Kcxinia(z_kcbptr)                /* initialize all control block */

struct KCB      *z_kcbptr;              /* base  address of KCB         */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void   _Kccpurg();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcyce.h"
#include   "_Kcype.h"
#include   "_Kcfkx.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short   z_i;                         /* counter                      */
   char    *z_wk;                       /* pointer for work             */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set address of YCE pool      */

/*----------------------------------------------------------------------*
 *      PURGE CHAIN TABLES
 *----------------------------------------------------------------------*/
   _Kccpurg(&kcb.bthchh);               /* init. ctl blocks(JTE)        */
   _Kccpurg(&kcb.jthchh);               /* init. ctl blocks(JKJ)        */
   _Kccpurg(&kcb.jkhchh);               /* init. ctl blocks(JLE)        */
   _Kccpurg(&kcb.jlhchh);               /* init. ctl blocks(BTE)        */
   _Kccpurg(&kcb.bthchh);               /* init. ctl blocks(PTE)        */
   _Kccpurg(&kcb.pthchh);               /* init. ctl blocks(FTE)        */
   _Kccpurg(&kcb.fthchh);               /* init. ctl blocks(FKX)        */

/*----------------------------------------------------------------------*
 *      CLEAR TABLES
 *----------------------------------------------------------------------*/
                                        /* init. ctl blocks(FKX)        */
   for ( z_i = 0, z_wk = (char *)kcb.fkxfkx;
                z_i < ( sizeof(struct FKX) * kcb.maxfkx ); z_i++, z_wk++ )
      *z_wk = NULL;
                                        /* init. ctl blocks(YCE)        */
   for ( z_i = 0, z_wk = (char *)kcb.ychyce;
                z_i < ( sizeof(struct YCE) * kcb.ychmxyce ); z_i++, z_wk++ )
      *z_wk = NULL;
                                        /* init. ctl blocks(YPE)        */
   for ( z_i = 0, z_wk = (char *)kcb.yphype;
                z_i < ( sizeof(struct YPE) * kcb.yphmxype ); z_i++, z_wk++ )
      *z_wk = NULL;
                                        /* init. ctl blocks(MCE)        */
   for ( z_i = 0, z_wk = (char *)kcb.mchmce;
                z_i < ( sizeof(struct MCE) * kcb.mchmxmce ); z_i++, z_wk++ )
      *z_wk = NULL;

/*----------------------------------------------------------------------* 
 *        INITIALIZE CONTROL VARIABLES IN GPW                          
 *----------------------------------------------------------------------*/
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */
   gpw.tbflag   = 1;                    /* Single phrase. Perfomance    */
                                        /* guarantee for floppy         */
   gpw.pendpos  = -2;                   /* Adjunct ayalysis position    */
   gpw.moraof2p = -1;                   /* Last pos. of 2nd phrase isn't*/
                                        /* defined.                     */
   gpw.pmode    = 0x99;                 /* Previous Mode                */
   gpw.pkakutei = PKAKNON;              /* Previous Kakutei status      */
   gpw.hykilvl  = 1;                    /* hyoki level                  */
   gpw.lastbte  = NULL;                 /* last created busetsu address */
   gpw.accfirm  = 0;                    /* internal kakutei no. yomi    */
   gpw.leftflg  = OFF;                  /* internal left word flag      */
   gpw.kakuflg  = OFF;                  /* internal kakutei flg         */
   gpw.pthpphin = 0;                    /* prev. hinsi                  */
   gpw.ppdflag[0]= 0;                   /* prev. flag[0]                */
   gpw.ppdflag[1]= 0;                   /* prev. flag[1]                */
   gpw.ppsahen  = 0;                    /* prev. sahen                  */
}
