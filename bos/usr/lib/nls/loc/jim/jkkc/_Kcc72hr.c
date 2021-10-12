static char sccsid[] = "@(#)24	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcc72hr.c, libKJI, bos411, 9428A410j 6/4/91 10:15:46";
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
 * MODULE NAME:       _Kcc72hr
 *
 * DESCRIPTIVE NAME:  EXCHANGE YOMI CODE TO DBCS HIRAKANA
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       DBCS Hirakana code
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
short  _Kcc72hr(z_yomi)
uschar     z_yomi;
{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_D_CHO    0x815B
#define    Z_D_HDK    0x814B
#define    Z_D_DKT    0x814A
#define    Z_D_HR     0x8280

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   switch(z_yomi)
   {
      case Y_CHO:                       /* chouon                       */
         return( Z_D_CHO );

      case Y_HDK:                       /* handukuon                    */
         return( Z_D_HDK );

      case Y_DKT:                       /* dukuon                       */
         return( Z_D_DKT );

      default :                         /* else                         */
         return( (short)z_yomi | Z_D_HR );
   }
}
