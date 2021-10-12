/* @(#)69	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kjcommon.h, libKJI, bos411, 9428A410j 6/6/91 14:30:40 */

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
 * MODULE NAME:         kjcommon.h
 *
 * DESCRIPTIVE NAME:    Defins Kanji Project Common Constans.
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
 *      Kanji Project Common Defines.
 */
#ifndef _kj_kjcom
#define _kj_kjcom
/*
 *      System Identifier.
 */
#define C_RTPC   (      1    )/* RT-PC System ID(IBM6100 GSL-Kanji).      */
#define C_SYSTEM (  C_RTPC   )/* System Idenfier.                         */
#define C_VER    (      1    )/* Version No.1                             */
#define C_RELESE (      0    )/* Release No.0                             */
#define C_MAINT  (      0    )/* Maintenance No.0                         */
/*
 *      Logical Switch.
 */
#define C_SWOFF  (      0     )/* Logical Value FALSE.                    */
#define C_SWON   (      1     )/* Logical Value TRUE.                     */
/*
 *      Arithmatic Constant.
 */
#define C_INIT   (      0     )/* Initial value Zero.                     */
#define C_ADD    (      1     )/* Incremnt 1.                             */
/*
 *      Number of Character & Type Information.
 */
#define C_ANK    (     1      )/* Single byte code.                       */
                               /* (Alpha/Numeric/Katakana code)           */
#define C_DBCS   (     2      )/* length of DBCS one character.           */
#define C_DCHR   ( C_DBCS*2   )/* length of DBCS double characters.       */
#define C_TRWORD (  C_DCHR    )/* Constant for bytes per word.            */
#define C_BITBYT (      8     )/* Number of bits for one byte.            */
#define C_HIBYTE (     256    )/* Total number of unsigned sigle byte     */
                               /* Member.                                 */
/*
 *      Nibble Mask.
 */
#define C_HHBYTE (    0xf0    )/* Mask code for higher nibble.            */
#define C_LHBYTE (    0x0f    )/* Mask code for lower nibble.             */
/*
 *      Byte Mask.
 */
#define C_BYTEM  (    0xff    )/* Mask code for lower one byte.           */
/*
 *      Unsigned Char Bit Mask Pattern.
 */
#define C_SB01   (    0x01    )/* b'00000001'.                            */
#define C_SB02   (    0x02    )/* b'00000010'.                            */
#define C_SB04   (    0x04    )/* b'00000100'.                            */
#define C_SB08   (    0x08    )/* b'00001000'.                            */
#define C_SB0F   (    0x0f    )/* b'00001111'.                            */
#define C_SB10   (    0x10    )/* b'00010000'.                            */
#define C_SB20   (    0x20    )/* b'00100000'.                            */
#define C_SB40   (    0x40    )/* b'01000000'.                            */
#define C_SB80   (    0x80    )/* b'10000000'.                            */
#define C_SBF0   (    0xf0    )/* b'11110000'.                            */
/*
 *      Unsigned Short Bit Mask Pattern.
 */
#define C_DB0000 (   0x0000   )/* b'0000000000000000'.                    */
#define C_DB0001 (   0x0001   )/* b'0000000000000001'.                    */
#define C_DB0002 (   0x0002   )/* b'0000000000000010'.                    */
#define C_DB0004 (   0x0004   )/* b'0000000000000100'.                    */
#define C_DB0008 (   0x0008   )/* b'0000000000001000'.                    */
#define C_DB0010 (   0x0010   )/* b'0000000000010000'.                    */
#define C_DB0020 (   0x0020   )/* b'0000000000100000'.                    */
#define C_DB0040 (   0x0040   )/* b'0000000001000000'.                    */
#define C_DB0080 (   0x0080   )/* b'0000000010000000'.                    */
#define C_DB0100 (   0x0100   )/* b'0000000100000000'.                    */
#define C_DB0200 (   0x0200   )/* b'0000001000000000'.                    */
#define C_DB0400 (   0x0400   )/* b'0000010000000000'.                    */
#define C_DB0800 (   0x0800   )/* b'0000100000000000'.                    */
#define C_DB1000 (   0x1000   )/* b'0001000000000000'.                    */
#define C_DB2000 (   0x2000   )/* b'0010000000000000'.                    */
#define C_DB4000 (   0x4000   )/* b'0100000000000000'.                    */
#define C_DB8000 (   0x8000   )/* b'1000000000000000'.                    */
/*
 *      Cursor Posotion Control  Value.
 */
