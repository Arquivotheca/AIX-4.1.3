static char sccsid[] = "@(#)38	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kccldcf.c, libKJI, bos411, 9428A410j 7/23/92 02:55:08";
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
 * MODULE NAME:       _Kccldcf
 *
 * DESCRIPTIVE NAME:  CLOSE FUZOKUGO GAKUSYU FILE
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
short   _Kccldcf( z_mcbptr )

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
 *      INITIALIZE INFORMATION ABOUT FUZOKU-GO DICTIONARY   
 *----------------------------------------------------------------------*/
   *mcb.dfznm   = 0x00;                 /*    in the MCB                */
   mcb.dfznmll =    0;                  /*                              */

/*----------------------------------------------------------------------*
 *      FREE MEMORY AREA
 *----------------------------------------------------------------------*/
   mcb.dfgdfg = NULL;

   return( SUCCESS );
}
