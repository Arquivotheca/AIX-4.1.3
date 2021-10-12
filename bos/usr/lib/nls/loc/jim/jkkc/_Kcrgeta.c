static char sccsid[] = "@(#)78	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrgeta.c, libKJI, bos411, 9428A410j 6/4/91 16:56:10";
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
 * MODULE NAME:       _Kcrgeta
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       code of grammar
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
uschar _Kcrgeta(z_type)

uschar     z_type;
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   uschar   z_ret;

/*----------------------------------------------------------------------*
 *      RETURN GRAMMAR BY TYPE
 *----------------------------------------------------------------------*/
   switch(z_type)
   {
      case TP_IPPAN:
      case TP_KOYUU:
      case TP_SUUCHI:
      case TP_USERDCT:
         z_ret = GR_JRT;
         break;

      case TP_SETTO:
      case TP_OSETTO:
      case TP_GSETTO:
         z_ret = GR_PREFIX;
         break;

      case TP_SETSUBI:
         z_ret = GR_SUFFIX;
         break;

      case TP_NSETTO:
         z_ret = GR_NUM_PREFIX;
         break;

      case TP_JOSUSHI:
         z_ret = GR_NUM_SUFFIX;
         break;

      case TP_PSETTO:
         z_ret = GR_PRO_PREFIX;
         break;

      case TP_PSETSUBI:
         z_ret = GR_PRO_SUFFIX;
         break;

      case TP_TOUTEN:
      case TP_KUTEN:
      case TP_CONVKEY:
      default:
         z_ret = 127;
         break;
   }

   return( z_ret );
}
