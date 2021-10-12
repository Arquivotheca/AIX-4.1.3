/* @(#)86	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jexm/exmkmisa.h, libKJI, bos411, 9428A410j 7/23/92 01:55:09 */
/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
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

/*
 *  ExMon version 6.0       06/28/88
 */
#ifndef _exm_KMISA
#define _exm_KMISA

/**********************************************************************/
/*      KMISA KANJI MONITOR INTERNAL SAVE AREA DEFINITION             */
/*                                                                    */
/**********************************************************************/

/* dict name structure */
typedef struct {
        char *sys;              /* system dict. file name               */
        char *user;             /* user dict. file name                 */
        char *adj;              /* adjunct dict. file name              */
} DNAMES;

typedef	struct	{
short  allcand; /* candidates display list format.                    */
				/* =0 ... Default candidates display mode.            */
				/* =1..255 ... Number of cadidates number.            */
				/* Kanji Monitor sets this.                           */

char   insert;  /* Insert/Replace method.                             */
				/* =1 ... Insert/Replace key.                         */
				/* =2 ... Reset  key.                                 */
				/* Kanji Monitor sets this.                           */

char  kjin;		/* Kanji Number Decide key.                           */
				/* b'xxx1' ... Action key active.(key position 64)    */
				/* b'xx1x' ... Enter  key active.(key position 43)    */
				/* b'x1xx' ... CR key active.    (key position 108)   */
				/* Kanji Monitor sets this.                           */

char  reset;	/* Previous Conversion Cancel Key.                    */
				/* b'xxx1' ... Action key active.(key position 64)    */
				/* b'xx1x' ... Enter  key active.(key position 43)    */
				/* b'x1xx' ... CR key active.    (key position 108)   */
				/* b'1xxx' ... Reset Key active. (key position 110)   */
				/* Kanji Monitor sets this.                           */

char   conversn;/* Kana to Kanji conversion algorithm                 */
				/* =1 ... Related Verb Conversion Mode                */
				/* =2 ... Simple  Verb Conversion Mode                */
				/* Kanji Monitor sets this.                           */

char   kjno;    /* Kanji Code type.                                   */
				/* =1 ... Japanese Industrial Standard Kuten Code     */
				/* =2 ... EBCDIC Code                                 */
				/* =3 ... IBM Kanji Code                              */
				/* =4 ... PC Kanji Code                               */
				/* Kanji Monitor sets this.                           */

char   aux1;    /* Kanji controle block auxiliary area No.1 default   */
				/* location initialize variable.                      */
				/* =0 ... Near cursor.                                */
				/* =1 ... Center.                                     */
				/* =2 ... Upper left.                                 */
				/* =3 ... Upper right.                                */
				/* =4 ... Lower left.                                 */
				/* =5 ... Lower right.                                */
				/* Kanji Monitor sets this.                           */

char   aux2;    /* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.2 default   */
				/* location initialize variable.                      */
				/* =0 ... Near cursor.                                */
				/* =1 ... Center.                                     */
				/* =2 ... Upper left.                                 */
				/* =3 ... Upper right.                                */
				/* =4 ... Lower left.                                 */
				/* =5 ... Lower right.                                */
				/* Kanji Monitor sets this.                           */

char   aux3;    /* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.3 default   */
				/* location initialize variable.                      */
				/* =0 ... Near cursor.                                */
				/* =1 ... Center.                                     */
				/* =2 ... Upper left.                                 */
				/* =3 ... Upper right.                                */
				/* =4 ... Lower left.                                 */
				/* =5 ... Lower right.                                */
				/* Kanji Monitor sets this.                           */

char   aux4;    /* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.4 default   */
				/* location initialize variable.                      */
				/* =0 ... Near cursor.                                */
				/* =1 ... Center.                                     */
				/* =2 ... Upper left.                                 */
				/* =3 ... Upper right.                                */
				/* =4 ... Lower left.                                 */
				/* =5 ... Lower right.                                */
				/* Kanji Monitor sets this.                           */

char   aux1maxc;/* Kanji controle block auxiliary area No.1 initialize*/
				/* variable. Set max cursor position of column        */
				/* Kanji Monitor sets this.                           */

char   aux1maxr;/* Kanji controle block auxiliary area No.1 initialize*/
				/* variable. Set max cursor position of row           */
				/* Kanji Monitor sets this.                           */

char   aux2maxc;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.2 initialize*/
				/* variable. Set max cursor position of column        */
				/* Kanji Monitor sets this.                           */

char   aux2maxr;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.2 initialize*/
				/* variable. Set max cursor position of row           */
				/* Kanji Monitor sets this.                           */

char   aux3maxc;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.3 initialize*/
				/* variable. Set max cursor position of column        */
				/* Kanji Monitor sets this.                           */

char   aux3maxr;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.3 initialize*/
				/* variable. Set max cursor position of row           */
				/* Kanji Monitor sets this.                           */

char   aux4maxc;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.4 initialize*/
				/* variable. Set max cursor position of column        */
				/* Kanji Monitor sets this.                           */

char   aux4maxr;/* **** RESERVED FOR FUTURE USE ****                  */
				/* Kanji controle block auxiliary area No.4 initialize*/
				/* variable. Set max cursor position of row           */
				/* Kanji Monitor sets this.                           */

char   beep;    /* Beep available flag.                               */
				/* =0 ... can not use to beep.                        */
				/* =1 ... can use to beep.                            */
				/* Kanji Monitor sets this.                           */

char   regist;  /* Interactive Dictionary Update flag.                */
				/* =1 ... Interactive Dic. Update Enable              */
				/* =2 ... interactive Dic. Update Disable             */
				/* Kanji Monitor sets this.                           */

char   mix;     /* Single Byte Character and Double Byte Chracter Sets*/
				/* can mixed flag.                                    */
				/* =0 ... Mixed Not Allow.                            */
				/* =1 ... Mixed Allow.                                */

char   indicat; /* Kanji controle block indle initialize variable     */
				/* Byte length of shift indicators at the end of      */
				/* the input field.                                   */
				/* This must be 4 for DBCS Editor.                    */
				/* Kanji Monitor sets this.                           */

char   rkc;     /* Romaji/kana Conversion initialal mode.             */
				/* =0 ... No Conversion Mode.                         */
				/* =1 ... Romaji(Alphabet)/kana Conversion Mode.      */
				/* Kanji Monitor sets this.                           */

char   kana;    /* Kana/Hiragena Initiall status.                     */
				/* =0 ... Kana mode.                                  */
				/* =1 ... Hiragana mode.                              */
				/* Kanji Monitor sets this.                           */

char   kblock;  /* Keyboard lock mode.                                */
				/* =0 ... Keyboard lock disable.                      */
				/* =1 ... Keyboard lock enable.                       */
				/* Kanji Monitor sets this.                           */

char   cursout; /* cusror movement area.(region)                      */
				/* =0 ... Cursor Movement Only field inside.          */
				/* =1 ... Cursor Movement anywhere.                   */
				/* Kanji Monitor sets this.                           */

char   katakana;/* Katakana Conversion mode.                          */
				/* =0 ... Katakana Conversion mode disable.           */
				/* =1 ... katakana Conversion mode enable.            */
				/* Kanji Monitor sets this.                           */

char   alphanum;/* Alpahabet Character have an effect on  Kana/Kanji  */
				/* Conversion candidates.                             */
				/* =0 ... to no effect.                               */
				/* =1 ... to effect.                                  */
				/* Kanji Monitor sets this.                           */

char   learning;/* Interactive Dictionary learning flag.              */
				/* =0 ... Not learning.                               */
				/* =1 ... Learning mode.                              */
				/* Kanji Monitor sets this.                           */

char   kanjinum;/* Numeric Character have an effect on  Kana/Kanji    */
				/* Conversion candidates.                             */
				/* =0 ... to no effect.                               */
				/* =1 ... to effect.                                  */
				/* Kanji Monitor sets this.                           */

char   pfkreset;/* Dictionary Registration MOde Exit Condition.           */
                /* =0 ... Return to Application.                          */
                /* =1 ... Continue Dictionary Registration.               */
char   convtype;/* Conversion Type.                                       */
char    kuten;  /* Kuten Number Mode      				  */
char   shmatnum;/* =N ... JIM uses mapped files less than N               */
char    udload; /* =0 ... User dictionary is never loaded after _Jopen()  */
                /* =1 ... User dictionary is loaded if it has been updated*/
char    modereset;      /* =0 ... ESC key in NOT used to reset input mode */
                        /* =1 ... ESC key is used to reset input mode     */
char    rsv0[3];/* **** RESERVED FOR FUTURE USE ****                      */
DNAMES  dnames;         /* dict name str address                          */
long    rsv7;   /* **** RESERVED FOR FUTURE USE ****                      */
long    rsv8;	/* **** RESERVED FOR FUTURE USE ****                      */
long    rsv9;   /* **** RESERVED FOR FUTURE USE ****                      */
long    rsv10;  /* **** RESERVED FOR FUTURE USE ****                      */
}	KMPF;


