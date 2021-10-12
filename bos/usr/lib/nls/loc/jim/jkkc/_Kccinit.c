static char sccsid[] = "@(#)34	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccinit.c, libKJI, bos411, 9428A410j 6/4/91 10:18:40";
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
 * MODULE NAME:       _Kccinit
 *
 * DESCRIPTIVE NAME:  INIT CONTROL BLOCK STRUCTURE
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
#define    NEGA   -1
 
/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
void   _Kccinit( z_chhptr, z_cheptr, z_elsize, z_eleno )
 
struct CHH *z_chhptr;                   /* address of CHH on which the  */
                                        /*   cb structure is to be built*/
unsigned char   *z_cheptr;              /* address of CHE pool          */
short   z_elsize;                       /* element size in bytes        */
short   z_eleno;                        /* no. of elements to be        */
                                        /*  included in the cb structure*/
{
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"
#include   "_Kcche.h"
 
/*----------------------------------------------------------------------*
 * LOCAL VARIABLE
 *----------------------------------------------------------------------*/
   short   z_rc;                        /* return code save area        */
   short   z_i;
 
/*----------------------------------------------------------------------*
 * FORMAT THE CHE POOL INTO ELEMENTS
 *----------------------------------------------------------------------*/
   chhptr1 = z_chhptr;               /* set base address of CHH      */
   chh.fre = 0;                              /* complete th CHH      */
   chh.chepool = (uschar *)z_cheptr;
   chh.act = NEGA;
   chh.size = z_elsize;
   chh.mxele = z_eleno;
   chh.actv = 0;
 
   for ( z_i = 0; z_i < z_eleno; z_i++ )
   {
      cheptr1 = CHEPTR(z_i);
      che.actv = 0;                  /* complete the CHE             */
      che.top = 0;                   /* set top-flag off             */
      che.chef = z_i+1;
      che.cheb = 0;
 
      if ( z_i == (z_eleno-1) )      /* set last-flg on if the elem  */
      {
         che.last = 1;               /* is the last element          */
         che.chef = NEGA;
      }
      else                           /* set last-flg off if the elem */
         che.last = 0;               /* is the middle element        */
 
      che.rsv01 = 0x00;
   }
 
/*----------------------------------------------------------------------*
 * RETURN
 *----------------------------------------------------------------------*/
}
