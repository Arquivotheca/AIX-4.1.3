/* @(#)75	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kkcdef.h, libKJI, bos411, 9428A410j 6/6/91 14:32:24 */

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
 * MODULE NAME:         kkcdef.h
 *
 * DESCRIPTIVE NAME:    Kana Kanji Control Block Constant Values.
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
#ifndef _kj_kkcdef
#define _kj_kkcdef
#define K_WLCON  (      0     )/* Word Level Conversion.                  */
#define K_SPLCON (      1     )/* Single phrase level conversion.         */
#define K_MPBCON (      2     )/* Multi phrase level batch                */
#define K_MPACON (      4     )/* Multi phrase level automatic conversion.*/
#define K_OFCON  (      6     )/* On the fly conversion.                  */
#define K_ALPCON (    0x0f    )/* Alpha Numeric conversion.               */
#define K_RYACON (    0x10    )/* Ryakusyou conversion.                   */
#define K_KANCON (    0x11    )/* Kanji number(Kansuuji) conversion.      */
/*
 *      Kanji Conversion Map.
 */
#define M_KJMNCV (    0x00    )/* Yomi.                                   */
#define M_KJMCTN (    0x01    )/* Continuation.                           */
#define M_KJMALN (    0x0f    )/* Alphabet/Numeric.                       */
#define M_KJMABB (    0x10    )/* Abbribate.                              */
#define M_KJMKNM (    0x11    )/* Kanji number.                           */
#define M_KJMJRT (    0x12    )/* Jiristugo.                              */
#define M_KJMJAN (    0x13    )/* Adjust without homonym.                 */
#define M_KJMJAH (    0x14    )/* Adjust with homonym.                    */
#define M_KJMAAN (    0x15    )/* All adjust without homonym.             */
#define M_KJMAAH (    0x16    )/* All adjust with homonym.                */
#define M_KJMPRF (    0x17    )/* Prefix.                                 */
#define M_KJMSUF (    0x18    )/* Suffix.                                 */
#define M_KJMNPF (    0x19    )/* Numeric prefix.                         */
#define M_KJMNSF (    0x1a    )/* Numeric suffix,                         */
#define M_KJMRES (    0x1b    )/* **** RESERVED FOR FUTURE USE ****       */
#define M_KJMPSF (    0x1c    )/* Proper noun suffix.                     */
/*
 *      Kana Kanji Conversion Status.
 */
#define M_KSNCNV (    0x00    )/* Non-conversion.                         */
#define M_KSCNSK (    0x11    )/* Single Kanji conversion.                */
#define M_KSCNVK (    0x10    )/* Kanji.                                  */
#define M_KSCNVY (    0x20    )/* Yomi.                                   */
#define M_KSCNSY (    0x21    )/* Single Kanji yomi.                      */
#define M_KSCNUM (    0x30    )/* Numeric.                                */
/*
 *      Character Class.( 7Bit yomi code for MKK).
 */
#define M_HIRA   (    0x00    )/* Hiragana.                               */
#define M_ALPHA  (    0x01    )/* Alphabet.                               */
#define M_KANA   (    0x02    )/* Katakana.                               */
#define M_KUTEN  (    0x03    )/* Kuten.                                  */
#define M_DOKU   (    0x04    )/* Dokuten.                                */
#define M_SYM1   (    0x05    )/* Symbol 1.                               */
#define M_SYM2   (    0x06    )/* Symbol 2.                               */
#define M_SYM3   (    0x07    )/* Symbol 3.                               */
#define M_SYM4   (    0x08    )/* Symbol 4.                               */
#define M_LEFTP  (    0x09    )/* Left  Bracket.                          */
#define M_RIGHTP (    0x0a    )/* Right Bracket.                          */
#define M_DELM   (    0x0b    )/* Delimiter                               */
#define M_NUM    (    0x0c    )/* Numeric                                 */
#define M_OTHER  (    0xff    )/* Other.                                  */
/*
 *      7bit yomi code for MKK.
 */
#define M_NESC   (    0x1d    )/* 7 bit ESC code for Alphnum Conversion.  */
/*
 *      Length of Dictionary Registration Limit.
 */
#define M_RGYLEN (     20     )/* Maixmum length of Yomi String.          */
#define M_RGIFL  (     40     )/* Maximum length of DBCS String for       */
                               /* Dictionary Registration.                */

#define K_EXTALL (      1     )/* All kana data have to be converted.     */
#define K_EXTNAL (      0     )/* Not all kana data have to be convered.  */

#define K_INTALL (      0     )/* All converted data should be returned   */
                               /* at once.                                */
#define K_INTEAC (      1     )/* Each converted data should be returned  */
                               /* repectively.                            */
/*
 *      KKC Error Level.
 */
#define KKCMAJOR(rc) ((unsigned short)rc & 0xff)
#define KKCMINOR(rc) ((unsigned short)rc>>8)
/*
 *      KKC Phigical Error Code Check.
 */
#define KKCPHYER(rc) ((KKCMAJOR(rc) & K_KCPZE)==K_KCPZE)
/*
 *      KKC Major Error Code.
 */
#define K_KCSUCC (   0x00     )/* Normal.                                 */
#define K_KCNMCA (   0x02     )/* No more candidate (page end).           */
#define K_KCNFCA (   0x04     )/* Candidate not found.                    */



/* #(B) 1987.12.15. Flying Conversion Add */
#define K_KCFLYC (   0x06     )/* Flying Conversion.                      */
/* #(E) 1987.12.15. Flying Conversion Add */



#define K_KCLOGE (   0x08     )/* Logical error.                          */
#define K_KCDCTE (   0x10     )/* Dictionary Registration Failier.        */
#define K_KCPZE  (   0x80     )/* Phisycal error on KKC.                  */
#define K_KCSYPE (   0x81     )/* Physical error on System Dictionary.    */
#define K_KCUSPE (   0x82     )/* Physical error on User   Dictionary.    */
#define K_KCFZPE (   0x84     )/* Physical error on Adjunct Dictionary.   */
#define K_KCMALE (   0x88     )/* Memory allocation error.                */
#define K_KCDPSE (    -1      )/* Entry already exist in sysdict.         */
#define K_KCDPUE (    -1      )/* Entry already exit in userdict.         */
#define K_KCOBOE (    -1      )/* Output buffer overflow.                 */
#define K_KCSDFE (    -1      )/* System dictionary is full.              */
#define K_KCUDFE (    -1      )/* User dictionary is full.                */
#define K_KCUDFE (    -1      )/* User dictionary is full.                */
/*
 *      KKC Minor Error Code.
 */
#define K_KLOPEN (   0x01     )/* Function Open.                          */
#define K_KLCLOS (   0x02     )/* Function Close.                         */
#define K_KLALOC (   0x03     )/* Function Malloc.                        */
#define K_KLFREE (   0x04     )/* Function Free.                          */
#define K_KLSEEK (   0x05     )/* Function Lseek.                         */
#define K_KLREAD (   0x06     )/* Function Read.                          */
#define K_KLWRIT (   0x07     )/* Function Write.                         */
#define K_KLLOCK (   0x08     )/* Function Lockf.                         */
#define K_KLSHMP (   0x09     )/* Function Shmap.                         */
#endif
