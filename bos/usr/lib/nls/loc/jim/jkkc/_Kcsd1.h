/* @(#)90	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcsd1.h, libKJI, bos411, 9428A410j 7/23/92 03:17:02	*/

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
 * MODULE NAME:       _Kcsd1.h
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
/* System dictionaly Data (SD1)                                           */
/**************************************************************************/
 struct SD1 *sd1ptr1;                   /* primary   SD1 pointer          */
 struct SD1 *sd1ptr2;                   /* secondary SD1 pointer          */

 struct SDM *sdmptr1;			/* primary   SDM pointer	  */
 struct SDC *sdcptr1;			/* primary   SDC pointer	  */
 struct SDA *sdaptr1;			/* primary   SDA pointer	  */
 struct SDL *sdlptr1;			/* primary   SDL pointer	  */

#define sd1      (*sd1ptr1)
#define sd12     (*sd1ptr2)

#define sdm      (*sdmptr1)
#define sdc      (*sdcptr1)
#define sda      (*sdaptr1)
#define sdl      (*sdlptr1)
