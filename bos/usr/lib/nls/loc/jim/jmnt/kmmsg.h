/* @(#)82	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/kmmsg.h, libKJI, bos411, 9428A410j 7/23/92 03:25:53 */
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
 * MODULE NAME:         kmmsg.h
 *
 * DESCRIPTIVE NAME:    Kanji Monitor DBCS Message Constant Values.
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
#ifndef _kj_kmmsg
#define _kj_kmmsg
#include "kjcommon.h"           /* Kanji Project Common Define.           */

/*
 **************************************************************************
 *      Caution: This Message Constans consist of DBCS Kanji Code Octal   *
 *               Expression. If Modify Messsage Then modify Each Message  *
 *               in Target Language Characte Set.                         *
 *               **** DBCS Character Set Constant Area is Marks'#' ****   *
 **************************************************************************
 */

/*
 *      Disgnostic Start Message Control  Information.
 */
/*########## Character Set Dependent Constant Start. #####################*/
#define M_TRSMSG ("\201\226\203\147\203\214\201\133\203\130\212\112\
\216\156\201\110\201\151\202\231\201\136\202\216\201\152\201\226")
                               /* Diagnostic Trace Start Message.         */
/*########## Character Set Dependent Constant End.   #####################*/
#define M_MSSLNC ((int)sizeof(M_TRSMSG)-1)
                               /* Diagnostic Trace Message Column Number  */
#define M_MSSLNR (      1     )/* Message Row Number.                     */
#define M_DGMSCL (     30     )/* Auxiliary Area Screen Column Number.    */
#define M_DGMSRL (      1     )/* Auxiliary Area Screen Row    Number.    */
#define M_DGMSCX (    C_FAUL  )/* Cursor Col. (unvislbe Cursor)           */
#define M_DGMSCY (    C_FAUL  )/* Cursor Row. (unvisble Cursor)           */
#define M_DGSCL  (      0     )/* Cursor left  Margin.                    */
#define M_DGSCR  (    C_DBCS  )/* Cursor right Margin.                    */
/*
 *      Diagnostic Stop Message Control Information.
 */
/*########## Character Set Dependent Constant Start. #####################*/
#define M_TREMSG ("\201\226\203\147\203\214\201\133\203\130\217\111\
\227\271\201\110\201\151\202\231\201\136\202\216\201\152\201\226")
                               /* Diagnostic Trace End Message.           */
/*########## Character Set Dependent Constant End.   #####################*/
#define M_MESLNC ((int)sizeof(M_TREMSG)-1)
                               /* Diagnostic Trace Message Column Number. */
#define M_MESLNR (      1     )/* Message Row Number.                     */
#define M_DGMECL (     30     )/* Auxiliary Area Column Number.           */
#define M_DGMERL (      1     )/* Auxiliary Area Row    Number.           */
#define M_DGMECX (   C_FAUL   )/* Cursor Col. (unvisible Cursor)          */
#define M_DGMECY (   C_FAUL   )/* Cursor Row. (unvisible Cursor)          */
#define M_DGECL  (      0     )/* Cursor left  Margin.                    */
#define M_DGECR  (   C_DBCS   )/* Cursor right Margin.                    */

/*
 *      All Candidate Mode Control Define.
 */
/*########## Character Set Dependent Constant Start. #####################*/
/*
 *      All Candidate:Input Field Message Set Mode.
 *      .... Word Level Candidate Remain Number Message.
 */
#define M_ACIFMG ("\201\100\222\120\216\143\201\100\201\100\201\100")
                               /* DBCS String.                            */
                               /* Phase Remain Number Message String.     */
/*
 *      All Candidate:Auxiliary Area Message Output:
 *      .... Word Level Candidate & Remain Candidate Number Messasge.
 */
#define M_ACAXM1 ("\201\100\222\120\212\277\201\100\216\143\202\350\
\201\100\201\100\201\100")
                               /* Word DBCS String Remain Number Message  */
                               /* String.                                 */
/*
 *      All Candidate:Auxiliary Area Message Output:
 *      .... Number & Candidate DBCS Message String.
 */
#define M_ACAXM2 ("\224\324\215\206\201\100\214\363\225\342")
                               /* DBCS String.                            */
                               /* All candidate Mode Help Message String. */