#define C_FAUL   (     -1     )/* Cursor Invisible Position.              */
#define C_POS    (      1     )/* Offset/Position Conversion.             */
                               /* Pos <= Offset + C_POS.                  */
#define C_OFFSET (     -1     )/* Position/Offset Conversion.             */
                               /* Offset <= Pos + C_OFFSET.               */
#define C_COL    (      0     )/* Initial column value.                   */
#define C_ROW    (      1     )/* Initial row value.                      */
/*
 *      Processing Area Code.
 */
#define C_INPFLD (      0     )/* DBCS input field.                       */
#define C_AUX1   (      1     )/* Auxiliary Area No.1.                    */
#define C_AUX2   (      2     )/* Auxiliary Area No.2.                    */
#define C_AUX3   (      3     )/* Auxiliary Area No.3.                    */
#define C_AUX4   (      4     )/* Auxiliary Area No.4.                    */
/*
 *      Character Code Specified Define.
 */
#define C_JKCSID (     301    )/* Character set ID.(Japanese Kanji)       */
#define C_JISSEG (     94     )/* JIS Kanji Code character number per     */
                               /* one segment.(0x7e-0x21)                 */
#define C_SFTSEG ( C_JISSEG*2 )/* DBCS Code one segment(X'21'-`X'7E')     */
#define C_PKC1BL (    0x81    )/* PC Kanji Code 1st block lower byte.     */
#define C_PKC1BH (    0x9f    )/* PC Kanji Code 1st block higher byte.    */
#define C_PKC2BH (    0xfc    )/* PC Kanji Code 2nd block higher byte.    */
#define C_PKC2BL (    0xe0    )/* PC Kanji Code 2nd block lower byte.     */
/*
 *      General Yes/No Character Code.
 */
#define C_1CDKMI (    0xd0    )/* Character Code Katakana 'Mi'            */
#define C_1CDKNN (    0xdd    )/* Character Code Kananana 'nn'            */
#define C_1CDLN  (     'N'    )/* Character Code 'N'                      */
#define C_1CDLY  (     'Y'    )/* Character Code 'Y'                      */
#define C_1CDSN  (     'n'    )/* Character Code 'n'                      */
#define C_1CDSY  (     'y'    )/* Character Code 'y'                      */
/*
 *      General Use DBCS Character & Single Character Code.
 */
#define C_BLANK  (     ' '    )/* Single byte blank character code.       */
#define C_SPACE  (   0x8140   )/* Blank code. (PC Kanji Code)             */
#define C_SPACEH (    0x81    )/* Higher one-byte code for blank code.    */
#define C_SPACEL (    0x40    )/* Lower one-byte code for blank code      */
#define C_QUOTO  (   0xfa57   )/* IBM extention Kanji code for double     */
                               /* quotation.                              */
#define C_LIT    (   0xfa56   )/* IBM extention Kanji code for single     */
                               /* quotation.                              */
/*
 *      Trace Control.
 */
#define C_TRFCL  (      1     )/* Process code for Trace File close.      */
#define C_TRFOP  (      0     )/* Process code for Trace File open.       */
#define C_TRIDXZ (      0     )/* Constant for Trace index block check.   */
#define C_TRWBZ  (      0     )/* Constant for Trace block boundary check */
#endif
