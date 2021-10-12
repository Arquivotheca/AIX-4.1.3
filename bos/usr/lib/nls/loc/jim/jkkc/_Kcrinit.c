static char sccsid[] = "@(#)79	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrinit.c, libKJI, bos411, 9428A410j 6/4/91 15:21:13";
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
 * MODULE NAME:       _Kcrinit
 *
 * DESCRIPTIVE NAME:  INITIALAZE OUTPUT BUFFERS
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
void  _Kcrinit(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcsei.h"
#include   "_Kcsem.h"
#include   "_Kcgrm.h"
#include   "_Kcymm.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short    z_i;                        /* declare loop counter         */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
   seiptr1 = kcb.seiaddr;               /* set seisho pointer           */

/*----------------------------------------------------------------------* 
 *       INITIALIZE SEISHO BUFFER                                        
 *----------------------------------------------------------------------*/
   for(z_i=0;z_i < kcb.seimaxll;z_i++)  /* loop about all seisho buffer */
   {
      sei.kj[z_i][0] = NULL;            /* clear seisho data buffer     */
      sei.kj[z_i][1] = NULL;            /* clear seisho data buffer     */
   }
   sei.ll[0] = 0;                       /* set seisho data length       */
   sei.ll[1] = 2;                       /* set seisho data length       */

/*----------------------------------------------------------------------* 
 *       INITIALIZE SEISHO MAP BUFFER                                    
 *----------------------------------------------------------------------*/
   semptr1 = kcb.semaddr;               /* set sei.map pointer          */
   for(z_i=0;z_i < kcb.semmaxll;z_i++)  /* loop about all sei.map buffer*/
   {
      sem.code[z_i] = NULL;             /* clear sei.map data buffer    */
   }
   sem.ll[0] = 0;                       /* set sei.map data length      */
   sem.ll[1] = 2;                       /* set sei.map data length      */

/*----------------------------------------------------------------------* 
 *       INITIALIZE GRAMMER MAP BUFFER                                   
 *----------------------------------------------------------------------*/
   grmptr1 = kcb.grmaddr;               /* set grm.map pointer          */
   for(z_i=0;z_i < kcb.grmmaxll;z_i++)  /* loop about all grm.map buffer*/
   {
      grm.byte[z_i] = NULL;             /* clear grm.map data buffer    */
   }
   grm.ll    = 1;                       /* set grm.map data length      */

/*----------------------------------------------------------------------* 
 *       INITIALIZE YOMI MAP BUFFER                                      
 *----------------------------------------------------------------------*/
   ymmptr1 = kcb.ymmaddr;               /* set ymm.map pointer          */
   for(z_i=0;z_i < kcb.ymmmaxll;z_i++)  /* loop about all ymm.map buffer*/
   {
      ymm.byte[z_i] = NULL;             /* clear ymm.map data buffer    */
   }
   ymm.ll    = 1;                       /* set ymm.map data length      */
   kcb.seill = 0;                       /* set current seisho length    */
   kcb.semll = 0;                       /* set current sem.map length   */
   kcb.grmll = 0;                       /* set current grammer length   */
   kcb.ymmll = 0;                       /* set current yomi map length  */
}                                       /* end of program               */
