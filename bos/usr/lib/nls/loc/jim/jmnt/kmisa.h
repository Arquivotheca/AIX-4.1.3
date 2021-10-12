/* @(#)81	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kmisa.h, libKJI, bos411, 9428A410j 6/6/91 14:33:32 */

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
 * MODULE NAME:         kmisa.h
 *
 * DESCRIPTIVE NAME:    Kanji Monitor Internal Save Area Structure Define.
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
 * CHANGE ACTIVITY:     Sept. 20 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *                      Added fcvovflg flag.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
#ifndef _kj_KMISA
#define _kj_KMISA

#include "kmdef.h"      /* Kanji Monitor Control  Defines.                */
#include "kjmacros.h"   /* Kanji Project Macros.                          */
#include "kmpf.h"       /* Kanji Monitor Profile Structure.               */
#include "fsb.h"        /* Field Information Save Structure.              */
#include "kkcb.h"       /* Kana/Kanji Control Block Structure.            */

/*
 *      Create Kanji Monitor Control Block General Name.
 */
typedef struct kmisablk KMISA;

/*
 *      Kanji Monitor Control Block Structure.
 */
struct kmisablk {
long   length;  /* Length of Kanji Internal Save Area(byte)               */
                /* Kanji Monitor sets this.                               */

long   kmisaid; /* Kanji Monitor Internal Save Area Id.                   */
                /* Kanji Monitor Sets this.                               */

KKCB   *kkcbsvpt;
                /* Pointer to Kana/Kanji Control Block Save Area          */
                /* Kanji Kanji Routine sets this.                         */

FSB    *ifsaveo;/* Pointer to Display String Save area for Display        */
                /* Message                                                */
                /* Kanji Monitor sets this.                               */

FSB    *ifsaved;/* Pointer to Display String Save area for Display        */
                /* Dictionary regist Message                              */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      KMPF    kmpf[2];                                                  */
/* New source.                                                            */
/*      KMPF    kmpf[M_PROFSA];                                           */
/*========================================================================*/
KMPF   kmpf[M_PROFSA];
		/* Kanji Monitor Profile Save Area.                       */
                /* kmpf[0] ... Profile Data.                              */
                /* kmpf[1] ... Profile Control Data.                      */
                /* Kanji Monitor sets this.                               */

uchar  *kjcvmap;/* pointer Kanji Conversion Map.                          */
                /* Kanji Monitor sets this.                               */
uchar  *stringsv;
                /* Pointer to Replace Mode Background String Save Area    */
                /* Kanji Monitor sets this.                               */

uchar  *kanamap;
                /* Pointer to Kana Kanji Conversion Kana Map.             */
                /* Kanji Monitor sets this.                               */

uchar  *grammap;/* Pointer to Grammer Map.                                */
                /* Kanji Monitor sets this.                               */

uchar  *kanadata;
                /* Pointer to yomi data.                                  */
                /* Kanji Monitor sets this.                               */

uchar  *kjdata1s;
                /* Pointer to First Conversion DBCS String.               */
                /* Kanji Monitor sets this.                               */

uchar  *kjmap1s;/* Pointer to First Conversion Map.                       */
                /* Kanji Monitor sets this.                               */

uchar  *gramap1s;
                /* Pointer to First Conversion Grammer Map.               */
                /* Kanji Monitor sets this.                               */

uchar  *iws1;   /* Internal Work String No.1                              */
                /* For Input Field Work Operation.                        */
                /* Kanji Monitor sets this.                               */

uchar  *iws2;   /* Internal Work String No.2                              */
                /* For Input Field Work Operation.                        */
                /* Kanji Monitor sets this.                               */

uchar  *iws3;   /* Internal Work String No.3                              */
                /* For Input Field Work Operation.                        */
                /* Kanji Monitor sets this.                               */

uchar  *iws4;   /* Internal Work String No.4                              */
                /* For Auxiliary Area Work Operation.                     */
                /* Kanji Monitor sets this.                               */

ulong  nextact; /* next action function indicate flag.                    */
                /* Kanji Monitor sets this.                               */

short  kjcvmax; /* Maximum length of Kanji Convertion Map.                */
                /* Kanji Monitor sets this.                               */

short  savemax; /* Maximim length of String Save Area.                    */
                /* Kanji Monitor sets this.                               */

short  kanabmax;/* Maximim length of Kana Bit Map.                        */
                /* Kanji Monitor sets this.                               */

short  grammax; /* Maximum length of Grammer Map.                         */
                /* Kanji Monitor sets this.                               */

short  kanamax; /* Maximum length of Kana Data.                           */
                /* Kanji Monitor sets this.                               */

short  dat1smax;/* Maximum length of First Kanji Data Area.               */
                /* Kanji Monitor sets this.                               */

short  map1smax;/* Maximim length of First Kanji Map Data Area.           */
                /* Kanji Monitor sets this.                               */

short  gra1smax;/* Maximum length of First Grammer Map Data Area.         */
                /* Kanji Monitor sets this.                               */

short  iws1max; /* Maximum length of Internal Work Area No.1              */
                /* ( For Input Field Operation ).                         */
                /* Kanji Monitor sets this.                               */

short  iws2max; /* Maximum length of Internal Work Area No.2              */
                /* ( For Input Field Operation ).                         */
                /* Kanji Monitor sets this.                               */

short  iws3max; /* Maximum length of Internal Work Area No.3              */
                /* ( For Auxiliary Area Operation ).                      */
                /* Kanji Monitor sets this.                               */

short  iws4max; /* Maximum length of Internal Work Area No.4              */
                /* ( For Auxiliary Area Operation ).                      */
                /* Kanji Monitor sets this.                               */

short  maxstc;  /* Maximum number of column                               */
                /* Kanji Monitor sets this.                               */

short  maxstr;  /* Maximum number of row.                                 */
                /* Kanji Monitor sets this.                               */
                /* Kanji Monitor sets this.                               */

short  kanalen; /* Yomi Map Length.                                       */
                /* Kanji Monitor sets this.                               */

short  kkmode1; /* Convert conversion mode No.1.                          */
                /* Kanji Monitor sets this.                               */

short  kkmode2; /* Convert conversion mode No.2.                          */
                /* Kanji Monitor sets this.                               */

short  rkclen;  /* Romaji/Kana Conversion Data length                     */
                /* Kanji Monitor sets this.                               */

short  regymlen;/* Length of  Dictionary Registration words.              */
                /* Kanji Monitor sets this.                               */

short  convpos; /* Conversion position                                    */
                /* Kanji Monitor sets this.                               */

short  convlen; /* Conversion Length.                                     */
                /* Kanji Monitor sets this.                               */

short  cconvpos;/* Current Conversion position                            */
                /* Kanji Monitor sets this.                               */

short  cconvlen;/* Current Conversion length                              */
                /* Kanji Monitor sets this.                               */

short  savepos; /* Replace mode savearea position                         */
                /* Kanji Monitor sets this.                               */

short  savelen; /* Replace mode String save length                        */
                /* Kanji Monitor sets this.                               */

short  chcodlen;/* Modify character number                                */
                /* Kanji Monitor sets this.                               */

short  preccpos;/* Previous Current Conversion.                           */
                /* Kanji Monitor sets this.                               */

short  kkcrc;   /* Kana Kanji Convertion Routine Return Code.             */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      short  allcstge[100];                                             */
/* New source.                                                            */
/*      short  allcstge[M_CANDNM];                                        */
/*========================================================================*/
short  allcstge[M_CANDNM];
                /* Number of each All candidates statge.                  */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      short  allcstgs[2];                                               */
/* New source.                                                            */
/*      short  allcstgs[M_SCNDNM];                                        */
/*========================================================================*/
short  allcstgs[M_SCNDNM];
                /* Number of each all candidates single kanji stage.      */
                /* Kanji Monitor sets this.                               */

short  alcancol;/* All Candidate indication col.                          */
                /* Kanji Monitor sets this.                               */

short  alcanrow;/* All Candidate indication row.                          */
                /* Kanji Monitor sets this.                               */

short  realcol; /* Maximum columns of line                                */
                /* Kanji Monitor sets this.                               */

short  curleft; /* Inside field movement limits of field Left Margin      */
                /* position.                                              */
                /* Kanji Monitor sets this.                               */

short  curright;/* Inside field movement limits of field Right Margin.    */
                /* position.                                              */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      uchar  rkcchar[4];                                                */
/* New source.                                                            */
/*      uchar  rkcchar[M_CVROKN];                                         */
/*========================================================================*/
uchar  rkcchar[M_CVROKN];
                /* Pointer to Romaji/Kana conversion Data                 */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      uchar  regyomi[12];                                               */
/* New source.                                                            */
/*      uchar  regyomi[M_RGYMA];                                          */
/*========================================================================*/
uchar  regyomi[M_RGYMA];
                /* Dictionary Registration 7bit kana code Work Area.      */
                /* Kanji Monitor sets this.                               */

/*========================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                     */
/* Changed source.                                                        */
/* Old source.                                                            */
/*      uchar  chcode[4];                                                 */
/* New source.                                                            */
/*      uchar  chcode[M_CHCODE];                                          */
/*========================================================================*/
uchar  chcode[M_CHCODE];
                /* Chrarcter code Information Code Interchange Work Area. */
                /* From Single Byte to DBCS.                              */
                /* Kanji Monitor sets this.                               */

char   kmact;   /* Field Active flag.                                     */
                /* =0 ... input field is not active.                      */
                /* =1 ... input field is ative.                           */
                /* Kanji Monitor sets this.                               */

char   hkmode;  /* Hiragana/Katakana Conversion mode.                     */
                /* =0 ... Reverse mode                                    */
                /* =1 ... Reverse and Reset mode                          */
                /* =2 ... Katakana mode                                   */
                /* Kanji Monitor sets this.                               */

char   kkcrmode;/* Dictionary Update Status.                              */
                /* =0 ... Not Dictionary Update status.                   */
                /* =1 ... Yomi Code Input status.                         */
                /* =2 ... Kanji Code Input staus.                         */
                /* =3 ... Dictionary Update status.                       */
                /* Kanji Monitor sets this.                               */

uchar  pscode;  /* pseudo code                                            */
                /* Kanji Monitor sets this.                               */

char   chmode;  /* Input Character Status                                 */
                /* =1 ... Hiragana.                                       */
                /* =2 ... Katakana.                                       */
                /* =3 ... Alaphabet.                                      */
                /* =4 ... Numberic.                                       */
                /* Kanji Monitor sets this.                               */

uchar  msetflg; /* Message Display Status Flag                            */
                /* b'1xxxxxxx' General Message.                           */
                /* b'x1xxxxxx' Dictionary Registration Message.           */
                /* Kanji Monitor sets this.                               */

uchar  auxflg1; /* Auxiliary area No.1 Use Message Number.                */
                /* Kanji Monitor sets this.                               */

uchar  auxflg2; /* **** RESERVE FOR FUTURE USE ****                       */
                /* Auxiliary area No.2 Use Message Number.                */
                /* Kanji Monitor sets this.                               */

uchar  auxflg3; /* **** RESERVE FOR FUTURE USE ****                       */
                /* Auxiliary area No.3 Use Message Number.                */
                /* Kanji Monitor sets this.                               */

uchar  auxflg4; /* **** RESERVE FOR FUTURE USE ****                       */
                /* Auxiliary area No.4 Use Message Number.                */
                /* Kanji Monitor sets this.                               */

uchar  actc1;   /* KMAT Action Code 1.                                    */
                /* Kanji Monitor sets this.                               */

uchar  actc2;   /* KMAT Action Code 2.                                    */
                /* Kanji Monitor sets this.                               */

uchar  actc3;   /* KMAT Action Code 3.                                    */
                /* Kanji Monitor sets this.                               */

char   sglcnvsw;/* Word Level Conversion Stage.                           */
                /* 0 ... Signle Kanji Conversion off.                     */
                /* 1 ... Single Kanji Conversion on.                      */
                /* Kanji Monitor sets this.                               */

char   convimp; /* Conversion impossible flag.                            */
                /* 0 ... Conversion Successful.                           */
                /* 1 ... Conversion unsuccessful.                         */
                /* Kanji Monitor sets this.                               */

uchar  charcont;/* Character continue flag                                */
                /* Kanji Monitor sets this.                               */

uchar  convnum; /* Count of conversion                                    */
                /* Kanji Monitor sets this.                               */

uchar  allcanfg;/* all candidate indication flag.                         */
                /* Kanji Monitor sets this.                               */

uchar  tankan;  /* Tankan Flag.                                           */
                /* Kanji Monitor sets this.                               */

uchar  knjnumfg;/* Kanji Number input area flag.                          */
                /* Kanji Monitor sets this.                               */

uchar  kkcflag; /* KKC Open Flag.                                         */
                /* 0 ... KKC wan't Opened.                                */
                /* 1 ... KKC was Opened.                                  */
                /* Kanji Monitor sets this.                               */

uchar  cvmdsw;  /* Conversion Mode Switch Destination flag                */
                /* 0 ... DBCS Input field.                                */
                /* 1 ... Auxiliary area No.1                              */
                /* Kanji Monitor sets this.                               */



/* #(B) 1987.12.15. Flying Conversion Add */
short  ax1lastc;/* Auxliary Area No.1 Last Character.                   */

short  ax2lastc;/* Auxliary Area No.2 Last Character.                   */

short  ax3lastc;/* Auxliary Area No.3 Last Character.                   */

short  ax4lastc;/* Auxliary Area No.4 Last Character.                   */

uchar  alcnmdfg;/* All Candidates Mode Flag.                            */

uchar  fconvflg;/* Flying Conversion Flag.                              */

uchar  fcnverfg;/* Flying Conversion Error Flag.                        */

short  kanalenf;/* Kanadata Length of Flying Conversion.                */

short  fconvpos;/* Flying Conversion Position.                          */

uchar  *kjdataf;/* Kanji Data of Flying Conversion.                     */

uchar  *kjmapf; /* Kanji Map of Flying Conversion.                      */

uchar  *gramapf;/* Grammer Map of Flying Conversion.                    */

uchar  *kanamapf;/* Kana Map of Flying Conversion.                      */

uchar  *kanadatf;/* Kana Data of Flying Conversion.                     */

short  datafmax;/* Length of Kanji Data of Flying Conversion.           */

short  mapfmax; /* Length of Kanji Map of Flying Conversion.            */

short  grafmax; /* Length of Grammer Map of Flying Conversion.          */

short  knmpfmax;/* Length of Kana Map of Flying Conversion.              */

short  kanafmax;/* Length of Kana Data of Flying Conversion.             */
/* #(E) 1987.12.15. Flying Conversion Add */

/* #(B) Sept. 20 1988 kjdataf etc. overflow support added by S,Higuchi  */
uchar  fcvovfg; /* Overflow flag of kjdataf etc.                        */

long   rsv1;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv2;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv3;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv4;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv5;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv6;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv7;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv8;    /* **** RESERVE FOR FUTURE USE ****                       */
};
#endif