/*
 *      All Candidate Display Area Too Short.
 */
#define M_ACERMG ("\201\226\221\123\214\363\225\342\201\106\225\134\
\216\246\227\314\210\346\225\163\221\253")
                               /* DBCS String.                            */
                               /* All candidate Display Area Too Short    */
                               /* Message String.                         */
/*########## Character Set Dependent Constant End.   #####################*/
/*
 *      All Candidate:Input Field Message Output.
 *      .... Input Field Minimum Column Number.
 *      .... Consist of Candidate Message & Least M_ALCMIN Candidate.
 *
 *      0.X 1.Y 2.Z Message(M_ACIFMG).
 *      11112222333ZZZZZZZZ
 *      ====~~~~===~~~~~~~~
 *        |   |  |    Message.
 *        |   |  +----No o Candidate. ~~| Number of M_ALCMIN
 *        |   +-------No m Candidate.   |
 *        +-----------No n Candidate.   |
 *                                    ~~~
 *      M_ALIFCL = sizeof(M_ACIFMG)-1 + (4 * C_DBCS) * M_ALCMIN - C_DBCS
 *
 */
#define M_ALCMIN (     3      )/* Mimimim Display Number of All Candidate*/
#define M_KNJNOL (  C_DBCS*3  )/* Length of disp. sequencial No. (All     */
                               /* Candidates & Aux. area.)                */
#define M_ALIFCL ((M_KNJNOL+C_DBCS)*M_ALCMIN-C_DBCS+(int)sizeof(M_ACIFMG)-1)
                               /* Minimum field length at All Candidates  */
                               /* mode.                                   */
/*
 *      All Candidate:Auxiliary Area Message Output,with Multi row.
 */
#define M_ACMGFD (      2     )/* Number of All Candidate Mode Message.   */
#define M_ALAXDR ( M_ACMGFD+M_ALCMIN )
                               /* Minimum Row for Multi Row Mode.         */
#define M_DFLTRC (     10     )/* Default request count for All Candidates*/
                               /* mode.                                   */
#define M_ALAXRW ( M_ACMGFD+M_DFLTRC)
                               /* Number of columns for All Candidate mode*/
                               /* Message.                                */
/*
 *      All Candidate:Auxiliary Area Message Output,with Signel Line.
 */
#define M_ACAX1R (      1     )/* Number of Rows.                         */

/*########## Character Set Dependent Constant Start. #####################*/
/*
 *      Dictionary Registration Mode Message:
 *      .... Successful Dictionary Registration.
 */
#define M_RGEND  ("\201\226\216\253\217\221\202\311\223\157\230\136\
\202\263\202\352\202\334\202\265\202\275")
                               /* Dictionary Registration Complete.       */
/*
 *      Dictionary Registration Mode Message:
 *      .... Unsuccessful Dictionary Registration.
 */


/* #(B) 1988.01.14. Flying Conversion Change */
#define M_RGEMG  ("\201\226\216\253\217\221\202\311\223\157\230\136\
\202\305\202\253\202\334\202\271\202\361\202\305\202\265\202\275")
                               /* Dictionary Registration Unsuccesful     */
                               /* Message.                                */
/*
 *      Dictionary Registration Mode Message:
 *      .... Invalid Dictionary Registration Yomi.
 */
#define M_RGEMSG ("\201\226\223\307\202\335\202\252\220\263\202\265\
\202\255\202\240\202\350\202\334\202\271\202\361")
                               /* Invalid  Dictionary Registration Yomi.  */
/*
 *      Dictionary Registration Mode Message:
 *      .... Input Field is Too Short.
 */
#define M_RGERMG ("\201\226\223\374\227\315\203\164\203\102\201\133\
\203\213\203\150\202\252\221\253\202\350\202\334\202\271\202\361")
                               /* Dictionary Registreation Message Cannot */
                               /* Display in Input Field.                 */
/*
 *      Dictionary Registration Mode Message:
 *      .... Dictionary Registration DBCS String Too Long.
 */
/**********************************/
/* #(B) 1988.01.27 Message change */
/* KANJI => GOKU                  */
/* #define M_RGKEMG ("\201\226\212\277\216\232\202\252\222\267\202\267\   */
/**********************************/
#define M_RGKEMG ("\201\226\214\352\213\345\202\252\222\267\202\267\
\202\254\202\334\202\267")     /* Too Long DBCS String for Dictionary     */
                               /* Registration,It's Maximum Length is     */
                               /* defined M_RGIFL.                        */
