/* @(#)56	1.2 6/4/91 10:26:01 */

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
 * MODULE NAME:       _Kcfax.h
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
/* Fuzokugo Attribute table indeX (FAX)                                   */
/*                                                                        */
/* Entry numbers of FAX (0-) corresponds mora codes.                      */
/* The last letter of the fuzokugo currently being processed should be    */
/* used when looking up the Fuzokugo attribute table via this index.      */
/**************************************************************************/
 struct FAX *faxptr1;                   /* FAX base, primary              */
 struct FAX *faxptr2;                   /* FAX base, secondary            */

#define fax      (*faxptr1)
#define fax2     (*faxptr2)
