static char sccsid[] = "@(#)39	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kccldcs.c, libKJI, bos411, 9428A410j 7/23/92 02:56:10";
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
 * MODULE NAME:       _Kccldcs
 *
 * DESCRIPTIVE NAME:  CLOSE SYSTEM DICTIONARY
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
short   _Kccldcs( z_mcbptr )

struct  MCB     *z_mcbptr;              /* pointer of MCB               */
{
    short        z_i;

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------* 
 *       SET BASE POINTER       
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* pointer of MCB               */

/*----------------------------------------------------------------------* 
 *       INITIALIZE INFORMATION OF SYSTEM DICTIONARY      
 *----------------------------------------------------------------------*/
   for( z_i = 0; z_i < mcb.mul_file; z_i++ ) {	/* in the MCB           */
       if( mcb.dsynm[z_i] != NULL ) {   /* Path Name(File Name)		*/
           free( mcb.dsynm[z_i] );
           mcb.dsynm[z_i] = NULL;
       }
       mcb.dsyseg[z_i]  = NULL;         /* Addr of shared mem segment   */
       mcb.sxesxe[z_i]  = NULL;         /* Addr of system dict area     */
       mcb.dsynmll[z_i] = 0;            /* Path Name length             */
   }
   return( SUCCESS );
}
