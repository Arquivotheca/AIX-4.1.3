/* @(#)08	1.2 6/4/91 15:30:11 */

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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcyce.h
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
/*************************************************************************/
/* Yomi Code table Entry (YCE) map                                       */
/*************************************************************************/
struct YCE *yceptr1;                   /* YCE base, primary              */
struct YCE *yceptr2;                   /* YCE base, secondary            */

#define yce      (*yceptr1)
#define yce2     (*yceptr2)
