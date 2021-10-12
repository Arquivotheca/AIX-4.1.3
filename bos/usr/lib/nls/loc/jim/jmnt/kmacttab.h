/* @(#)78	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kmacttab.h, libKJI, bos411, 9428A410j 6/6/91 14:32:49 */

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
 * MODULE NAME:         kmacttab.h
 *
 * DESCRIPTIVE NAME:    Kanji Monitor Action Code Values.
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
#ifndef _kj_acttab
#define _kj_acttab
/*
 *      Kanji Monitor Action Code Type.
 *      Type 1:Group 1: Shift Indicator Control.
 *             Group 2: Kana/Kanji Conversion Control.
 *             Group 3: String Editing Control.
 *             Group 4: Diagnosis Trace Control .
 *      Code 2:Cursor Control.
 *      Code 3:Editing Mode Control.
 */
#define A_GRPSFT (    0x01    )/* GROUP1:S-Code(Shift).                   */
#define A_GRPKKC (    0x02    )/* GROUP2:K-Code(KKC)  Kana to Kanji Conv- */
                               /* ersion.                                 */
#define A_GRPEDT (    0x03    )/* GROUP3:E-Code(Edit).                    */
/*
 *      Kanji Monitor Action Code Type 1 Group 1.
 *      Releted Shift Indicator Control.
 */
#define A_NOP    (    0x00    )/* NOP(No Operation).                      */
                               /* This Value is Same Mean All Group.      */
#define A_SFTKAT (    0x01    )/* To set Katakana shift.                  */
#define A_SFTALP (    0x02    )/* To set Alpha/Num shift.                 */
#define A_SFTHIR (    0x03    )/* To set Hiragana shift.                  */
#define A_SFTRKC (    0x04    )/* To set Romaji-Kana Conversion shift.    */
#define A_RESET  (    0x05    )/* To do reset action.                     */
#define A_INSERT (    0x06    )/* To become Insert mode (Fix converted    */
                               /* characters if mode is changed.)         */
/*
 *      Kanji Monitor Action Code Type 1 Group 2.
 *      Releted Kana/Kanji Conversion.
 */
#define A_REDDEC (    0x10    )/* To do post fixing process.              */
#define A_CNVDEC (    0x20    )/* To fix converted characters.            */
#define A_ENTALC (    0xa1    )/* To be input process. for All Candidates */
                               /* or Switching Conv. mode.                */
#define A_ENTKJN (    0xa2    )/* To be input process. for Kanji No. mode.*/
#define A_NOCNV1 (    0xb1    )/* To become Non-conversion Status.        */
#define A_NOCNV2 (    0xb2    )/* Non-conversion mode (b2).               */
#define A_NOCNV3 (    0xb3    )/* Non-conversion mode (b3).               */
#define A_NOCNV4 (    0xb4    )/* Non-conversion mode (b4).               */
#define A_NOCNV5 (    0xb5    )/* Non-conversion mode (b5).               */
#define A_CONV1  (    0xc1    )/* To execute Kana to Kanji conversion.    */
#define A_NXTCNV (    0xc2    )/* To display next candidate.              */
#define A_NCVACN (    0xc3    )/* To become Next/Previous Conversion mode.*/
#define A_YOMICV (    0xd1    )/* To do partial phrase non-conversion     */
                               /* process for Conv. or Edit(B)            */
#define A_YOMCV2 (    0xd2    )/* To do partial phrase non-conversion.    */
                               /* process for Conv. or Edit(E)            */
#define A_PRVRSV (    0xe1    )/* **** RESERVED FOR FUTURE USE ****       */
#define A_PRVCAN (    0xe2    )/* To display previous candidate.          */
#define A_PRVALL (    0xe3    )/* To display previous candidates.(All     */
                               /* Candidates mode)                        */
#define A_ALLCAN (    0xe4    )/* To display the candidates.              */
                               /* (All Candidates mode)                   */
/*
 *      Kanji Monitor Action Code Type 1 Group 3.
 *      Releted String Editing Control.
 */
#define A_1STCHR (    0x01    )/* To be First Objective Character         */
#define A_CONCHR (    0x02    )/* To be Continuous Objective Character    */
                               /* Input Status.                           */
#define A_BCURDL (    0x03    )/* To delete a character at the left cursor*/
#define A_RCURDL (    0x04    )/* A character at cursor's position and all*/
                               /* characters which are                    */
