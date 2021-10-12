/* @(#)26	1.2 6/4/91 12:58:28 */

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
 * MODULE NAME:       _Kckcb.h
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
/**************************************************************************/
/* _Kkc _Control _Block (KCB)					  */
/*                                                                        */
/*    The KCB is a control block which is used to form a Standard KKC     */
/*  interface (OFKKC).  Because the Standard KKC Interface is supposed    */
/*  to be shared by many departments, any modification of the KCB must    */
/*  be made under the agreement of the related departments including      */
/*  IP E/S, TRL and YAMATO.                                               */
/**************************************************************************/
 struct KCB *kcbptr1;                   /* primary   KCB pointer          */

#define kcb      (*kcbptr1)
