/* @(#)85	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jexm/exmkcb.h, libKJI, bos411, 9428A410j 7/23/92 01:51:15 */
/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
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

/*
 *  ExMon version 6.0       06/28/88
 */

/**********************************************************************/
/*      KCB KANJI CONTROLE BLOCK STRUCTUE                             */
/*                                                                    */
/**********************************************************************/

typedef int		TRB;

typedef struct kjcblk KCB;

struct kjcblk {
long   length;  /* Length of Kanji Controle Block(byte).              */
                /* Kanji Monitor Sets this.                           */

long   id;      /* Kanji Monitor Id.                                  */
                /* Kanji Monitor Sets this.                           */

KMISA  *kjsvpt; /* Pointer to Kanji Monitor Internal Save Area.(KMISA)*/
                /* When Kanji is opened, Kanji Monitor allocates the  */
                /* memory for its internal data save area and saves   */
                /* the pointer ot it here.                            */

TRB    *tracep; /* Pointer to Trace Area.(TRB)                        */
                /* Kanji monitor sets this.                           */

char  *string; /* Pointer to whole input string.                     */
                /* It points to an active input field.                */
		/* DBCS Editor Sets this.                             */

char  *hlatst; /* Pointer to whole input string highlighting         */
                /* attribute.                                         */
                /* Kanji monitor sets this.                           */

char  *aux1;   /* Pointer to auxiliary area No.1.                    */
                /* Kanji monitor sets this.                           */

char  *hlata1; /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.1 Highlighting        */
                /* attribute.                                         */
                /* Kanji monitor sets this.                           */

char  *aux2;   /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.2.                    */
                /* Kanji monitor sets this.                           */

char  *hlata2; /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.2 Highlighting        */
                /* attribute.                                         */
                /* Kanji monitor sets this.                           */

char  *aux3;   /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.3.                    */
                /* Kanji monitor sets this.                           */

char  *hlata3; /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.3 Highlighting        */
                /* attribute.                                         */
                /* Kanji monitor sets this.                           */

char  *aux4;   /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.4.                    */
                /* Kanji monitor sets this.                           */

char  *hlata4; /* **** RESERVED FOR FUTURE USE ****                  */
                /* Pointer to auxiliary area No.4 Highlighting        */
                /* attribute.                                         */
                /* Kanji monitor sets this.                           */

char  *auxdir; /* Pointer to auiliary area.                          */

short  csid;    /* Character set id.                                  */
                /* DBCS Editor Calls DBCS Monitor with a parameter of */
                /* CSID and Kanji Monitor sets it here. It must be    */
                /* 370 for Kanji. After then DBCS Monitor always      */
                /* reads it to decide what language monitor it        */
                /* should call.                                       */

short  actcol;  /* The number (byte) of the columns in the active     */
                /* input field.                                       */
                /* Until rectanguler input area becomes available     */
                /* this means the length of the active input field.   */
                /* DBCS Editor sets this.                             */

short  actrow;  /* The number of the rows in the active input field.  */
                /* DBCS Editor sets  1 to this.                       */

short  ax1col;  /* The number (byte) of the columns in the auxiliary  */
                /* area No.1.                                         */
                /* Kanji Monitor sets this.                           */

short  ax1row;  /* The number of the rows in the auxiliary area No.1. */
                /* Kanji Monitor sets this.                           */

short  ax2col;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number (byte) of the columns in the auxiliary  */
                /* area No.2.                                         */
                /* Kanji Monitor sets this.                           */

short  ax2row;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number of the rows in the auxiliary area No.2. */
                /* Kanji Monitor sets this.                           */

short  ax3col;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number (byte) of the columns in the auxiliary  */
                /* area No.3.                                         */
                /* Kanji Monitor sets this.                           */

short  ax3row;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number of the rows in the auxiliary area No.3. */
                /* Kanji Monitor sets this.                           */

short  ax4col;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number (byte) of the columns in the auxiliary  */
                /* area No.4.                                         */
                /* Kanji Monitor sets this.                           */

short  ax4row;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* The number of the rows in the auxiliary area No.4. */
                /* Kanji Monitor sets this.                           */

short  maxa1c;  /* Maximum number(byte) of columns of auxiliary       */
		/* area No.1.                                         */
		/* The value is a smaller one of either of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa1r;  /* Maximum number of rows of auxiliaru area No.1.     */
		/* The value is a smaller one of eihter of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa2c;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number(byte) of columns of auxiliary       */
		/* area No.2.                                         */
		/* The value is a smaller one of either of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa2r;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number of rows of auxiliaru area No.2.     */
		/* The value is a smaller one of eihter of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa3c;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number(byte) of columns of auxiliary       */
		/* area No.3.                                         */
		/* The value is a smaller one of either of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa3r;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number of rows of auxiliaru area No.3.     */
		/* The value is a smaller one of eihter of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa4c;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number(byte) of columns of auxiliary       */
		/* area No.4.                                         */
		/* The value is a smaller one of either of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  maxa4r;  /* **** RESERVED FOR FUTURE USE ****                  */
		/* Maximum number of rows of auxiliaru area No.4.     */
		/* The value is a smaller one of eihter of the one    */
		/* specified by DBCS Editor(deopen) or specified
		/* Kanji Monitor sets this.                           */

short  curcol;  /* Cursor position (offset) in the input line in byte.*/
                /* In the Double byte case (not mixed of single and   */
                /* double byte code), it must be even number          */
                /* (0, 4, 4 ...). DBCS Editor sets the initial        */
                /* position when it initializes the input field and   */
                /* after then Kanji Monitor sets the new position.    */

short  currow;  /* Cursor position in row of the input rectangular    */
                /* area. DBCS Editor sets the initial position when it*/
                /* initializes the input field and after then kanji   */
                /* Monitor sets the new position.                     */

short  setcsc;  /* Cursor position (offset) in the input line in byte.*/
                /* It is used for DBCS Editor to set the cursor       */
                /* position directly.                                 */
                /* DBCS Editor sets this.                             */

short  setcsr;  /* Cursor position in row of the input rectangular    */
                /* area. It is used for DBCS Editor to set the        */
                /* cursor posotion directly.                          */
                /* DBCS Editor sets this.                             */

short  cura1c;  /* Pseudo Cursor potition (offset) in the line of     */
                /* auxiliary area No.1 in byte.                       */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.1.                                         */
                /* Kanji Monitor sets this.                           */

short  cura1r;  /* Pseudo cursor position in row of the auxiliary     */
                /* area No.1.                                         */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.1.                                         */
                /* Kanji Monitor sets this.                           */

short  cura2c;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo Cursor potition (offset) in the line of     */
                /* auxiliary area No.2 in byte.                       */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.2.                                         */
                /* Kanji Monitor sets this.                           */

short  cura2r;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo cursor position in row of the auxiliary     */
                /* area No.2.                                         */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.2.                                         */
                /* Kanji Monitor sets this.                           */

short  cura3c;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo Cursor potition (offset) in the line of     */
                /* auxiliary area No.3 in byte.                       */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.3.                                         */
                /* Kanji Monitor sets this.                           */

short  cura3r;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo cursor position in row of the auxiliary     */
                /* area No.3.                                         */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.3.                                         */
                /* Kanji Monitor sets this.                           */

short  cura4c;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo Cursor potition (offset) in the line of     */
                /* auxiliary area No.4 in byte.                       */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.4.                                         */
                /* Kanji Monitor sets this.                           */

short  cura4r;  /* ***** RESERVED FOR FUTURE USE ****                 */
                /* Pseudo cursor position in row of the auxiliary     */
                /* area No.4.                                         */
                /* -1 means there is no pseudo cursor in auxiliary    */
                /* area No.4.                                         */
                /* Kanji Monitor sets this.                           */

short  chpos;   /* Changed character position (from/offset).          */
                /* Kanji Monitor sets this to request DBCS Editor to  */
                /* redraw the input chracters (echo) from this        */
                /* position.                                          */

short  chlen;   /* Changed character length in byte.                  */
                /* Kanji Monitor sets this request DBCS Editor to     */
                /* redraw the input characters (echo) of this length. */

short  chpsa1;  /* Changed character position of auxiliary area No.1. */
                /* Kanji Monitor sets this to requests DBCS Editor to */
                /* redraw the characters in auxiliary area No.1.      */

short  chlna1;  /* Changed character byte length of auxiliary area    */
                /* No.1.                                              */
                /* Kanji Monitor sets this to requests DBCS Editor to */
                /* redraw the characters in auxiliary area No.1.      */

short  chpsa2;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character position of auxiliary area No.2. */
                /* Kanji Monitor sets this.                           */

short  chlna2;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character byte length of auxiliary area    */
                /* No.2.                                              */
                /* Kanji Monitor sets this.                           */

short  chpsa3;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character position of auxiliary area No.3. */
                /* Kanji Monitor sets this.                           */

short  chlna3;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character byte length of auxiliary area    */
                /* No.3.                                              */
                /* Kanji Monitor sets this.                           */

short  chpsa4;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character position of auxiliary area No.4. */
                /* Kanji Monitor sets this.                           */

short  chlna4;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Changed character byte length of auxiliary area    */
                /* No.4.                                              */
                /* Kanji Monitor sets this.                           */

short  lastch;  /* Last character position.                           */
                /* Kanji Monitor sets this.                           */

char   type;    /* Input code type.                                   */
                /* =1 ... Input code is a characte code.              */
                /*        (ASCII/JISCII)                              */
                /* =2 ... Input code is a binary data.                */
                /*        (function code/Escape sequence)             */
                /* =3 ... Input code is a binary data.                */
                /* DBCS Editor sets this.                             */

char  code;    /* Input code.                                        */
                /* DBCS  Editror sets this. it is a character code,   */
                /* pseudo code, or binary data.                       */

char   flatsd;  /* Input field attribute of single/double byte.       */
                /* =1 ... Only double byte code can be input.         */
                /* =2 ... Single and double byte code can be mixed.   */
                /* DBCS Editor sets this.                             */

char   axuse1;  /* Auxiliary area No.1. use flag.                     */
                /* =0 ... Auxiliary area No.1 is not in use.          */
                /* =1 ... AUxiliary area NO.1 is in use.              */
                /* Kanji Monitor sets this.                           */

char   axuse2;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.2. use flag.                     */
                /* =0 ... Auxiliary area No.2 is not in use.          */
                /* =1 ... AUxiliary area NO.2 is in use.              */
                /* Kanji Monitor sets this.                           */

char   axuse3;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.3. use flag.                     */
                /* =0 ... Auxiliary area No.3 is not in use.          */
                /* =1 ... AUxiliary area NO.3 is in use.              */
                /* Kanji Monitor sets this.                           */

char   axuse4;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.4. use flag.                     */
                /* =0 ... Auxiliary area No.4 is not in use.          */
                /* =1 ... AUxiliary area NO.4 is in use.              */
                /* Kanji Monitor sets this.                           */

char   ax1loc;  /* Auxiliary area No.1 default location.              */
                /* =0 ... Near cursor.                                */
                /* =1 ... Center.                                     */
                /* =2 ... Upper left.                                 */
                /* =3 ... Upper right.                                */
                /* =4 ... Lower left.                                 */
                /* =5 ... Lower right.                                */
                /* Kanji Monitor sets this.                           */

char   ax2loc;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.2 default location.              */
                /* =0 ... Near cursor.                                */
                /* =1 ... Center.                                     */
                /* =2 ... Upper left.                                 */
                /* =3 ... Upper right.                                */
                /* =4 ... Lower left.                                 */
                /* =5 ... Lower right.                                */
                /* Kanji Monitor sets this.                           */

char   ax3loc;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.3 default location.              */
                /* =0 ... Near cursor.                                */
                /* =1 ... Center.                                     */
                /* =2 ... Upper left.                                 */
                /* =3 ... Upper right.                                */
                /* =4 ... Lower left.                                 */
                /* =5 ... Lower right.                                */
                /* Kanji Monitor sets this.                           */

char   ax4loc;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Auxiliary area No.4 default location.              */
                /* =0 ... Near cursor.                                */
                /* =1 ... Center.                                     */
                /* =2 ... Upper left.                                 */
                /* =3 ... Upper right.                                */
                /* =4 ... Lower left.                                 */
                /* =5 ... Lower right.                                */
                /* Kanji Monitor sets this.                           */

char   indlen;  /* Byte length of shift indicators at the end of      */
                /* the input field.                                   */
                /* This must be 4 for DBCS Editor.                    */
                /* Kanji Monitor sets this.                           */

char  shift;   /* Shift change flag.                                 */
                /* =0 ... Any shift states has not been changed       */
                /* =b'xxx1' ... shift1 state has been changed.        */
                /* =b'xx1x' ... shift2 state has been changed.        */
                /* =b'x1xx' ... shift3 state has been changed.        */
                /* =b'1xxx' ... shift4 state has been changed.        */

char   shift1;  /* AN/KANA sift.                                      */
                /* =0 ... Undefined.                                  */
                /* =1 ... Alpha-Numberic shift                        */
                /* =3 ... Hiragana shift                              */
                /* Kanji Monitor sets this.                           */

char   shift2;  /* RKC shift.                                         */
                /* =1 ... RKC ON.                                     */
                /* =2 ... RKC OFF.                                    */
                /* Kanji Monitor sets this.                           */

char   shift3;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Single/Double byte shift.                          */
                /* =1 ... Single byte shift.                          */
                /* =2 ... Double byte shift.                          */
                /* Kanji Monitor sets this.                           */

char   shift4;  /* * RESERVED FOR FUTURE USE (OTHER THAN DBCS Editor) */
                /* Shift 4.                                           */
                /* Kanji Monitor sets this.                           */

char   curlen;  /* **** RESERVED FOR FUTURE USE ****                  */
                /* Cursor length.                                     */
                /* =1 ... Cursor is at single byte code.              */
                /* =2 ... Cursor is at double byte code.              */
                /* Kanji Monitor sets this.                           */

char   cnvsts;  /* Conversion state.                                  */
                /* =0 ... Conversion is finished.                     */
                /* =1 ... Conversion is going on.                     */
                /* Kanji Monitor sets this.                           */

char   repins;  /* Replace/insert flag.                               */
                /* =0 ... Replace mode.                               */
                /* =1 ... Insert  mode.                               */
                /* DBCS Editor sets current mode here when it         */
                /* initializes Kanji inputfield. After then           */
                /* Kanji Monitor sets this to request DBCS Editor to  */
                /* display corrent shaped cursor.                     */

char   beep;    /* Beep flag.                                         */
                /* =0 ... not to beep.                                */
                /* =1 ... to beep.                                    */
                /* Kanji Monitor sets this to request DBCS Editor to  */
                /* or not to beep.                                    */


char   discrd;  /* Discard flag.                                      */
                /* =0 ... When Kanji input field is active, Enter key */
                /*        caused event and New line/Tab/Back tab key  */
                /*        moves the cursor to next/previous field.    */
                /*        When Kanji input field is not active,       */
                /*        DBCS Editor can use the input code as normal*/
                /*        code.                                       */
                /* =1 ... When Kanji input field is active,  Enter key*/
                /*        does not cause event, New line/Tab/BAck tab */
                /*        key does not move the cursor to             */
                /*        next/previous field.                        */
                /*        When Kanji field is not active, DBCS Editor */
                /*        discards the input code. (Ususally this is  */
                /*        1 when DBCS unique funtion code is input.)  */
                /* Enter Key ........ Key position 64 and 108 ?       */
                /* New Line key ..... Key position 43 ?               */
                /* Tab key .......... Key position 16 ?               */
                /* Back Tab Key ..... Key position 16 (shift) ?       */
                /* Kanji Monitor sets this                            */

char   trace;   /* Trace flag.                                        */
                /* =0 ... Limited trace data to be output to trace    */
                /*        area  in system memory.                     */
                /* =1 ... All trace data to be output to trace file   */
                /*        in disk.                                    */

char   conv;    /* Conversion Flag.                                   */
		/* =0 ... Input Kana character cannot be conveted     */
		/*        Input Kanji.                                */
		/* =1 ... Input Kana character can be converted       */
		/*        into Kanji.                                 */
		/* If this flag is changed from 1 to 0 when a         */
		/* conversion is going on, ther conversion is finished*/
		/* DBCS Editor sets this.                             */

long   rsv1;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv2;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv3;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv4;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv5;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv6;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv7;    /* **** RESERVED FOR FUTURE USE                       */
long   rsv8;    /* **** RESERVED FOR FUTURE USE                       */
};
