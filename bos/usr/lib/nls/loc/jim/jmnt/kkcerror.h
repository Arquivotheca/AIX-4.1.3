/* @(#)77	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kkcerror.h, libKJI, bos411, 9428A410j 6/6/91 14:32:39 */

/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kkcerror.h
 *
 * DESCRIPTIVE NAME:    Kana Kanji Routine Error Code Values.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Macro.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
#ifndef _kj_kkcerror
#define _kj_kkcerror
/*
 *      DBCS Editor Return Error Code.
 */
#define KKSUCC   (     0      )/* KKC open succeeded.                     */
#define KK_BIAS  (   -4000    )/* KKC Error Code Bias.                    */
#define KK_BIASW (   -400     )/* KKC Warning Level Error Code Bias.      */
#define KKMALOCE (KK_BIAS-0   )/* Memory allocation error.                */
#define KKSYDCOE (KK_BIAS-1   )/* System dictionary open error.           */
#define KKFZDCOE (KK_BIAS-2   )/* Adjunct dictionary open error.          */
#define KKUSDCOE (KK_BIAS-3   )/* User dictionary open error.             */
#define KKUSDCNE (KK_BIAS-4   )/* User dictionary does not exist.         */
#define KKFATALE (KK_BIAS-5   )/* KKC  Fatal Error Occure.                */
#endif
