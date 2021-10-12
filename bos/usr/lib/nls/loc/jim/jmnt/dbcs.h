/* @(#)59	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/dbcs.h, libKJI, bos411, 9428A410j 6/6/91 14:28:00 */

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
 * MODULE NAME:         dbcs.h
 *
 * DESCRIPTIVE NAME:    DBCS Monitor & Kanji Monitor DBCS Constant.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
 *                      DBCS  Monitor V1.0
 *                      Kanji Monitor V1.0
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

/*
 *      Define:Double Byte Character Sets Dependent.
 */
#define E_CHIVXC (     1      )/* DBCS Character Special Coordinate(X)    */
#define E_CHIVYC (     5      )/* DBCS Character Special Coordinate(Y)    */
#define E_ACIVXC (     1      )/* DBCS Character Special Coordinate(X)    */
#define E_ACIVYC (     4      )/* DBCS Character Special Coordinate(Y)    */

#define E_CHLFT  (     0      )/* Most Left Caracter.                     */
#define E_CHNLFT (     1      )/* None Most Left Character.               */

/*
 *      DBCS Editor Use DBCS Character Code.
 *      Which is Outline Character,Indicaor,Cursor,Underline,etc.
 */
#define E_CHNZY0 (   0xfcd4   )/* Spacing Character(Background Character  */
#define E_CHNZY1 (   0xfcd5   )/* Spacing Character(Background Character  */
#define E_CHNZY2 (   0xfcd6   )/* Spacing Character(Background Character  */
#define E_CHNZY3 (   0xfcd7   )/* Spacing Character(Background Character  */
#define E_CHNZY4 (   0xfcd8   )/* Spacing Character(Background Character  */
#define E_CHNZYO (   0xfcd9   )/* Spacing Character(Background Character  */
#define E_CHZEY0 (   0xfcda   )/* Spacing Character.                      */
#define E_CHZEY1 (   0xfcdb   )/* Spacing Character.                      */
#define E_CHZEY2 (   0xfcdc   )/* Spacing Character.                      */
#define E_CHZEY3 (   0xfcdd   )/* Spacing Character.                      */
#define E_CHZEY4 (   0xfcde   )/* Spacing Character.                      */
#define E_CHZEYO (   0xfcdf   )/* Spacing Character.                      */
#define E_CURRN1 (   0xfce0   )/* Replace mode normal type cursor.(1byte) */
#define E_CURRM2 (   0xfce1   )/* Replace mode normal type cursor.(2dot)  */
#define E_CURIN2 (   0xfce2   )/* Insert mode normal type cursor.(2byte)  */
#define E_CURRMS (   0xfce3   )/* Replace mode normal type cursor(1byte)  */
#define E_CURRM1 (   0xfce4   )/* Replace mode normal type cursor.(1dot)  */
#define E_AREVET (   0xfce5   )/* Spacing Character.                      */
#define E_AREVEY (   0xfce6   )/* Spacing Character.                      */
#define E_AREVEX (   0xfce7   )/* Spacing Character.                      */
#define E_AUNDER (   0xfce8   )/* Spacing Character.                      */
#define E_AREAUN (   0xfce9   )/* Spacing Character.                      */
#define E_REVE   (   0xfcea   )/* Character code for reverse.             */
#define E_UNDER  (   0xfceb   )/* Character code for underline letter.    */
#define E_REAUN  (   0xfcec   )/* Character code for reverse &            */
#define E_BOXLLC (   0xfced   )/* Lower Left Corner Character.            */
#define E_BOXLRC (   0xfcee   )/* Lower Right Corner Character.           */
#define E_BOXULC (   0xfcef   )/* Upper Left Corner Character.            */
#define E_BOXURC (   0xfcf0   )/* Upper Right Corner Character.           */
#define E_BOXUHZ (   0xfcf1   )/* Upper Horizonal Chrracter.              */
#define E_BOXLHZ (   0xfcf2   )/* Lower Horizonal Character.              */
#define E_BOXLVT (   0xfcf3   )/* Levt  Vertical  Character.              */
#define E_BOXRVT (   0xfcf4   )/* Right Vertical  Character.              */
/*
 *      DBCS Indicator Code.
 */
#define M_IINS   (   0xfcf5   )/* Insert mode.                            */
#define M_IKBLK  (   0xfcf6   )/* Keyboard lock.                          */
#define M_IBLNK  (   0xfcf7   )/* Unsuccessfully conversion.              */
#define M_IROM   (   0xfcf8   )/* Romaji to Kana Conv.                    */
#define M_IAN    (   0xfcf9   )/* Alpha/Numeric.                          */
#define M_IKATA  (   0xfcfa   )/* Katakana.                               */
#define M_IHIRA  (   0xfcfb   )/* Hiragana.                               */

#define M_INCHG  (   0x81a1   )/* Unsuccessful conversion.                */

