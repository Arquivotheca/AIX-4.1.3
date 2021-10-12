/* @(#)83	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/kmpf.h, libKJI, bos411, 9428A410j 7/23/92 03:25:57 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
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

#ifndef _kj_KMPF
#define _kj_KMPF

#include "kjmacros.h"   /* Kanji Project Macros.                          */
#include "kmpfdef.h"    /* Kanji Monitor Profile Structure Define.        */

/* dict name structure */
typedef struct {
	char *sys;          	/* system dict. file name 		*/
	char *user;         	/* user dict. file name   		*/
	char *adj;          	/* adjunct dict. file name		*/
} DNAMES;

/*
 *      Create Profile Structure General Name.
 */
typedef struct profblk KMPF;
/*
 *      Kanji Monitor Profile Structure.
 */
struct profblk {
short  allcand; /* candidates display list format.                        */
                /* =0 ... Default candidates display mode.                */
                /* =1..32767 .. Number of cadidates number.               */
                /* Kanji Monitor sets this.                               */

char   insert;  /* Insert/Replace method.                                 */
                /* =1 ... Insert/Replace key.                             */
                /* =2 ... Reset  key.                                     */
                /* Kanji Monitor sets this.                               */

uchar  kjin;    /* Kanji Number Decide key.                               */
                /* b'xxx1' ... Action key active.(key position 64)        */
                /* b'xx1x' ... Enter  key active.(key position 43)        */
                /* b'x1xx' ... CR key active.    (key position 108)       */
                /* Kanji Monitor sets this.                               */

uchar  reset;   /* Previous Conversion Cancel Key.                        */
                /* b'xxx1' ... Action key active.(key position 64)        */
                /* b'xx1x' ... Enter  key active.(key position 43)        */
                /* b'x1xx' ... CR key active.    (key position 108)       */
                /* b'1xxx' ... Reset Key active. (key position 110)       */
                /* Kanji Monitor sets this.                               */

char   conversn;/* Kana to Kanji conversion algorithm                     */
                /* =1 ... Related Verb Conversion Mode                    */
                /* =2 ... Simple  Verb Conversion Mode                    */
                /* =3 ... Multi   Verb Conversion Mode.                   */
                /* Kanji Monitor sets this.                               */

char   kjno;    /* Kanji Code type.                                       */
                /* =1 ... Japanese Industrial Standard Kuten Code         */
                /* =2 ... EBCDIC Code                                     */
                /* =3 ... IBM Kanji Code                                  */
                /* =4 ... PC Kanji Code                                   */
                /* Kanji Monitor sets this.                               */

char   aux1;    /* Kanji control  block auxiliary area No.1 default       */
                /* location initialize variable.                          */
                /* =0 ... Near cursor.                                    */
                /* =1 ... Center.                                         */
                /* =2 ... Upper left.                                     */
                /* =3 ... Upper right.                                    */
                /* =4 ... Lower left.                                     */
                /* =5 ... Lower right.                                    */
                /* Kanji Monitor sets this.                               */

char   aux2;    /* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.2 default       */
                /* location initialize variable.                          */
                /* =0 ... Near cursor.                                    */
                /* =1 ... Center.                                         */
                /* =2 ... Upper left.                                     */
                /* =3 ... Upper right.                                    */
                /* =4 ... Lower left.                                     */
                /* =5 ... Lower right.                                    */
                /* Kanji Monitor sets this.                               */

char   aux3;    /* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.3 default       */
                /* location initialize variable.                          */
                /* =0 ... Near cursor.                                    */
                /* =1 ... Center.                                         */
                /* =2 ... Upper left.                                     */
                /* =3 ... Upper right.                                    */
                /* =4 ... Lower left.                                     */
                /* =5 ... Lower right.                                    */
                /* Kanji Monitor sets this.                               */

char   aux4;    /* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.4 default       */
                /* location initialize variable.                          */
                /* =0 ... Near cursor.                                    */
                /* =1 ... Center.                                         */
                /* =2 ... Upper left.                                     */
                /* =3 ... Upper right.                                    */
                /* =4 ... Lower left.                                     */
                /* =5 ... Lower right.                                    */
                /* Kanji Monitor sets this.                               */

char   aux1maxc;/* Kanji control  block auxiliary area No.1 initialize    */
                /* variable. Set max cursor position of column            */
                /* Kanji Monitor sets this.                               */

char   aux1maxr;/* Kanji control  block auxiliary area No.1 initialize    */
                /* variable. Set max cursor position of row               */
                /* Kanji Monitor sets this.                               */

char   aux2maxc;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.2 initialize    */
                /* variable. Set max cursor position of column            */
                /* Kanji Monitor sets this.                               */

char   aux2maxr;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.2 initialize    */
                /* variable. Set max cursor position of row               */
                /* Kanji Monitor sets this.                               */

char   aux3maxc;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.3 initialize    */
                /* variable. Set max cursor position of column            */
                /* Kanji Monitor sets this.                               */

char   aux3maxr;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.3 initialize    */
                /* variable. Set max cursor position of row               */
                /* Kanji Monitor sets this.                               */

char   aux4maxc;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.4 initialize    */
                /* variable. Set max cursor position of column            */
                /* Kanji Monitor sets this.                               */

char   aux4maxr;/* **** RESERVED FOR FUTURE USE ****                      */
                /* Kanji control  block auxiliary area No.4 initialize    */
                /* variable. Set max cursor position of row               */
                /* Kanji Monitor sets this.                               */

char   beep;    /* Beep available flag.                                   */
                /* =0 ... can not use to beep.                            */
                /* =1 ... can use to beep.                                */
                /* Kanji Monitor sets this.                               */

char   regist;  /* Interactive Dictionary Update flag.                    */
                /* =1 ... Interactive Dic. Update Enable                  */
                /* =2 ... interactive Dic. Update Disable                 */
                /* Kanji Monitor sets this.                               */

char   mix;     /* Single Byte Character and Double Byte Chracter Sets    */
                /* can mixed flag.                                        */
                /* =0 ... Mixed Not Allow.                                */
                /* =1 ... Mixed Allow.                                    */

char   indicat; /* Kanji control  block indle initialize variable         */
                /* Byte length of shift indicators at the end of          */
                /* the input field.                                       */
                /* This must be 0 or 4 for DBCS Editor.                   */
                /* Kanji Monitor sets this.                               */

char   rkc;     /* Romaji/Kana Conversion initialal mode.                 */
                /* =0 ... No Conversion Mode.                             */
                /* =1 ... Romaji(Alphabet)/kana Conversion Mode.          */
                /* Kanji Monitor sets this.                               */

char   kana;    /* Kana/Hiragena Initiall status.                         */
                /* =0 ... Kana mode.                                      */
                /* =1 ... Hiragana mode.                                  */
                /* Kanji Monitor sets this.                               */

char   kblock;  /* Keyboard lock mode.                                    */
                /* =0 ... Keyboard lock disable.                          */
                /* =1 ... Keyboard lock enable.                           */
                /* Kanji Monitor sets this.                               */

char   cursout; /* cusror movement area.(region)                          */
                /* =0 ... Cursor Movement Only field inside.              */
                /* =1 ... Cursor Movement anywhere.                       */
                /* Kanji Monitor sets this.                               */

char   katakana;/* Katakana Conversion mode.                              */
                /* =0 ... Katakana Conversion mode disable.               */
                /* =1 ... katakana Conversion mode enable.                */
                /* Kanji Monitor sets this.                               */

char   alphanum;/* Alpahabet Character have an effect on  Kana/Kanji      */
                /* Conversion candidates.                                 */
                /* =0 ... to no effect.                                   */
                /* =1 ... to effect.                                      */
                /* Kanji Monitor sets this.                               */

char   learning;/* Interactive Dictionary learning flag.                  */
                /* =0 ... Not learning.                                   */
                /* =1 ... Learning mode.                                  */
                /* Kanji Monitor sets this.                               */

char   kanjinum;/* Numeric Character have an effect on  Kana/Kanji        */
                /* Conversion candidates.                                 */
                /* =0 ... to no effect.                                   */
                /* =1 ... to effect.                                      */
                /* Kanji Monitor sets this.                               */

char   pfkreset;/* Dictionary Registration MOde Exit Condition.           */
                /* =0 ... Return to Application.                          */
                /* =1 ... Continue Dictionary Registration.               */



/* #(B) 1987.12.15. Flying Conversion Add */
char   convtype;/* Conversion Type.                                       */
/* #(E) 1987.12.15. Flying Conversion Add */


char	kuten;	/* Kuten Number Mode	  */ 

char	shmatnum;/* =N ... JIM uses mapped files less than N	  	  */
char	udload;	/* =0 ... User dictionary is never loaded after _Jopen()  */
		/* =1 ... User dictionary is loaded if it has been updated*/
char	modereset;	/* =0 ... ESC key in NOT used to reset input mode */
		   	/* =1 ... ESC key is used to reset input mode	  */
char	rsv0[3];/* **** RESERVED FOR FUTURE USE ****                      */
DNAMES	dnames;		/* dict name str address			  */
long    rsv7;   /* **** RESERVED FOR FUTURE USE ****                      */
long    rsv8;   /* **** RESERVED FOR FUTURE USE ****                      */
long    rsv9;   /* **** RESERVED FOR FUTURE USE ****                      */
long    rsv10;  /* **** RESERVED FOR FUTURE USE ****                      */

};
#endif
