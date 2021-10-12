/* @(#)66	1.2 6/4/91 12:48:26 */

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
 * MODULE NAME:       _Kcfkj.h
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
/* Fuzokugo KanJi hyoki table (FKJ)                                       */
/**************************************************************************/
union  FKJ *fkjptr1;                    /* FKJ (entry) pointer            */
                                        /* 06/07/87 add * by c.o          */
union  FKJ *fkjptr2;                    /* FKJ (entry) pointer            */

#define fkj  (*fkjptr1)
#define fkj2 (*fkjptr2)
