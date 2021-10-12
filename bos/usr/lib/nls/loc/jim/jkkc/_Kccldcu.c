static char sccsid[] = "@(#)40	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kccldcu.c, libKJI, bos411, 9428A410j 7/23/92 02:57:15";
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
 * MODULE NAME:       _Kccldcu
 *
 * DESCRIPTIVE NAME:  CLOSE USER DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *
 * CATEGORY:          Unique
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kccldcu( z_mcbptr )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *      SET BASE POINTER                                                 
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* pointer of MCB               */

/*----------------------------------------------------------------------*
 *      INITIALIZE INFORMATION ABOUT USER DICTIONARY                         
 *----------------------------------------------------------------------*/
   *mcb.dusnm  = 0x00;                  /*    in the MCB                */
   mcb.dusnmll =    0;                  /*                              */

/*----------------------------------------------------------------------*
 *      FREE MEMORY AREA
 *----------------------------------------------------------------------*/
   mcb.mdemde = NULL;
   mcb.uxeuxe = NULL;

   mcb.dusfd   =   -1;               /* reset user dictionary        */

   return( SUCCESS );
}
