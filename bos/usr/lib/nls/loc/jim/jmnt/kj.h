/* @(#)67	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kj.h, libKJI, bos411, 9428A410j 6/6/91 14:30:05 */

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
 * MODULE NAME:         kj.h
 *
 * DESCRIPTIVE NAME:    Kanji Project General Constant Defines and
 *                      Internal Subroutine Linkage Constant Defines.
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
#ifndef _kj_kjdef
#define _kj_kjdef
#include "kjmacros.h"         /* Kanji Project Macros.                    */
#include "kjdebug.h"          /* Kanji Project Debugging.                 */
#include "kjextern.h"         /* Kanji Project External Define List.      */
#include "kjcommon.h"         /* Kanji Project Common Define.             */
#include "kjerror.h"          /* Kanji Project Error Code Define.         */
#include "kjcodtab.h"         /* Keyboard & Information Interchange Code. */
#include "degsl.h"            /* DBCS Editor GSL Interface.               */
#include "kmacttab.h"         /* Kanji Monitor Action Trigger Code.       */
#include "kmmsg.h"            /* Kanji Monitor Message Data.              */
#include "dbcs.h"
/*
 **************************************************************************
 *                                                                        *
 *      DBCS Editor Define Start.                                         *
 *                                                                        *
 **************************************************************************
 */
/*
 *      GSL Output File Desctiptor.
 */
#define E_MAXSTR (      1     )/* Maximum Row of DBCS input field.        */
                               /* ***** Not Support Mulitple Row *****    */
/*
 *      Auxiliary Area Control Define.
 */
#define E_AX1RTC (     50     )/* Percentage for Aux. area columns /      */
                               /* display maximum columns.                */
#define E_AX1RTR (     50     )/* Percentage for Aux. area lines /        */
                               /* display maxmum lines.                   */
#define E_AXBXH  (      2     )/* Number of Auxiliary Area No.1 Outline   */
                               /* Character in Row.  (unit by Char).      */
#define E_AXBXW1 (   C_DBCS   )/* Number of Auxiliary Area No.1 Outline   */
                               /* Character in Coumns(unit by Char).      */
#define E_AXBXW2 ( C_DBCS*E_AXBXH )
                               /* Number of Auxiliary Area NO.1 Outline   */
                               /* Character in Column (unit by byte).     */
#define E_AXLNH2 (      2     )/* Multipler for Aux. area.                */
#define E_AXLNW  (  C_DBCS*E_AXLNH2 )
                               /* Multipler for Aux. area.                */
#define E_AXLNH1 (      1     )/* Multipler for Aux. area.                */
#define E_AXQUT  (      2     )/* Divisor for Aux. area. (Center location */
                               /* calculation)                            */
#define E_GAP    (     20     )/* Distance from cursor to aux. area top.  */
#define E_LSPACE (      2     )/* Number of Pixel in Rows Spacing.        */
/*
 *      Maximum Number of DBCS Editor Trigger Action from Kanji Monitor.
 */
#define E_MAXEXE (     16     )/* Maximum number of Trigger Number.       */
/*
 **************************************************************************
 *                                                                        *
 *      Kanji Monitor Define Start.                                       *
 *                                                                        *
 **************************************************************************
 */
/*
 *      Number of Input Field Available Row.
 */
#define M_MAXSTR (      1     )/* Number of rows.                         */
/*
 *      Mimimum Number of Input Field Column.
 */
#define M_MNMXSC (      1     )/* Minimum value of maxstc.                */
/*
 *      Kanji Number(JIS Code) Work Area Size.
 */
#define M_MAXKJN ( C_DBCS*5+1 )/* Maximum number of Kanji No. Input       */
                               /* Buffer.                                 */
/*
 *      DBCS Chractger Code One Segment.
 */
#define M_TKNJN  ( C_SFTSEG+1 )/* JIS Kanji code one segment + 1          */
                               /* Contains X'7F'.                         */
/*
 *      Failier of DBCS Table Index.
 */
#define M_UNDEF  (     -1     )/* Undefined Double byte code.             */
/*
 *      DBCS Character Class.
 */
#define M_YMKANA (      1     )/* Kana Data.                              */
#define M_YMALNM (      2     )/* Alphabet/Numeric Data.                  */
#define M_YMIMPO (      3     )/* Non-Conversion Data.                    */
/*
 *      Kanji Monitor Internal Subroutine Internal Subroutine Interface.
 */
/*      Module : kmmvch
 *      Memory Move Operation Mode.
 */
#define M_FORWD  (      0     )/* Direction of character movement.        */
                               /* (Forward)                               */