/*
 *      Kanji Monitor INternal Save Area.
 */

typedef	struct	{
long   length;  /* Length of Kanji Internal Save Area(byte)               */
				/* Kanji Monitor sets this.                               */

long   kmisaid; /* Kanji Monitor Internal Save Area Id.                   */
				/* Kanji Monitor Sets this.                               */

int    *kkcbsvpt;
				/* Pointer to Kana/Kanji Control Block Save Area          */
				/* Kanji Kanji Routine sets this.                         */

int    *ifsaveo;/* Pointer to Display String Save area for Display        */
				/* Message                                                */
				/* Kanji Monitor sets this.                               */

int    *ifsaved;/* Pointer to Display String Save area for Display        */
				/* Dictionary regist Message                              */
				/* Kanji Monitor sets this.                               */

KMPF   kmpf[2]; /* Kanji Monitor Profile Save Area.                       */
				/* kmpf[0] ... Profile Data.                              */
				/* kmpf[1] ... Profile Control Data.                      */
				/* Kanji Monitor sets this.                               */

char  *kjcvmap;	/* pointer Kanji Conversion Map.                          */
				/* Kanji Monitor sets this.                               */
char  *stringsv;
				/* Pointer to Replace Mode Background String Save Area    */
				/* Kanji Monitor sets this.                               */

char  *kanamap;
				/* Pointer to Kana Kanji Conversion Kana Map.             */
				/* Kanji Monitor sets this.                               */

char  *grammap;	/* Pointer to Grammer Map.                                */
				/* Kanji Monitor sets this.                               */

char  *kanadata;
				/* Pointer to yomi data.                                  */
				/* Kanji Monitor sets this.                               */

char  *kjdata1s;
				/* Pointer to First Conversion DBCS String.               */
				/* Kanji Monitor sets this.                               */

char  *kjmap1s;	/* Pointer to First Conversion Map.                       */
				/* Kanji Monitor sets this.                               */

char  *gramap1s;
				/* Pointer to First Conversion Grammer Map.               */
				/* Kanji Monitor sets this.                               */

char  *iws1;	/* Internal Work String No.1                              */
				/* For Input Field Work Operation.                        */
				/* Kanji Monitor sets this.                               */

char  *iws2;	/* Internal Work String No.2                              */
				/* For Input Field Work Operation.                        */
				/* Kanji Monitor sets this.                               */

char  *iws3;	/* Internal Work String No.3                              */
				/* For Input Field Work Operation.                        */
				/* Kanji Monitor sets this.                               */

char  *iws4;	/* Internal Work String No.4                              */
				/* For Auxiliary Area Work Operation.                     */
				/* Kanji Monitor sets this.                               */

long  nextact;	/* next action function indicate flag.                    */
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

short  allcstge[100];
				/* Number of each All candidates statge.                  */
				/* Kanji Monitor sets this.                               */

short  allcstgs[2];
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

char  rkcchar[4];
				/* Pointer to Romaji/Kana conversion Data                 */
				/* Kanji Monitor sets this.                               */

char  regyomi[12];
				/* Dictionary Registration 7bit kana code Work Area.      */
				/* Kanji Monitor sets this.                               */

char  chcode[4];
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

char  pscode;	/* pseudo code                                            */
				/* Kanji Monitor sets this.                               */

char   chmode;  /* Input Character Status                                 */
				/* =1 ... Hiragana.                                       */
				/* =2 ... Katakana.                                       */
				/* =3 ... Alaphabet.                                      */
				/* =4 ... Numberic.                                       */
				/* Kanji Monitor sets this.                               */

char  msetflg;	/* Message Display Status Flag                            */
				/* b'1xxxxxxx' General Message.                           */
				/* b'x1xxxxxx' Dictionary Registration Message.           */
				/* Kanji Monitor sets this.                               */

char  auxflg1;	/* Auxiliary area No.1 Use Message Number.                */
				/* Kanji Monitor sets this.                               */

char  auxflg2;	/* **** RESERVE FOR FUTURE USE ****                       */
				/* Auxiliary area No.2 Use Message Number.                */
				/* Kanji Monitor sets this.                               */

char  auxflg3;	/* **** RESERVE FOR FUTURE USE ****                       */
				/* Auxiliary area No.3 Use Message Number.                */
				/* Kanji Monitor sets this.                               */

char  auxflg4;	/* **** RESERVE FOR FUTURE USE ****                       */
				/* Auxiliary area No.4 Use Message Number.                */
				/* Kanji Monitor sets this.                               */

char  actc1;	/* KMAT Action Code 1.                                    */
				/* Kanji Monitor sets this.                               */

char  actc2;	/* KMAT Action Code 2.                                    */
				/* Kanji Monitor sets this.                               */

char  actc3;	/* KMAT Action Code 3.                                    */
				/* Kanji Monitor sets this.                               */

char   sglcnvsw;/* Word Level Conversion Stage.                           */
				/* 0 ... Signle Kanji Conversion off.                     */
				/* 1 ... Single Kanji Conversion on.                      */
				/* Kanji Monitor sets this.                               */

char   convimp; /* Conversion impossible flag.                            */
				/* 0 ... Conversion Successful.                           */
				/* 1 ... Conversion unsuccessful.                         */
				/* Kanji Monitor sets this.                               */

char  charcont;	/* Character continue flag                                */
				/* Kanji Monitor sets this.                               */

char  convnum;	/* Count of conversion                                    */
				/* Kanji Monitor sets this.                               */

char  allcanfg;	/* all candidate indication flag.                         */
				/* Kanji Monitor sets this.                               */

char  tankan;	/* Tankan Flag.                                           */
				/* Kanji Monitor sets this.                               */

char  knjnumfg;	/* Kanji Number input area flag.                          */
				/* Kanji Monitor sets this.                               */

char  kkcflag;	/* KKC Open Flag.                                         */
				/* 0 ... KKC wan't Opened.                                */
				/* 1 ... KKC was Opened.                                  */
				/* Kanji Monitor sets this.                               */

char  cvmdsw;	/* Conversion Mode Switch Destination flag                */
				/* 0 ... DBCS Input field.                                */
				/* 1 ... Auxiliary area No.1                              */
				/* Kanji Monitor sets this.                               */

short  ax1lastc;/* Auxliary Area No.1 Last Character.                     */
short  ax2lastc;/* Auxliary Area No.2 Last Character.                     */
short  ax3lastc;/* Auxliary Area No.3 Last Character.                     */
short  ax4lastc;/* Auxliary Area No.4 Last Character.                     */
char  alcnmdfg; /* All Candidates Mode Flag.                              */
char  fconvflg; /* Flying Conversion Flag.                                */
char  fcnverfg; /* Flying Conversion Error Flag.                          */
short  kanalenf;/* Kanadata Length of Flying Conversion.                  */
short  fconvpos;/* Flying Conversion Position.                            */
char  *kjdataf; /* Kanji Data of Flying Conversion.                       */
char  *kjmapf;  /* Kanji Map of Flying Conversion.                        */
char  *gramapf; /* Grammer Map of Flying Conversion.                      */
char  *kanamapf;/* Kana Map of Flying Conversion.                         */
char  *kanadatf;/* Kana Data of Flying Conversion.                        */
short  datafmax;/* Length of Kanji Data of Flying Conversion.             */
short  mapfmax; /* Length of Kanji Map of Flying Conversion.              */
short  grafmax; /* Length of Grammer Map of Flying Conversion.            */
short  knmpfmax;/* Length of Kana Map of Flying Conversion.               */
short  kanafmax;/* Length of Kana Data of Flying Conversion.              */
char   fcvovfg; /* Overflow flag of kjdataf etc.                          */
long   rsv1;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv2;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv3;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv4;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv5;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv6;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv7;    /* **** RESERVE FOR FUTURE USE ****                       */
long   rsv8;    /* **** RESERVE FOR FUTURE USE ****                       */
}	KMISA;
#endif /* _exm_KMISA */
