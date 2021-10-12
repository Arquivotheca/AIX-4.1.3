/* @(#)10        1.5 8/27/91 12:19:11  */
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: header file
 *
 * ORIGINS: IBM
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kucommon.h
 *
 * DESCRIPTIVE NAME:    Defines User Dictionary Maintenance Common Contant.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
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
#ifndef _kj_kucom
#define _kj_kucom
/*
 *      Arithmatic Constant.
 */
#define C_INIT   (      0     )/* Initial value Zero.                     */
#define C_ADD    (      1     )/* Incremnt 1.                             */
/*
 *      Number of Character Set Byte Size.
 */
#define C_ANK    ((int)sizeof(char))
                               /* Single byte code.                       */
                               /* (Alpha/Numeric/Katakana code)           */
#define C_DBCS   ((int)sizeof(short))
                               /* length of DBCS one character.           */
#define C_DCHR   ( C_DBCS*2   )/* length of DBCS double characters.       */
#define C_TRWORD (  C_DCHR    )/* Constant for bytes per word.            */
#define C_BITBYT (      8     )/* Number of bits for one byte.            */
#define C_HIBYTE (     256    )/* Total number of unsigned sigle byte     */
                               /* Member.                                 */
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

#define C_BYTEM  (    0xff    )/* Mask code for lower one byte.           */
/*
 *      Bit Mask Operation Pattern.
 */
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
 *      Nibble Mask.
 */
#define C_HHBYTE (    0xf0    )/* Mask code for higher nibble.            */
#define C_LHBYTE (    0x0f    )/* Mask code for lower nibble.             */
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
 *      Character Code Specified Define.
 */
#define C_JKCSID (     301    )/* Character set ID.(Japanese Kanji)       */

#define C_JISSEG (     94     )/* JIS Kanji Code character number per     */
                               /* one segment.(0x7e-0x21)                 */
#define C_SFTSEG ( C_JISSEG*2 )/* JIS Kanji Code one segment(X'21'Å`X'7E) */
#define C_PKC1BL (    0x81    )/* PC Kanji Code 1st block lower byte.     */
#define C_PKC1BH (    0x9f    )/* PC Kanji Code 1st block higher byte.    */
#define C_PKC2BH (    0xfc    )/* PC Kanji Code 2nd block higher byte.    */
#define C_PKC2BL (    0xe0    )/* PC Kanji Code 2nd block lower byte.     */
/*
 *      Cursor Posotion Control  Value.
 */
#define C_POS    (      1     )/* Offset/Position Conversion.             */
                               /* Pos <= Offset + C_POS.                  */
#define C_OFFSET (     -1     )/* Position/Offset Conversion.             */
                               /* Offset <= Pos + C_OFFSET.               */
#define C_COL    (      0     )/* Initial column value.                   */
#define C_ROW    (      1     )/* Initial row value.                      */
/*
 *      Logical Switch.
 */
#define C_SWOFF  (      0     )/* Logical Value FALSE.                    */
#define C_SWON   (      1     )/* Logical Value TRUE.                     */
#endif
