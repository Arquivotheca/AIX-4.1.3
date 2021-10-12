/* @(#)68	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kjcodtab.h, libKJI, bos411, 9428A410j 6/6/91 14:30:22 */

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
 * MODULE NAME:         kjcodtab.h
 *
 * DESCRIPTIVE NAME:    JIS 8bit Character Constant & Pseudo Code
 *                      Constant Defines.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
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
 *      Keyboard Data Stroke & Kanji Monitor Interface Data Interchange
 *      Code.
 */
#ifndef _kj_codtab
#define _kj_codtab
/*
 *      DBCS Editor DBCS Character Code Processing Effective Key Code Define.
 *      Releted GSL gsevwt_ Return Code Type 1,Type 2.
 *      (ANSI VDT Sequence)
 */
/*
 *      ESC + Termination Character Sequence.
 */
#define S_BTAB   (      0     )/* Back Tab.                               */
#define S_CURD   (      0     )/* Cursor Down.                            */
#define S_CURL   (      0     )/* Cursor Left.                            */
#define S_CURR   (      0     )/* Cursor Right.                           */
#define S_CURU   (      0     )/* Cursor Up.                              */
#define S_DELETE (      0     )/* Delete.                                 */
#define S_HOME   (      0     )/* Home.                                   */
/*
 *      ESC + Alpha Numeric + Terminate Character Sequecne.
 */
#define S_KATA   (      1     )/* Katakana Shift.                         */
#define S_ALPHA  (      2     )/* Alpha/Num shift.                        */
#define S_HIRA   (      3     )/* Hiragana Shift.                         */
#define S_RKC    (      4     )/* RKC.                                    */
#define S_CONV   (      5     )/* Convert                                 */
#define S_NCONV  (      6     )/* No Convert.                             */
#define S_ACAND  (      7     )/* All Candidates.                         */
#define S_REG    (      8     )/* Registration.                           */
#define S_KANJI  (      9     )/* Kanji No.                               */
#define S_CNVMSW (     10     )/* Conversion Mode Switch.                 */
#define S_DIAG   (     11     )/* Diagnosis.                              */
#define S_PCAND  (     12     )/* Previous Candidates.                    */
#define S_REVOD  (     13     )/* Reserved for DBCS.                      */
#define S_REVOE  (     14     )/* Reserved for DBCS.                      */
#define S_REVOF  (     15     )/* Reserved for DBCS.                      */
#define S_REV10  (     16     )/* Reserved for DBCS.                      */
#define S_REV11  (     17     )/* Reserved for DBCS.                      */
#define S_REV12  (     18     )/* Partial phrash non conversion.          */
#define S_REV13  (     19     )/* Reserved for DBCS.                      */
#define S_REV14  (     20     )/* Reserved for DBCS.                      */
#define S_ACTION (     114    )/* Action                                  */
#define S_RESET  (     122    )/* Reset.                                  */
#define S_INSERT (     139    )/* Insert.                                 */
#define S_EREOF  (     146    )/* ErEOF.                                  */
#define S_EPINP  (     149    )/* ErInput.                                */
#define S_CURDL  (     160    )/* Cursor Double Left.                     */
#define S_CURDR  (     169    )/* Cursor Double Right.                    */
/*
 *      Ascii Code Control Character Define.
 */
#define J_NULL   (    0x00    )/* Null Character.                         */
#define J_SOH    (    0x01    )/* Start of Header Character.              */
#define J_STX    (    0x02    )/* Start of Text Character.                */
#define J_ETX    (    0x03    )/* End of Text Character.                  */
#define J_EOT    (    0x04    )/* End of Transmission Character.          */
#define J_ENQ    (    0x05    )/* Enquiry Character.                      */
#define J_ACK    (    0x06    )/* Acknowlege Character.                   */
#define J_BEL    (    0x07    )/* Bell Character.                         */
#define J_BS     (    0x08    )/* Backspace Character.                    */
#define J_HT     (    0x09    )/* Horizonal Tab Character.                */
#define J_LF     (    0x0a    )/* Line Feed Character.                    */
#define J_VT     (    0x0b    )/* Vertical Tab Character.                 */
#define J_FF     (    0x0c    )/* Feed Form Character.                    */
#define J_CR     (    0x0d    )/* Carriage Return Character.              */
#define J_SO     (    0x0e    )/* Shift Out Character.                    */
#define J_SI     (    0x0f    )/* Shift In Character.                     */
#define J_DLE    (    0x10    )/* Data Link Escape Character.             */
#define J_DC1    (    0x11    )/* Device Control 1 Character.             */
#define J_DC2    (    0x12    )/* Device Control 2 Character.             */
#define J_DC3    (    0x13    )/* Device Control 3 Character.             */
#define J_DC4    (    0x14    )/* Device Control 4 Character.             */
#define J_NAK    (    0x15    )/* Negative Acknowledge Character.         */
#define J_SYN    (    0x16    )/* Syncronous Character.                   */
#define J_ETB    (    0x17    )/* End of Transmission Block Character.    */
#define J_CAN    (    0x18    )/* Cancel Character.                       */
#define J_EM     (    0x19    )/* End of Medium Character.                */
#define J_SUB    (    0x1a    )/* Substitute Character.                   */
#define J_ESC    (    0x1b    )/* Escape Character.                       */
#define J_FS     (    0x1c    )/* File Separator Character.               */
#define J_GS     (    0x1d    )/* Group Separator Character.              */
#define J_RS     (    0x1e    )/* Record Separator Character.             */
#define J_US     (    0x1f    )/* Unit Separator Character.               */
#define J_DEL    (    0x7f    )/* Delete Character.                       */
/*
 *      Pseude Code Defines(Kanji Monitor Action Trigger).
 */
