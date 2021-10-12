/* @(#)70	1.2 6/4/91 12:49:00 */

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
 * MODULE NAME:       _Kcfpe.h
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
/* Fuzokugo Penalty adjustment table Entry (FPE) format                   */
/**************************************************************************/
 struct FPE *fpeptr1;                   /* FPE pointer, primary           */
 struct FPE *fpeptr2;                   /* FPE pointer, secondary         */

#define fpe  (*fpeptr1)
#define fpe2 (*fpeptr2)

/* sample code:                                                           */
/*                                                                        */
/* fpeptr1 = fle.fpendx;                                                  */
/* if (fpe.hin == xxhin) penalty = penalty + fpe.pen;                     */
/*                                                                        */
