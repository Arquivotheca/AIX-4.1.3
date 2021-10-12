static char sccsid[] = "@(#)37	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kccldc0.c, libKJI, bos411, 9428A410j 7/23/92 02:53:46";
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
 * MODULE NAME:       _Kccldc0
 *
 * DESCRIPTIVE NAME:  CLOSE DICTIONARY, MAINLINE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x0281(SYS_CLOSE): error of close() with system dict
 *                    0x0282(USR_CLOSE): error of close() with user dict
 *                    0x0284(FZK_CLOSE): error of close() with fuzoku-go dict
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


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kccldc0( z_mcbptr, mode )
struct  MCB     *z_mcbptr;              /* pointer of MCB               */
short   mode;                           /* Dicrionary Closed mode       */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE                                       
 *----------------------------------------------------------------------*/
   extern _Kccldcs();                   /* to close system dictionaly   */
   extern _Kccldcu();                   /* to close user   dictionaly   */
   extern _Kccldcf();                   /* to close fuzokugo gakusyu    */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES                                           
 *----------------------------------------------------------------------*/
   short        z_rt;                   /* return code to caller        */
   short        z_rcldcs;               /* return code from _Kccldcs()  */
   short        z_rcldcu;               /* return code from _Kccldcu()  */
   short        z_rcldcf;               /* return code from _Kccldcf()  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS                                            
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'ty to mcb  */

   z_rt = SUCCESS;                      /* initialize return code       */

if( mode >= SYS_CLOSE ) {
/*----------------------------------------------------------------------*
 *      CLOSE STSTEM DICTIONARY                                          
 *----------------------------------------------------------------------*/
   if ( ( z_rcldcs = _Kccldcs( mcbptr1 ) ) != SUCCESS )
   {
      z_rt = z_rcldcs;                  /* error of close()             */
   }
}

if( mode >= USR_CLOSE ) {
/*----------------------------------------------------------------------*
 *      CLOSE USER DICTIONARY                                          
 *----------------------------------------------------------------------*/
   if ( ( z_rcldcu = _Kccldcu( mcbptr1 ) ) != SUCCESS )
   {
      z_rt = z_rcldcu;                  /* error of close()             */
   }
}

if( mode >= FZK_CLOSE ) {
/*----------------------------------------------------------------------*
 *      CLOSE FUZOKU-GO DICTIONARY                                          
 *----------------------------------------------------------------------*/
   if ( ( z_rcldcf = _Kccldcf( mcbptr1 ) ) != SUCCESS )
   {
      z_rt = z_rcldcf;                  /* error of close()             */
   }
}

/*----------------------------------------------------------------------*
 *      RETURN                                          
 *----------------------------------------------------------------------*/
   return( z_rt );
}
