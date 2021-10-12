/* @(#)74	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/kkcb.h, libKJI, bos411, 9428A410j 7/23/92 03:25:49 */
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kkcb.h
 *
 * DESCRIPTIVE NAME:    Kana Kanji Control Block Structure.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Monitor V1.0
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
 * CHANGE ACTIVITY:     06/09/92 add alphakan entry
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
/*
 *      Kana Kanji Control block Structure.
 */
#ifndef _kj_KKCB
#define _kj_KKCB

#include "kjmacros.h"   /* Kanji Project Macros.                          */
#include "kkcdef.h"     /* Kana/Kanji Control Block Define.               */

/*
 *      Create Kana/Kanji Control Block NickName.
 */
typedef struct kkcblock KKCB;

struct kkcblock {
long   length;  /* Length of Kana Kanji Control Block.                    */
                /* KKC Sets this.                                         */

long   id;      /* KKC Module Identifier.                                 */
                /* KKC Sets this.                                         */

uchar  *kanadata;
                /* Pointer to Kana (yomi) data.                           */
                /* KKC sets this.                                         */

uchar  *kjdata; /* Pointer to Kanji(seisho) data.                         */
                /* KKC sets this.                                         */

uchar  *kjmap;  /* Pointer to Kanji (seisho) map.                         */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji Monitor set this at next and        */
                /* previous conversion, and all candidates.               */

uchar  *kanamap;/* Pointer to Kana (yomi) map.                            */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji Monitor set this at next and        */
                /* previous conversion, and all candidates.               */

uchar  *grammap;/* Pointer to Grammer Map                                 */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji MOnitor set this at next and        */
                /* previous conversion, and all candidates.               */

uchar  convmode;/* Conversion mode.                                       */
                /* =0 ... Word Level Conversion.                          */
                /* =1 ... Single Phrase Level Conversion.                 */
                /* =2 ... Multi  Phrase Level Conversion.                 */
                /* =4 ... Multi  Phrase Level automatic Conversion.       */
                /* =6 ... On the fly conversion.                          */
                /* =15 .. "Ryakusho" conversion.                          */
                /* =16 .. Alpha-number conversion.                        */
                /* =17 .. Kanji number(Kansuji) conversion.               */
                /* Kanji Monitor Sets this at conversion mode set.        */

uchar  extconv; /* External Conversion flag.                              */
                /* Kanji Monitor sets this to indicate whether            */
                /* all Kana(yomi) data should be converted or not.        */
                /* =0 ... Not all Kana data have to be converted.         */
                /* =1 ... All Kana data have to be converted.             */
                /* At the first release, Kanji Monitor always             */
                /* sets 1 to it.                                          */

uchar  intconv; /* Internal Conversion flag.                              */
                /* Kanji Monitor sets this to indicate whether            */
                /* KKC should return all converted data at once or        */
                /* each conveted data respectively.                       */
                /* =0 ... All conveted data should be returned at once    */
                /* =1 ... Each Converted data should be returned          */
                /*        respectively.                                   */
                /* This has meaning only when extconv=0. At the           */
                /* first release, Kanji Monitor always sets               */
                /* 0 to intconv anyway.                                   */

uchar  leftchar;/* Class of the left character.                           */
                /* Kanji Monitor Sets this.                               */

uchar  rightchr;/* Class of the right character.                          */
                /* Kanji Monitor Sets this.                               */

uchar  alphakan;/* status of alpha/kansu conv   			  */
		/*  0x01: alpha conv only       			  */
		/*  0x02: kansu conv only       			  */
		/*  0x03: alpha & kansu conv    			  */

short  kanamax; /* Maximum Kana data buffer length(byte).                 */
                /* KKC sets this.                                         */

short  kanalen1;/* Length of Kana (yomi) data.                            */
                /* Kanji Monitor Sets this.                               */

short  kanalen2;/* Length of the first phrase of Kana(yomi) data          */
                /* at the first conversion.                               */
                /* Length of "jiritugo" at the next conversion.           */
                /* Kanji Monitor Sets this.                               */

short  kjmax;   /* Maximum Kanji data buffer length(byte).                */
                /* KKC sets this.                                         */

short  kjlen;   /* Length of Kanji(seisho) data.                          */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji MOnitor set this at next and        */
                /* previous conversion, and all candidates.               */

short  kjmpmax; /* Maximum Kanji map data buffer length(byte).            */
                /* KKC sets this.                                         */

short  kjmapln; /* Length of Kanji(seisho) map.                           */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji MOnitor set this at next and        */
                /* previous conversion, and all candidates.               */

short  knmpmax; /* Maximum Kana map data buffer length(byte).             */
                /* KKC sets this.                                         */

short  knmapln; /* Length of Kana(yomi) map.                              */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji MOnitor set this at next and        */
                /* previous conversion, and all candidates.               */

short  grmpmax; /* Maximum Grammer map data buffer length(byte).          */
                /* KKC sets this.                                         */

short  grmapln ;/* Length of grammer map.                                 */
                /* KKC Sets this at first conversion.                     */
                /* Both KKC and Kanji MOnitor set this at next and        */
                /* previous conversion, and all candidates.               */

short  totalcan;/* Number of all candidates or                            */
                /* singla Kanji candidates.                               */
                /* KKC Sets this at all candidates open.                  */

short  reqcnt;  /* Request Count for all candidates or single Kanji       */
                /* candidates.                                            */
                /* Kanji Monitor sets this at.                            */
                /* all candidates forward/backward or                     */
                /* single Kanji forward/backward.                         */

short  rsnumcnd;/* Number of candidates retured.                          */
                /* KKC sets this at all candidates forward/backward       */
                /* or single Kanji forward/backward                       */

short  candtop; /* Candidate List Top.                                    */

short  candbotm;/* Candidate List End.                                    */

short  maxkjlen;/* Max Legth of Kanji(seisho) in all candidates.          */
                /* KKC sets this at all candidates open.                  */

short  retcode1;/* Return code(1).                                        */
                /* KKC sets this every time besides the function          */
                /* return code.                                           */

short  retcode2;/* Return code(2).                                        */
                /* KKC sets this every time besides the function          */
                /* return code.                                           */

uchar  *rsv[10];/* **** RESERVED FOR FUTURE USE ****                      */

};
#endif
