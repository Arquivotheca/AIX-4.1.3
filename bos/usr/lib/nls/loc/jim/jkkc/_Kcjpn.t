/* @(#)98	1.2 6/4/91 12:54:15 */

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
 * MODULE NAME:       _Kcjpn.t
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
/************************************************************************/
/* _Jiritsu-go _Pe_Nalty table					*/
/*                                                                      */
/*    The JPN is a table of jiritsu-go own's penalty.                   */
/************************************************************************/
/************************************************************************/
                               /* 0   1   2   3   4   5   6   7 */
static uschar jpnlty[15][8] = {
/*                                D   C       B       A  MRU  0  */
/*                                0,  1,  2,  3,  4,  5,  6,  7  */
/* UNDEF                   0 */  50, 50, 50, 50, 50, 50, 50, 50,
/* IPPAN                   1 */  19, 16, 14, 13, 11, 10,  8,  0,
/* SETTO                   2 */  14, 11, 10,  9,  8,  7,  5,  5,
/* OSETTO                  3 */   5,  5,  5,  5,  5,  5,  5,  0,
/* GSETTO                  4 */   5,  5,  5,  5,  5,  5,  5,  0,
/* SETSUBI                 5 */  23, 20, 19, 18, 17, 16, 15, 15,
/* SUUCHI                  6 */  19, 16, 14, 13, 11, 10,  8,  0,
/* NSETTO                  7 */   9,  7,  7,  7,  7,  7,  5,  5,
/* JOSUSHI                 8 */  18, 16, 16, 16, 16, 16, 15, 15,
/* KOYUU                   9 */  19, 16, 14, 13, 11, 10,  8,  0,
/* PSETTO                 10 */  13, 10,  9,  8,  7,  6,  5,  5,
/* PSETSUBI               11 */  19, 16, 15, 14, 13, 12, 11, 11,
/* TOUTEN                 12 */   0,  0,  0,  0,  0,  0,  0,  0,
/* KUTEN                  13 */   0,  0,  0,  0,  0,  0,  0,  0,
/* CONVKEY                14 */   0,  0,  0,  0,  0,  0,  0,  0,
};