#define M_BACKWD (      1     )/* Direction of character movement.        */
                               /* (Backward)                              */
/*      Module : kmkjgrst
 *      Kana/Kanji Conversion Routine Interface Data Select.
 */
#define M_CCDATA (      0     )/* Current Conversion Status Use.          */
#define M_1SDATA (      1     )/* First Conversion Data Use.              */
/*      Module : kmindset
 *      Indicator Modify Mode.
 */
#define M_INDB   (      0     )/* To set both sides indicator.            */
#define M_INDR   (      1     )/* To set right indicator.                 */
#define M_INDL   (      2     )/* To set left indicator.                  */
/*      Module : kmecho
 *      Editing Mode.
 */
#define M_NORMAL (      0     )/* Continuation input mode.                */
#define M_EDITIN (      1     )/* Character input status at Edit mode.    */
/*      Module : kmdisv
 *      Field Message Save Mode.
 */
#define M_SVIF   (    0x00    )/* Save All of Input Field Data.           */
#define M_NSVIF  (    0x01    )/* Save All of Input Field Data,exclude    */
                               /* Input Field String & Hilighting.        */
/*      Module : rkc
 *      Hiragana/Katakana Mode Flag.
 */
#define M_DBKATA (  K_ST1KAT   )/* Double byte KATAKANA mode.             */
#define M_DBHIRA (  K_ST1HIR   )/* Double byte HIRAGANA mode.             */
/*      Module :  kmhtdc
 *      Signle Byte Data Code Interchange for DBCS Data Code Conversion
 *      Mode.
 */
#define M_SFTANK (  K_ST1AN   )/* DBCS Alphabet/Numeric Mode.             */
#define M_SFTKAT (  K_ST1KAT  )/* DBCS Katakana Mode.                     */
#define M_SFTHIR (  K_ST1HIR  )/* DBCS Hiragana Mode.                     */
/*      Module :  kmkcnxpr
 *      Next/Previous Conversion Flag.
 */
#define M_KKCNXT (      1     )/* Next Conversion.                        */
#define M_KKCPRE (      2     )/* Previous Conversion.                    */
/*      Module :  kmrest
 *      Lock Status Clear Mode.
 */
#define M_ALLRST (      0     )/* To reset appropriate Mode.              */
#define M_KLRST  (      1     )/* keyboard lock reset Mode.               */
#define M_MSGRST (      2     )/* Message reset Mode.                     */
#define M_RIRST  (      3     )/* Insert/replace reset Mode.              */
#define M_RGRST  (      4     )/* Registration reset Mode.                */
/*      Module :  kmkanasd
 *      7bit yomi code String for MKK Interchange to DBCS String.
 */
#define M_HIRACV (    0x01    )/* Hiragana Mode.                          */
#define M_KATACV (    0x02    )/* Katakana Mode.                          */
#define M_NUMCV  (    0x03    )/* Numeric Mode.                           */
#define M_ALPHCV (    0x04    )/* Alphabet Mode.                          */
/*      Module : kmR_rtn
 *      RKC Interface Mode.
 */
#define M_RKCON  (    0x01    )/* RKC ordinary process.                   */
#define M_RKCOFF (    0x02    )/* "n" code process.                       */
/*      Module  : kmhrktsw
 *      DBCS Hiragana String Data Interchange DBCS Katakana Data.
 */
#define M_HKALPH (    0x00    )/* AlphaNumeric/Symbol.                    */
#define M_HKHIRA (    0x01    )/* Hiragana.                               */
#define M_HKKATA (    0x02    )/* Katakana.                               */
#define M_NOHRKT (    0x10    )/* Alpha/Numeric/Symbol.                   */
#define M_HRKTMX (    0x20    )/* Hira/Kata/Alpha/Symbol.                 */
#define M_KTALL  (    0x30    )/* Katakana/Alphabet/Symbol.               */
/*
 *      Diagnosis Routine Error Return.
 */
#define IMDGIVK  (      0     )/* Invalid key is input in case of         */
                               /* Diagnostic mode.                        */
/*
 *      Module  :kmhtdc
 *      Signal Byte Character to DBCS Character Convertion Routine
 *      Internal Error Code.
 */
#define IMHTDOK  (      0     )/* Successful.                             */
#define IMHTDCAN (     -1     )/* Invalid input parameter.                */
#define IMHTDERR (     -2     )/* Unsuccessful.                           */
/*      Module  :kmktec
 *      DBCS Kana String to Signle Byte Alphabet Code String Convertion.
 *      Internal Table Error Code.
 */
#define IMKTEERR (    0xff    )/* Invalid Code.                           */
#endif