#define A_IFDEL  (    0x05    )/* To delete all characters in the input   */
                               /* field.                                  */
#define A_CUPDEL (    0x06    )/* To delete a character at cursor's       */
                               /* position.                               */
#define A_CHREDT (    0x07    )/* To edit characters.                     */
#define A_BCKSPC (    0x09    )/* To be Back Space Edit mode (B).         */
#define A_ALCINP (    0x0a    )/* To input the appropriate number for the */
                               /* candidates.(All Candidatr mode)         */
#define A_KJNINP (    0x0b    )/* To be input process for Kanji No. mode. */
#define A_KJNDEL (    0x0c    )/* To be Delete Edit process for Kanji     */
                               /* No. mode.                               */
#define A_CMSINP (    0x0d    )/* To be character input process for       */
                               /* Switching Conversion Mode.              */
#define A_KJNBSP (    0x0e    )/* To be Back Space Edit process for kanji */
                               /* No. mode.                               */
/*
 *      Kanji Monitor Action Code Type 2.
 *      Releted Cursor Control.
 */
#define A_CRSC   (    0x01    )/* To move cursor one character right.     */
#define A_CLSC   (    0x02    )/* To move cursor one character left.      */
#define A_CUSC   (    0x03    )/* To move cursor upward. (One line)       */
#define A_CDSC   (    0x04    )/* To move cursor downward.(One line)      */
#define A_CRDC   (    0x05    )/* To move cursor two characters right.    */
#define A_CLDC   (    0x06    )/* To move cursor two characters left.     */
#define A_CIFS   (    0x0a    )/* To move cursor at the top of input field*/
#define A_CCVN   (    0x0b    )/* To move cursor at the top of converted  */
                               /* characters.                             */
#define A_CCVE   (    0x0c    )/* To move cursor at the end of converted  */
                               /* characters.                             */
#define A_CYMS   (    0x0d    )/* To move cursor at the head of Kana.     */
#define A_CSTP   (    0x0e    )/* To move cursor at the top of Field.     */
#define A_CRNA   (A_CRSC |0x10)/* To move cursor one character right.     */
                               /* ( Field Attribute is not changed)       */
#define A_CLNA   (A_CLSC |0x10)/* To move cursor one character left.      */
                               /* (Field Attribute is not changed.)       */
/*
 *      Kanji Monitor Action Code Type 3.
 *      Releted Editing Mode Control.
 */
#define A_BEEP   (    0x10    )/* Beep.                                   */
#define A_DICOFF (    0x20    )/* Return to Application.( Set discard flag*/
                               /* OFF.)                                   */
#define A_INPCON (    0x50    )/* To set unsuccessful conversion indicator*/
                               /* & Beep                                  */
#define A_MNOCNV (    0xa0    )/* To become First Kana Input mode if there*/
                               /* are no objective characters             */
#define A_1STINP (    0x01    )/* To become First Kana Input mode.        */
#define A_CONINP (    0x02    )/* To become Continuous Kana Input mode.   */
#define A_CNVMOD (    0x03    )/* To become Conversion mode.              */
#define A_EDTMOA (    0x04    )/* To be Edit mode.                        */
#define A_HIRKAT (    0x05    )/* To become Hiragana to/from Katakana Con-*/
                               /* version mode.                           */
#define A_ALCADM (    0x06    )/* To become All candidates mode.          */
#define A_KNJNOM (    0x07    )/* To become Kanji No. mode.( Display msg.)*/
#define A_MODESW (    0x08    )/* To become Switching Conversion Mode mode*/
                               /* ( Display message )                     */
#define A_DCREGA (    0x09    )/* To become Yomi Registration Stage mode. */
#define A_EDTMOB (    0x0a    )/* To be Edit mode (B).                    */
#define A_EDTMOC (    0x0b    )/* To be Edit mode (C).                    */
#define A_EDTMOD (    0x0c    )/* To be Edit mode (D).                    */
#define A_EDTMOE (    0x0d    )/* To be Edit mode (E).                    */
/*
 *      Kanji Monitor Action Code Type 4.
 *      Releted Diagnostic Trace Control.
 */
#define A_DAGMSG (    0x01    )/* Diag Start Switch On.                   */
#define A_GRPDIA (    0x04    )/* GROUP4:Diagnosis  (Trace Function)      */
#endif
