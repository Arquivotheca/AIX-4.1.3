/* @(#)79	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kmdef.h, libKJI, bos411, 9428A410j 6/6/91 14:33:03 */

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
 * MODULE NAME:         kmdef.h
 *
 * DESCRIPTIVE NAME:    Kanji Monitor Control Block Constant Values.
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
 *                              DCBID    :Generate Event Control ID.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 27 1988 Satoshi Higuchi
 *                      Added a multiple ahout the work area.
 *
 *                      Sept. 28 1988 Satoshi Higuchi
 *                      Added a value of array size.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
#ifndef _kj_kmdef
#define _kj_kmdef
/*
 *      Kanji Monitor Control Block Identify.
 */
#define K_KCBID  ECBID(C_SYSTEM,C_VER,C_RELESE,C_MAINT)
                               /* Kanji Control Block Identifier.         */
#define K_KCBMSG "**** Kanji Monitor Control Block **** "
                               /* Kanji Control Block Message.            */
/*
 *      Kanji Monitor Internal Save Area Control BLock Identify.
 */
#define K_ISAID  ( ('I'<<24) + ('S'<<16) + ('A'<<8) + ' ' )
                               /* Kanji Monitor Internal Save Area        */
                               /* Identify.                               */
#define K_ISAMSG "**** Kanji Monitor Internal Save Area ****"
                               /* Kanji Monitor Internal Save Area Message*/
/*
 *      DBCS String Hilighting Attribute Mode
 */
#define K_HLAT0  (      0     )/* Normal.                                 */
#define K_HLAT1  (      1     )/* Reverse.                                */
#define K_HLAT2  (      2     )/* Underline.                              */
#define K_HLAT3  (      3     )/* Reverse & Underline.                    */
/*
 *      Special Cursor Position.
 */
#define M_CUROUT (  C_FAUL    )/* No Cursor in Field,or Auxiliary         */
                               /* Area.                                   */
/*
 *      Input Data Type.
 */
#define K_INASCI (      1     )/* Character code. (ASCII)                 */
#define K_INESCF (      2     )/* Control  Sequence Code(ESC).            */
#define K_INBIN  (      3     )/* Binary Data.                            */
/*
 *      Input Field Attribute.
 */
#define K_ODUBYT (      0     )/* Only DBCS String.                       */
#define K_MIXMOD (      1     )/* DBCS String or ANK String.              */
/*
 *      Auxiliary Area Usage Status Flag.
 */
#define K_AINUSE (      1     )/* Auxiliary area is in use.               */
#define K_ANOUSE (      0     )/* Auxiliary area is not in use.           */
/*
 *      Length of Indicator.
 */
#define M_NOINDL (      0     )/* Constant for minimum Indicator length.  */
/*
 *      Shift Status 1 Change Status Flag.
 */
#define K_STNOT  (   0x00     )/* Any shift status unchanged.             */
#define K_STSFT1 (   0x01     )/* shift1 changed.                         */
#define K_STSFT2 (   0x02     )/* shift2 changed.                         */
#define K_STSFT3 (   0x04     )/* shift3 changed.                         */
#define K_STSFT4 (   0x08     )/* shift4 changed.                         */
/*
 *      Shift Status 1 Change Status.
 */
#define K_ST1UDF (      0     )/* Undefined.                              */
#define K_ST1AN  (      1     )/* Alpha-Numeric.                          */
#define K_ST1KAT (      2     )/* Katakana.                               */
#define K_ST1HIR (      3     )/* Hiragana.                               */
/*
 *      Shift Stasus 2 Change Status.
 */
#define K_ST2RON (      1     )/* RKC ON.                                 */
#define K_ST2ROF (      2     )/* RKC OFF.                                */
/*
 *      Shift Status 3 Change Status.
 */
#define K_ST3SIN (      1     )/* Single byte shift.                      */
#define K_ST3DBL (      2     )/* Double byte shift.                      */
/*
 *      Cursor Length.
 */
#define K_C1BYTC (      1     )/* Single byte cursor code.                */
#define K_C2BYTC (      2     )/* Double byte cursor code.                */
/*
 *      Conversion Stage Flag.
 */
#define K_CONVF  (      0     )/* Conversion is finished.                 */
#define K_CONVGO (      1     )/* Conversion is going on.                 */
/*
 *      Insert/Replace Mode Status Flag.
 */
#define K_REP    (      0     )/* Replace mode.                           */
#define K_INS    (      1     )/* Insert mode.                            */
/*
 *      Discard Data or Not Discard Data.
 */
#define K_DISOFF (      0     )/* Discard is OFF.                         */
#define K_DISON  (      1     )/* Discard is ON.                          */
/*
 *      Trace Status Flag.
 */
#define K_TALL   (      1     )/* To trace out all contents to disk.      */
#define K_TLIMIT (      0     )/* To trace over all contents to limited   */
                               /* system memory.                          */
/*
 *      Kana/Kanji Conversion Enable/Disable.
 */
