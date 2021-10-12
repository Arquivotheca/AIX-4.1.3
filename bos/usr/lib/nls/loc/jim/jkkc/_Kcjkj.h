/* @(#)94	1.2 6/4/91 12:53:36 */

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
 * MODULE NAME:       _Kcjkj.h
 *
 * DESCRIPTIVE NAME:  JIRITUSGO KANJI TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/************************************************************************/
/* Jiritsugo KanJi hyoki table entry (JKJ)                              */
/************************************************************************/
 struct JKJ *jkjptr1;                   /* JKJ base, primary            */
 struct JKJ *jkjptr2;                   /* JKJ base, secondary          */

#define jkj      (*jkjptr1)
#define jkj2     (*jkjptr2)
