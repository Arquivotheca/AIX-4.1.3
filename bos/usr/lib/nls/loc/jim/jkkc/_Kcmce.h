/* @(#)48	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmce.h, libKJI, bos411, 9428A410j 7/23/92 03:16:28	*/

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcmce.h
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/**************************************************************************/
/* Mora Code table Entry (MCE)                                            */
/**************************************************************************/
   struct MCE *mceptr1;                 /* MCE pointer, primary           */
   struct MCE *mceptr2;                 /* MCE pointer, secondary         */
   struct MCE *mcetmp;                  /* MCE pointer, tmp               */

#define mce  (*mceptr1)
#define mce2 (*mceptr2)