/*
 *      Dictionary Registration Mode Message:
 *      .... Dictionary Registration DBCS Input Prompt.
 */


/* #(B) 1988.01.14. Flying Conversion Change */
#define M_RGKMSG ("\214\352\213\345\201\204")
                               /* Dictionary Registration DBCS String     */
                               /* Input Prompt.                           */
/*
 *      Dictionary Registration Mode Message:
 *      .... Dictionary Registartion Yomi Input Prompt.
 */
#define M_RGYMSG ("\223\307\202\335\201\204")
                               /* Dictionary Registration Yomi String     */
                               /* Input Prompt.                           */
/*########## Character Set Dependent Constant End.   #####################*/
/*
 *      Conversion Mode Switch Change Message.
 */
/*########## Character Set Dependent Constant Start. #####################*/

/* #(B) 1988.01.12. Flying Conversion Change */

#define M_MSWMSG ("\
\225\317\212\267\203\202\201\133\203\150\201\106\201\100\201\100\201\100\
\202\120\201\104\220\346\223\307\202\335\230\101\225\266\220\337\201\100\
\202\121\201\104\210\352\212\207\230\101\225\266\220\337\201\100\201\100\
\202\122\201\104\222\120\225\266\220\337\201\100\201\100\201\100\201\100\
\202\123\201\104\225\241\215\207\214\352\201\100\201\100\201\100\201\100")
                               /* Conversion Mode Select Message.         */
/*########## Character Set Dependent Constant End.   #####################*/

/* #(B) 1988.01.12. Flying Conversion Change */
#define M_MSWMSC (     18     )/* Auxiliary Area Column Number.           */

/* #(B) 1988.01.12. Flying Conversion Change */
#define M_MSWMSR (      5     )/* Auxiliary Area Row    Number.           */

/* #(B) 1988.01.18. Flying Conversion Add */
#define M_MSWATR (     16     )/* Atribute Length of Conversion Mode Switch*/

#define M_FUKPOS (M_MSWMSC*4  )/* Word Level Hilighting Position.         */
#define M_TANPOS (M_MSWMSC*3  )/* Single Phase Hilighting Position.       */
#define M_RENPOS (M_MSWMSC*2  )/* Multi Phase Conversion Hilightin Pos.   */

/* #(B) 1988.01.12. Flying Conversion Add */
#define M_FLYPOS (M_MSWMSC    )/* Flying Conversion and
                                  Multi Phase Conversion Hilightin Pos.   */
/*
 *      DBCS Kanji Number Message Information.(JIS Mode)
 */
/*########## Character Set Dependent Constant Start. #####################*/
#define M_KNMSG  ("\202\151\202\150\202\162\213\346\223\137\201\204\
\201\100\201\100\201\100\201\100\201\100")
                               /* Kanji Number Input Prompt Message.      */
#define M_KNMSGI ("\212\277\216\232\224\324\215\206\201\100\201\204\
\201\100\201\100\201\100\201\100\201\100")
                               /* IBM Kanji Number Input Prompt Message.  */
#define M_KNMSGE ("\201\226\223\374\227\315\210\346\225\163\221\253\
\201\100\201\100\201\100\201\100")
                               /* Kanji Number Input Length Too Short     */
                               /* Error Message.                          */
/*########## Character Set Dependent Constant End,   #####################*/
#define M_KNMSRL (      1     )/* Kanji No. Number of Message Row.        */
#define M_KNMCSL (     12     )/* cursor left margin Position.            */
#define M_KNMCSR (     12     )/* cursor right margin Position.           */
#define M_KNMCSX (     12     )/* cursor column position.                 */
#define M_KNMCSY (      1     )/* cursor row position.                    */
#define M_KNMSCL ((int)sizeof(M_KNMSG)-1)
                               /* Kanji Number Message Length.            */
#define M_MAXKJN ( C_DBCS*5+1 )/* Maximum number of Kanji No. input.      */
                               /* Mimimum Length of Kanji Code Input      */
                               /* Buffer.                                 */
#endif
