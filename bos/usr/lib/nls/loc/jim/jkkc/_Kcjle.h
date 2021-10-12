/* @(#)96	1.2 6/4/91 12:53:56 */

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
 * MODULE NAME:       _Kcjle.h
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
/************************************************************************/
/* Jiritsugo Long-word table Entry (JLE)                                */
/************************************************************************/
 struct JLE *jleptr1;                   /* JLE base, primary            */
 struct JLE *jleptr2;                   /* JLE base, secondary          */

#define jle      (*jleptr1)
#define jle2     (*jleptr2)
