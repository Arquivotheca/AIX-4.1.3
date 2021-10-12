/* @(#)88	1.1  src/bos/usr/lpp/kls/dictutil/hucommon.h, cmdkr, bos411, 9428A410j 5/25/92 14:40:39 */
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hucommon.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hucommon.h
 *
 *  Description:  header file.
 * 		  Defines User Dictionary Maintenance Common Contant.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ********************************************************************************/

#ifndef _HUCOMMON_H_
#define _HUCOMMON_H_

/*------------------------------------------------------------------------------*/
/*              	Utility Common Constants.				*/
/*------------------------------------------------------------------------------*/

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
#define C_SBCS    ((int)sizeof(char))
                               /* Single byte code.                       */
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
#define C_SPACE  (   0xa1a1   )/* Blank code. (PC Kanji Code)             */
#define C_SPACEH (    0xa1    )/* Higher one-byte code for blank code.    */
#define C_SPACEL (    0xa1    )/* Lower one-byte code for blank code      */
#define C_QUOTO  (   0xfa57   )/* IBM extention Kanji code for double     */
                               /* quotation.                              */
#define C_LIT    (   0xfa56   )/* IBM extention Kanji code for single     */
                               /* quotation.                              */

#define C_BYTEM  (    0xff    )/* Mask code for lower one byte.           */
/*
 *      Nibble Mask.
 */
#define C_HHBYTE (    0xf0    )/* Mask code for higher nibble.            */
#define C_LHBYTE (    0x0f    )/* Mask code for lower nibble.             */

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

/*------------------------------------------------------------------------------*/
/*              	Utility Common Constants.				*/
/*------------------------------------------------------------------------------*/
#endif