#define P_CDNULL (    0x00    )/* NULL                                    */
#define P_KATA   (    0x01    )/* Katakana shift.                         */
#define P_ALPHA  (    0x02    )/* Alpha/Num shift.                        */
#define P_HIRA   (    0x03    )/* Hiragana shift.                         */
#define P_RKC    (    0x04    )/* RKC.                                    */
#define P_CONV   (    0x05    )/* Convert.                                */
#define P_NCONV  (    0x06    )/* No convert.                             */
#define P_ACAND  (    0x07    )/* All candidates.                         */
#define P_REG    (    0x08    )/* Registration.                           */
#define P_KANJI  (    0x09    )/* Kanji No.                               */
#define P_CNVMSW (    0x0a    )/* Conversion mode switch.                 */
#define P_DIAG   (    0x0b    )/* Diagnosis.                              */
#define P_PCAND  (    0x0c    )/* Preversion Candidates.                  */
#define P_REVOD  (    0x0d    )/* **** RESERVED FOR DBCS ****             */
#define P_REVOE  (    0x0e    )/* **** RESERVED FOR DBCS ****             */
#define P_REVOF  (    0x0f    )/* **** RESERVED FOR DBCS ****             */
#define P_REV10  (    0x10    )/* **** RESERVED FOR DBCS ****             */
#define P_REV11  (    0x11    )/* **** RESERVED FOR DBCS ****             */
#define P_PARPHY (    0x12    )/* **** RESERVED FOR DBCS ****             */
#define P_REV13  (    0x13    )/* **** RESERVED FOR DBCS ****             */
#define P_REV14  (    0x14    )/* **** RESERVED FOR DBCS ****             */
#define P_CDESP  (    0x1b    )/* Escape                                  */
#define P_CDUS   (    0x1f    )/* US Code.                                */
#define P_ENTER  (    0x20    )/* Enter.                                  */
#define P_ACTION (    0x21    )/* Action.                                 */
#define P_CR     (    0x22    )/* CR.                                     */
#define P_RESET  (    0x23    )/* Reset.                                  */
#define P_CURR   (    0x24    )/* Cursor Right.                           */
#define P_CURL   (    0x25    )/* Cursou Left.                            */
#define P_CURU   (    0x26    )/* Cursor Up.                              */
#define P_CURD   (    0x27    )/* Cursor Down.                            */
#define P_CURDR  (    0x28    )/* Cursor Double Right.                    */
#define P_CURDL  (    0x29    )/* Cursor Double Left.                     */
#define P_EREOF  (    0x2a    )/* ErEOF.                                  */
#define P_EPINP  (    0x2b    )/* ErInput.                                */
#define P_INSERT (    0x2c    )/* Insert.                                 */
#define P_DELETE (    0x2d    )/* Delete.                                 */
#define P_BSPACE (    0x2e    )/* Back Space.                             */
#define P_TAB    (    0x2f    )/* Tab (Jump).                             */
#define P_BTAB   (    0x30    )/* Back Tab.                               */
#define P_HOME   (    0x31    )/* Home.                                   */
#define P_CAPSLK (    0x33    )/* CAPS Lock.                              */
#define P_ESCAPE (    0x32    )/* Escape.                                 */
#define P_CLR    (    0x40    )/* Pseudo code (function code) exception.  */
#endif
