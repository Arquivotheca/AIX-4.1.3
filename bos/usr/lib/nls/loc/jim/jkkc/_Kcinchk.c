static char sccsid[] = "@(#)88	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcinchk.c, libKJI, bos411, 9428A410j 6/4/91 12:52:29";
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
 * MODULE NAME:       _Kcinchk
 *
 * DESCRIPTIVE NAME:  CHECK INPUT & INVOKE PURGE FUNCIONS
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x01ff(Z_ADD1):  1 yomi code is added to the last
 *                    0x02ff(Z_PART):  only the last yomi code is changed
 *                    0x04ff(Z_NEW):   many yomi codes are changed
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
short _Kcinchk(z_kcbptr)                /* check input                  */

struct KCB      *z_kcbptr;              /* base  address of KCB         */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void   _Kcxinia();
   extern void   _Kcrinit();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcype.h"
#include   "_Kcymi.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define    Z_ADD1   0x01ff              /* added one char.              */
#define    Z_PART   0x02ff              /* same length but last char chg*/
#define    Z_NEW    0x04ff              /* changed prev. yomi           */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short      z_i;                      /* define loop counter          */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
   ypeptr1 = kcb.yphype;                /* set address of YCE pool      */
   gpwptr1 = kcb.gpwgpe;                /* set address of GPE pool      */

/*----------------------------------------------------------------------* 
 *       1.Check environment                                         
 *----------------------------------------------------------------------*/
   if(kcb.mode != gpw.pmode)            /* compare current conv. mode   */
   {                                    /* a) not same conv. mode       */
      _Kcxinia( kcbptr1 );              /* init. all ctl blocks         */
      _Kcrinit(kcbptr1);             /* clean up output area         */
      return(Z_NEW);                    /* Initialization error.return  */
   }

   if(gpw.pkakutei != PKAKNON )         /* b) if preceeding call request*/
   {                                    /* firm up                      */
      if(gpw.pkakutei == PKAKALL)
      {
         _Kcxinia(kcbptr1);             /* init. all ctl blocks         */
         _Kcrinit(kcbptr1);             /* clean up output area         */
         return(Z_NEW);
      }
      else
      {
         _Kcrinit(kcbptr1);             /* clean up output area         */
         gpw.accfirm = 0;
      }
   }

/*----------------------------------------------------------------------* 
 *       2.Check yomi data                                            
 *----------------------------------------------------------------------*/
   ymiptr1 = kcb.ymiaddr;               /* set addr of input yomi date  */

   if((kcb.ymill1 > kcb.yphacype+1)||   /* if increase more than 1 mora */
      (kcb.ymill1 < kcb.yphacype))      /* or decreased mora            */
   {                                    /* then                         */
      _Kcxinia(kcbptr1);                /* init. all ctl blocks         */
      _Kcrinit(kcbptr1);             /* clean up output area         */
      return(Z_NEW);                    /* return                       */
   }                                    /* when not last char.          */
                   
   for(z_i=0;z_i < kcb.ymill1-1;z_i++,ypeptr1++)
                                        /* compare input yomi & prev    */
   {
     if(ype.prev != ymi.yomi[z_i])/*if defferent from previous */
      {                                 /* then                         */
         _Kcxinia(kcbptr1);             /* init. all ctl blocks         */
         _Kcrinit(kcbptr1);             /* clean up output area         */
         return(Z_NEW);                 /* return                       */
      }                                 /* endif(ype.prev != ymi.yomi[z_i])*/
   }                                    /* loop next yomi               */

   if(kcb.ymill1==kcb.yphacype+1)
   {
       return(Z_ADD1);                  /*return as added only one char.*/
   }
   else
   {
      if(ype.prev != ymi.yomi[z_i])/* if defferent from previous   */
      {                                 /* last char.                   */
         return(Z_PART);                /* return last char changed     */
      }
   }

   return( SUCCESS );
}
