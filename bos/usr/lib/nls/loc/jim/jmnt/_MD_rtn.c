static char sccsid[] = "@(#)43	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MD_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:18:15";
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
 * MODULE NAME:         _MD_rtn
 *
 * DESCRIPTIVE NAME:    Code Process Routine
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            Call the following routines according to
 *                      the depressed key and Kanji Monitor Action Table.
 *                              Shift setting routine.
 *                              KKC interface routine.
 *                              Character editting routine.
 *                              Cursor control routine.
 *                              Kanji Monitor Mode Switching Mode routine.
 *                              Flying Conversion Processing.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3300 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MD_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MD_rtn( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MS_rtn() : Shift Setting Routine.
 *                              _MK_rtn() : KKC Interface Routine.
 *                              _ME_rtn() : Character Editting Routine.
 *                              _MC_rtn() : Cursor Control Routine.
 *                              _MM_rtn() : Kanji Monitor Mode
 *                                          Switching Routine.
 *                              _Mflypro(): Flying Conversion Processing.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              kjsvpt  type    curlen  kbdlok
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chcodelen       kkmode1 pscode  msetflg
 *                              nextact
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              charcont        chcode
 *                              actc1   actc2   actc3   nextact
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Bug Fix.
 *                      1987,12/10 Phrase Delimited Convertion
 *                                 Table is Illegal.
 *                      1989,08/18 Conform to ANSI C standard. 
 *				   Aggregate Initialization Rule.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Kanji Monitor Action Table (Key Groups)
 *
 *              Pseudo Code Group Static Name : PSC_GRP[]
 *                      PSC_GRP[Group]
 */
        static uchar PSC_GRP[52] = {
                                A_GRPEDT ,      A_GRPSFT ,      A_GRPSFT,
                                A_GRPSFT ,      A_GRPSFT ,      A_GRPKKC,
                                A_GRPKKC ,      A_GRPKKC ,      A_GRPKKC,
                                A_GRPKKC ,      A_GRPKKC ,      A_GRPDIA,
                                A_GRPKKC ,      A_NOP    ,      A_NOP   ,
                                A_NOP    ,      A_NOP    ,      A_NOP   ,
                                A_GRPKKC ,      A_NOP    ,      A_NOP   ,
                                /*--------------  DUMMY  ---------------*/
                                A_NOP    ,      A_NOP    ,      A_NOP   ,
                                A_NOP    ,      A_NOP    ,      A_NOP   ,
                                A_NOP    ,      A_NOP    ,      A_NOP   ,
                                A_NOP    ,      A_NOP    ,
                                /*--------------  DUMMY  ---------------*/
                                A_GRPKKC ,      A_GRPKKC ,      A_GRPKKC,
                                A_GRPSFT ,      A_GRPEDT ,      A_GRPEDT,
                                A_GRPEDT ,      A_GRPEDT ,      A_GRPEDT,
                                A_GRPEDT ,      A_GRPEDT ,      A_GRPEDT,
                                A_GRPSFT ,      A_GRPEDT ,      A_GRPEDT,
                                A_GRPEDT ,      A_GRPEDT ,      A_NOP   ,
                                A_NOP    ,      A_GRPSFT
                           };

/*
 *      Kanji Monitor Action Table (Action Codes)
 *
 *              KMAT Substances Static Name : KMAT[][][]
 *                      KMAT[ mode ][ pseudo code ][ code 1.2.3 ]
 */
        static uchar KMAT[13][52][3] = {
/*===== Mode 01 : First Yomi Input Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_1STCHR             ,  A_CRNA  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_INPCON            },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_NOP                ,  A_NOP   ,       A_KNJNOM            },
      { A_NOP                ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_DICOFF            },
      { A_NOP                ,  A_NOP   ,       A_DICOFF            },
      { A_NOP                ,  A_NOP   ,       A_DICOFF            },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_NOP               },
      { A_NOP                ,  A_CLSC  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_CRDC  ,       A_NOP               },
      { A_NOP                ,  A_CLDC  ,       A_NOP               },
      { A_RCURDL             ,  A_NOP   ,       A_NOP               },
      { A_IFDEL              ,  A_CSTP  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CUPDEL             ,  A_NOP   ,       A_NOP               },
      { A_BCURDL             ,  A_CLNA  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_DICOFF            },
      { A_NOP                ,  A_CIFS  ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 02 : Continuous Yomi Input Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CONCHR             ,  A_CRNA  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_CONV1              ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_HIRKAT            },

/* #(B) 1987.12.08. Flying Conversion Change */
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
/* #(E) 1987.12.08. Flying Conversion Change */

      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_REDDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_REDDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_REDDEC             ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_REDDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_REDDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_BCURDL             ,  A_CLNA  ,       A_MNOCNV            },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 03 : Conversion Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_1STCHR  ,  A_CRNA  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NXTCNV             ,  A_NOP   ,       A_NOP               },
      { A_NOCNV1             ,  A_CYMS  ,       A_EDTMOA            },
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_PRVCAN             ,  A_NOP   ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_YOMICV             ,  A_CYMS  ,       A_EDTMOA            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_BCURDL  ,  A_CLNA  ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 04 : Edit Mode (A) =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_1STCHR  ,  A_CRNA  ,       A_CONINP            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NXTCNV             ,  A_NOP   ,       A_EDTMOA            },
      { A_NOCNV2             ,  A_CYMS  ,       A_EDTMOA            },
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_PRVCAN             ,  A_CCVE  ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_YOMICV             ,  A_CYMS  ,       A_EDTMOA            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_BCURDL  ,  A_CLNA  ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 05 : Hiragana/Katakana Conversion Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_REDDEC | A_1STCHR  ,  A_CRNA  ,       A_CONINP            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_INPCON | A_1STINP },
      { A_NOCNV4             ,  A_NOP   ,       A_NOP               },
      { A_REDDEC             ,  A_NOP   ,       A_BEEP | A_1STINP   },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_REDDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_REDDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_BEEP | A_1STINP   },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_BEEP | A_1STINP   },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_REDDEC             ,  A_CRSC  ,       A_1STINP            },
      { A_REDDEC             ,  A_CLSC  ,       A_1STINP            },
      { A_REDDEC             ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC             ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC             ,  A_CRDC  ,       A_1STINP            },
      { A_REDDEC             ,  A_CLDC  ,       A_1STINP            },
      { A_REDDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_REDDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_REDDEC | A_BCURDL  ,  A_CLNA  ,       A_1STINP            },
      { A_REDDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_REDDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 06 : All Candidates Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_ALCINP             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NCVACN             ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 04 (Diagnostic) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_PRVALL             ,  A_NOP   ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 07 : Kanji Number Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_KJNINP             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_KNJNOM            },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 04 (Diagnostic) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_ENTKJN             ,  A_NOP   ,       A_NOP               },
      { A_ENTKJN             ,  A_NOP   ,       A_NOP               },
      { A_ENTKJN             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_NOP               },
      { A_NOP                ,  A_CLSC  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_CRDC  ,       A_NOP               },
      { A_NOP                ,  A_CLDC  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_KJNDEL             ,  A_NOP   ,       A_NOP               },
      { A_KJNBSP             ,  A_CLNA  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 08 : Conversion Mode Switching =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CMSINP             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 04 (Diagnostic) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
      { A_ENTALC             ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 01 (S-Code) ---*/

/* #(B) 1988.01.12. Flying Conversion Change */
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/* #(E) 1988.01.12. Flying Conversion Change */

/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 09 : Dictionary Registration Mode =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_1STCHR             ,  A_CRNA  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 04 (Diagnostic) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_NOP               },
      { A_NOP                ,  A_CLSC  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_CRDC  ,       A_NOP               },
      { A_NOP                ,  A_CLDC  ,       A_NOP               },
      { A_RCURDL             ,  A_NOP   ,       A_NOP               },
      { A_IFDEL              ,  A_CSTP  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CUPDEL             ,  A_NOP   ,       A_NOP               },
      { A_BCKSPC             ,  A_CLNA  ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 0a : Edit Mode (B) =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CHREDT             ,  A_CRNA  ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_CONV1              ,  A_NOP   ,       A_EDTMOA            },
      { A_NOCNV3             ,  A_CRSC  ,       A_EDTMOA            },

/* #(B) 1987.12.08. Flying Conversion Change */
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
/* #(E) 1987.12.08. Flying Conversion Change */

      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOCNV3             ,  A_CRSC  ,       A_EDTMOA            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CUPDEL             ,  A_NOP   ,       A_EDTMOA            },
      { A_BCKSPC             ,  A_CLNA  ,       A_NOP               },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 0b : Edit Mode (C) =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_1STCHR  ,  A_CRNA  ,       A_CONINP            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_INPCON            },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_BCURDL  ,  A_CLNA  ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 0c : Edit Mode (D) =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CHREDT             ,  A_CRNA  ,       A_EDTMOA            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NXTCNV             ,  A_CCVE  ,       A_EDTMOA            },
      { A_NOCNV2             ,  A_CYMS  ,       A_EDTMOA            },
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_PRVCAN             ,  A_CCVE  ,       A_NOP               },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      /******************************************************************/
      /*#(B)  Bug Fix. Thu Dec 10,1987                                  */
      /*        Modify Reason.                                          */
      /*                Table Data Invalid.                             */
      /*        Modify Source.                                          */
      /*                { A_YOMICV,A_SYMS,A_NOP   }                     */
      /*                        |                                       */
      /*                        v                                       */
      /*                { A_YOMICV,A_SYMS,A_EDTMOA}                     */
      /*                                  ---------                     */
      /******************************************************************/
      { A_YOMICV             ,  A_CYMS  ,       A_EDTMOA            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_BCKSPC             ,  A_CLNA  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

/*===== Mode 0d : Edit Mode (E)  =====*/
    {				/* updt 08/18/89, See Change Activity */
/*----< Code 1 >--------------< Code 2 >------< Code 3 >----------------*/
/*--- Group 03 (E-Code) ---*/
      { A_CHREDT             ,  A_CRNA  ,       A_EDTMOA            },
/*--- Group 01 (S-Code) ---*/
      { A_SFTKAT             ,  A_NOP   ,       A_NOP               },
      { A_SFTALP             ,  A_NOP   ,       A_NOP               },
      { A_SFTHIR             ,  A_NOP   ,       A_NOP               },
      { A_SFTRKC             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_CONV1              ,  A_NOP   ,       A_EDTMOA            },
      { A_NOCNV5             ,  A_NOP   ,       A_EDTMOA            },

/* #(B) 1987.12.08. Flying Conversion Change */
      { A_ALLCAN             ,  A_NOP   ,       A_ALCADM            },
/* #(E) 1987.12.08. Flying Conversion Change */

      { A_NOP                ,  A_NOP   ,       A_DCREGA            },
      { A_CNVDEC             ,  A_NOP   ,       A_KNJNOM            },
      { A_CNVDEC             ,  A_NOP   ,       A_MODESW            },
/*--- Group 04 (Diagnostic) ---*/
      { A_DAGMSG             ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_BEEP              },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 02 (K-Code) ---*/
      { A_YOMCV2             ,  A_NOP   ,       A_EDTMOA            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Dummy ------------------------------------------------------------*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*----------------------------------------------------------------------*/
/*--- Group 02 (K-Code) ---*/
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
/*--- Group 01 (S-Code) ---*/
      { A_RESET              ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_NOP                ,  A_CRSC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLSC  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC             ,  A_NOP   ,       A_1STINP            },
      { A_NOP                ,  A_CRDC  ,       A_EDTMOA            },
      { A_NOP                ,  A_CLDC  ,       A_EDTMOA            },
      { A_CNVDEC | A_RCURDL  ,  A_NOP   ,       A_1STINP            },
      { A_CNVDEC | A_IFDEL   ,  A_CSTP  ,       A_1STINP            },
/*--- Group 01 (S-Code) ---*/
      { A_INSERT             ,  A_NOP   ,       A_NOP               },
/*--- Group 03 (E-Code) ---*/
      { A_CNVDEC | A_CUPDEL  ,  A_NOP   ,       A_1STINP            },
      { A_BCKSPC             ,  A_CLNA  ,       A_EDTMOA            },
      { A_CNVDEC             ,  A_NOP   ,       A_DICOFF | A_1STINP },
      { A_CNVDEC             ,  A_CIFS  ,       A_1STINP            },
/*--- Group 00 (Nop) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
      { A_NOP                ,  A_NOP   ,       A_NOP               },
/*--- Group 01 (S-Code) ---*/
      { A_NOP                ,  A_NOP   ,       A_NOP               },
    },

    };

/*
 *      This module calls the following routine
 *              Call the following routines according to
 *              the depressed key and Kanji Monitor Action Table.
 *                      Shift setting routine.
 *                      KKC interface routine.
 *                      Character editting routine.
 *                      Cursor control routine.
 *                      Kanji Monitor Mode Switching routine.
 */
int  _MD_rtn( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _MS_rtn();      /* Shift Setting Routine        */
        extern  int     _MK_rtn();      /* KKC Interface Routine        */
        extern  int     _ME_rtn();      /* Character Editting Routine   */
        extern  int     _MC_rtn();      /* Coursor Control Routine      */
        extern  int     _MM_rtn();      /* Kanji Monitor Mode Switching */
                                        /* Routine                      */
        extern  int     _Mflypro();     /* Flying conversion Processing.*/

        uchar   group;          /* KMAT Group Code                      */
        short   chcodlen;       /* Changed Code Length                  */
        int     i;              /* Loop Counter                         */
        int     j;              /* Loop Counter                         */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MD_rtn,"start _MD_rtn");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = IMSUCC;

        /* 1. 1
         *      Check Type Code
         */

        if ( pt->type == K_INASCI )
           {
                /*
                 *      ASCII Code (Charcter Code)
                 */
                kjsvpt->charcont = C_SWOFF;             /* RKC Flag     */

                /* Changed Code Length */
                chcodlen = kjsvpt->chcodlen / C_DBCS;
           }
        else
           {
                /*
                 *      Not ASCII Code
                 */
                chcodlen = 1;           /* Set Changed Length           */
           };

        for ( i = 0; i < chcodlen; i++ )
            {

                /* 1. a
                 *      Move Changed Code to Left
                 */

                if ( i != 0 )
                   {
                        for ( j = 0; j < pt->curlen; j++ )
                            {
                                /* Move chcode to Left */
                                kjsvpt->chcode[j] =
                                        kjsvpt->chcode[pt->curlen+j];
                            };
                   };

                /* 1. b
                 *      Check Keyboard Lock Flag
                 */

                if ( pt->type == K_INASCI &&
                     i != 0 &&
                     pt->kbdlok == K_KBLON )
                   {
                        break;
                   };

                /* 1. c
                 *      RKC Mode Switch
                 */

                if ( pt->type == K_INASCI && i != 0 )
                        kjsvpt->charcont = C_SWON;      /* Set RKC Flag */

                /* 1. 1
                 *      Kanji Monitor Action Table
                 *              Set Action Code or Group Code
                 */

                /* Set KMAT Action Code 1 */
                kjsvpt->actc1 = KMAT[kjsvpt->kkmode1-1][kjsvpt->pscode][0];

                /* Set KMAT Action Code 2 */
                kjsvpt->actc2 = KMAT[kjsvpt->kkmode1-1][kjsvpt->pscode][1];

                /* Set KMAT Action Code 3 */
                kjsvpt->actc3 = KMAT[kjsvpt->kkmode1-1][kjsvpt->pscode][2];

                /* Set KMAT Group */
                group = PSC_GRP[kjsvpt->pscode];

                /* 1. 2
                 *      Process by Group or Action Code
                 */

                switch ( group )
                       {
                        case A_NOP:
                             break;

                       /* 1. 2-1. a
                        *      Shift Process
                        */
                        case A_GRPSFT:
                             /* Call Shift Setting Routine */
                             rc = _MS_rtn( pt, kjsvpt->actc1 );
                             break;

                        /* 1. 2-1. b
                         *      Kana / Kanji Conversion Routine
                         */
                        case A_GRPKKC:
                             /* Call KKC Interface Routine */
                             rc = _MK_rtn( pt, kjsvpt->actc1 );
                             break;

                        /* 1. 2-1. c-1.2
                         *      Fix Converted Charcter and
                         *                    Charcter Edit Routine
                         *      (Conversion Fixing if Necessary)
                         */
                        case A_GRPEDT:
                             /* Call KKC Interface Routine */
                             rc = _MK_rtn( pt, kjsvpt->actc1 & C_SBF0 );

                             /* Call Character Editting Routine */
                             rc = _ME_rtn( pt, kjsvpt->actc1 );
                             break;

                        /* 1. 2-1. d
                         *      Set Trace Start
                         */
                        case A_GRPDIA:
                             if ( kjsvpt->actc1   == A_DAGMSG &&
                                  kjsvpt->msetflg == 0 )

                                  /* Set Trace Start Flag */
                                  kjsvpt->nextact = kjsvpt->nextact |
                                                            M_DAGMON;
                             break;
                       };

                /* 1. 2-2
                 *      Cursor Control Routine
                 */

                if ( pt->kbdlok != K_KBLON  ||
                     kjsvpt->charcont == C_SWON )
                   {
                        /* Call Cursor Control Routine */
                        rc = _MC_rtn( pt, kjsvpt->actc2 );

                        /* 1. 2-3
                         *      Mode Change Routine
                         *              Call Mord Conversion Routine
                         */
                        rc = _MM_rtn( pt, kjsvpt->actc3 );



/* #(B) 1987.12.08. Flying Conversion Add */
                       /*
                        *       Flying Conversion Processing.
                        */
                       rc = _Mflypro( pt );
/* #(E) 1987.12.08. Flying Conversion Add */



                   };
            };

        /* 1. 3
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MD_rtn,"return _MD_rtn");

        return( rc );
}
