/* @(#)55	1.2 6/4/91 10:25:38 */

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
 * MODULE NAME:       _Kcfae.h
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
/* Fuzokugo Attribute table Entry (FAE) format                            */
/*                                                                        */
/* FAE entry numbers (0-) are also referred to as FUZOKUGO Numbers        */
/**************************************************************************/
 struct FAE *faeptr1;                   /* FAE base, primary              */
 struct FAE *faeptr2;                   /* FAE base, secondary            */

#define fae  (*faeptr1)
#define fae2 (*faeptr2)
