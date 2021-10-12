/* @(#)72	1.2 6/4/91 12:49:15 */

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
 * MODULE NAME:       _Kcfry.h
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
/* Fuzokugo Reversed Yomi table (FRY)                                     */
/**************************************************************************/
 struct FRY *fryptr1;                   /* FRY base, primary              */
 struct FRY *fryptr2;                   /* FRY base, secondary            */

#define fry      (*fryptr1)
#define fry2     (*fryptr2)