#define K_KKCOFF (      0     )/* Kana/Kanji Conversion Disable.          */
#define K_KKCON  (      1     )/* Kana/Kanji Conversion Enable.           */
/*
 *      Next Action Code.
 *      Specified Bit is ON,then execute Specified Function.
 */
#define M_DAGMON ( 0x80000000 )/* Diagnosis message routine.              */
#define M_DAGSON ( 0x40000000 )/* Diagnosis start routine.                */
#define M_DAGEON ( 0x20000000 )/* Diagnosis end routine.                  */
#define M_KLRSON ( 0x10000000 )/* Keyboard lock reset function.           */
#define M_RGAON  ( 0x08000000 )/* kmRG_a routines.                        */
#define M_RGBON  ( 0x04000000 )/* kmRG_b routines.                        */
#define M_RGCON  ( 0x02000000 )/* kmRG_c routines.                        */
#define M_CNRSON ( 0x01000000 )/* Unsuccessful conv.indicator reset func. */
#define M_RMRSON ( 0x00800000 )/* kmRM_rs routines.                       */
#define M_MGRSON ( 0x00400000 )/* Message reset func.                     */
/*
 *      DBCS Input Field Active/Inactive Status.
 */
#define K_IFACT  (      1     )/* DBCS Input field is active.             */
#define K_IFINAC (      0     )/* DBCS Input field is inactive.           */
/*
 *      Hiragana/Katakana Conversion Enable/Disable Status.
 */
#define K_HKTEN  (      0     )/* Conversion status.                      */
#define K_HKRES  (      1     )/* Reset status.                           */
#define K_HKKAN  (      2     )/* Katakana status.                        */
/*
 *      Dictionary Registration Stage.
 */
#define K_NODIC  (      0     )/* Not Registration Stage.                 */
#define K_REDIC  (      1     )/* Yomi Prompt & Yomi String Input Stage.  */
#define K_KADIC  (      2     )/* DBCS Prompt & DBCS String Input Stage.  */
#define K_MEDIC  (      3     )/* Dictionary Registration Complete Message*/
                               /* Output Stage.                           */
/*
 *      DBCS Character Mode.
 */
#define K_CHHIRA (      1     )/* Hiragana include long vowel character.  */
#define K_CHKATA (      2     )/* Katakana.                               */
#define K_CHALPH (      3     )/* Alphabet.                               */
#define K_CHNUM  (      4     )/* Numeric.                                */
#define K_CHBLNK (      5     )/* Blank.                                  */
#define K_CHKIGO (      6     )/* Other DBCS Character code.              */
#define K_CHBYTE (      7     )/* Single byte character.                  */
/*
 *      Field Save Area ID.
 */
#define K_MSGOTH (    0x80    )/* General Use Area.                       */
#define K_MSGDIC (    0x40    )/* Dictionary Registration Area.           */
/*
 *      All Candidate Signle Kanji(Word) Conversion Stage.
 */
#define K_SGLCOF (    0x00    )/* Not Sigle  Kanji Conversion Stage.      */
#define K_SGLCON (    0x01    )/* Single Kanji Conversion Stage.          */
/*
 *      Conversion Suucessful Status.
 */
#define K_CVIPOF (      0     )/* Unsuccessful conversion flag is OFF.    */
#define K_CVIPON (      1     )/* Unsuccessful conversion flag is ON.     */
/*
 *      All Candidate Display Mode.
 */
#define M_ACIF   (    0x01    )/* To Display DBCS active field.           */
#define M_ACAX1A (    0x02    )/* To Display Auxiliary Area,with multi-row*/
#define M_ACAX1S (    0x03    )/* To Display Auxiliary Area,with signle   */
                               /* row.                                    */
/*
 *      KKC Open Request Finish Status.
 */
#define M_KKNOP  (      0     )/* KKC Not Open.                           */
#define M_KKOPN  (      1     )/* KKC Opened.                             */

/*      Define a multiple about size of the kjdata,kjmap1s and gramap1s
 *      Sept. 27 1988 Added by Satoshi Higuchi
 */
#define M_MULT   (      2     )/* A multiple is two                       */

/*      Kanji Monitor Profile Save Area fixed.
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_PROFSA (      2     )/*Kanji Monitor profile save area array val*/

/*      Number of each all candidates stage fixed.
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_CANDNM (    100     )/* Number of each all candidates stage     */
			       /* array value.                            */

/*      Number of each all candidates single stage fixed
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_SCNDNM (      2     )/* Number of each all candidates single    */
			       /* Kanji stage array value.                */

/*      Pointer ro Romaji/Kana conversion Data fixed
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_CVROKN (      4     )/* Pointer to Romaji/Kana conversion Data  */
			       /* array value.                            */

/*      Dictionary Registration 7bit Kana code work area fixed
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_RGYMA (      12     )/* Dictionary Registration 7bit Kana code  */
			       /* Work Area array value.                  */

/*      Character code fixed
 *      Sept. 28 1988 Added by Satoshi Higuchi
 */
#define M_CHCODE (      4     )/* Character code array value.             */

#endif
