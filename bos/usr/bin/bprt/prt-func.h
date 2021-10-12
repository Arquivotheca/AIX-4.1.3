/* @(#)29	1.2  src/bos/usr/bin/bprt/prt-func.h, libbidi, bos411, 9428A410j 11/4/93 15:33:19 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: TRACE
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************HEADER FILE HEADER*******************************/
/*                                                                            */
/*  Header File Name: PRT-FUNC.H                                              */
/*                                                                            */
/*  FUNCTION: PRT-FUNC.c contains all functions a particular ASCII Control    */
/*            Code or Escape Sequence requires for the right-to-left          */
/*            printing.                                                       */
/*                                                                            */
/******************************************************************************/

#define WIDTH_TAB_OFF           0x00+0x40
#define WIDTH_TAB_ON            0x80+0x40
#define SPECIAL_TAB             0x20
#define TURN_ON_ATTR            0x80
#define TURN_OFF_ATTR           0x00
#define EXP_ATTR_LENGTH         32
#define DWDH                    1  /* double width double hieght   */	
#define SFG                     2  /* select font global           */
#define SCP                     3  /* select cursor position       */
#define RSCP                    4  /* save restore cursor position */
#define SVHM                    5  /* set vertical and orizontal margins */
#define PTU                     6  /* passthrough ..does not affect bidi */



/* defenition of Arabic font fgid that fully describe the corresponding */
/* pitch for the 4019 laeser printer                                    */

#define BASKERVIL_NASSEEM_2             752     /* typo  */
#define BASKERVIL_NASSEEM_BOLD_18       754     /* typo  */
#define BASKERVIL_NASSEEM_BOLD_24       755     /* typo  */
#define BASKERVIL_NASSEEM_IT_2          756     /* typo  */
#define BASKERVIL_NASSEEM_BOLD_IT_12    757     /* typo  */
#define BASKERVIL_NASSEEM_BOLD_IT_18    758     /* typo  */
#define BASKERVIL_NASSEEM_BOLD_IT_24    759     /* typo  */

#define COURIER_NASSEEM_BOLD_79         266     /* 7.9 cpi */
#define COURIER_NASSEEM_BOLD_79_IT      267     /* 7.9 cpi */

#define COURIER_10                      11      /* 10 cpi */
#define COURIER_10_BOLD                 46      /* 10 cpi */
#define COURIER_NASSEEM_10              61      /* 10 cpi */
#define COURIER_NASSEEM_IT_10           62      /* 10 cpi */
#define COURIER_NASSEEM_BOLD_10         63      /* 10 cpi */
#define COURIER_NASSEEM_BOLD_IT_10      64      /* 10 cpi */

#define COURIER_12                      85      /* 12 cpi */
#define COURIER_NASSEEM_12              103     /* 12 cpi */
#define COURIER_NASSEEM_IT_12           104     /* 12 cpi */

#define COURIER_NASSEEM_15              213     /* 15 cpi */

#define COURIER_17                      254     /* 17.1 cpi */
#define COURIER_NASSEEM_17              279     /* 17.1 cpi */

#define GOTHIC_NASSEEM_20               283     /* 20 cpi */

#define BOLDFACE_PS                     159     /* psm */

#define BASKERVIL_NASSEEM               8759    /* typo 12 */
#define BASKERVIL_NASSEEM_IT            8887    /* typo it 12 */


/* defenition of Arabic font fgid that do NOT fully describe the           */
/* corresponding pitch for the 4019 laeser printer                         */
#define BASKERVIL_NASSEEM_BOLD_TYPO_PSM 8779    /* typo : 12, 18, 24 & psm */
#define BASKERVIL_NASSEEM_BOLD_IT       8907    /* typo : 12, 18, 24 */
#define BASKERVIL_BOLDFACE_NASSEEM_BOLD  753    /* psm and typo : 12 */



/* defenition of Hebrew font fgid that fully describe the corresponding */
/* pitch for the 4019 laeser printer                                    */
#define COURIER_SHALOM_10      		49      /* 10 cpi */
#define COURIER_SHALOM_BOLD_10      	50	/* 10 cpi */

#define COURIER_SHALOM_BOLD_12      	98	/* 12 cpi */
#define COURIER_SHALOM_BOLD_15      	226	/* 15 cpi */

#define LETTER_GOTHIC_AVIV_20		282	/* 20 cpi */

#define BOLDFACE_BARAK			167	/* psm */

#define TIMES_ROMAN_NARK		12855	/* typo */
#define TIMES_ROMAN_NARK_BOLD		12875	/* typo */


/* fgids of printers other than the 4019 */
#define PRESTIGE_ELITE                  12


/* 4201 font ids used by esc I */
#define I_4201_0        0       /* 10 cpi (standard) */
#define I_4201_1        1       /* 12 cpi */
#define I_4201_2        2       /* 10 cpi ? (NLQ) */
#define I_4201_3        3       /* 10 cpi ? (NLQ II)*/
#define I_4201_4        4       /* 10 cpi ? (DP download) */
#define I_4201_5        5       /* 12 cpi (download) */
#define I_4201_6        6       /* 10 cpi ? (NLQ download) */
#define I_4201_7        7       /* 10 cpi ? (NLQ II download) */
#define I_4201_11       11      /* 10 cpi ? (Italic NLQ II) */
#define I_4201_15       15      /* 10 cpi ? (NLQ II download) */



/* 4207-8 font ids used by esc I */
#define I_4207_0        0       /* 10 cpi */
#define I_4207_8        8       /* 12 cpi */
#define I_4207_16       16      /* 17 cpi */
#define I_4207_2        2       /* 10 cpi */
#define I_4207_10       10      /* 12 cpi */
#define I_4207_18       18      /* 17 cpi */
#define I_4207_3        3       /* psm */
#define I_4207_4        4       /* 10 cpi */
#define I_4207_12       12      /* 12 cpi */
#define I_4207_20       20      /* 17 cpi */
#define I_4207_6        6       /* 10 cpi */
#define I_4207_14       14      /* 12 cpi */
#define I_4207_22       22      /* 17 cpi */
#define I_4207_7        7       /* psm */


/* 5202 font ids used by esc I */
#define I_5202_32       32      /* 10 cpi */
#define I_5202_33       33      /* 12 cpi */
#define I_5202_34       34      /* 17 cpi */
#define I_5202_35       35      /* 10 cpi */

/* 5204 font ids used by esc I */
#define I_5204_32       32      /* 10 cpi */
#define I_5204_33       33      /* 12 cpi */
#define I_5204_34       34      /* 17 cpi */
#define I_5204_35       35      /* 10 cpi */
#define I_5204_64       64      /* 10 cpi */
#define I_5204_65       65      /* 10 cpi */
#define I_5204_66       66      /* 10 cpi */
#define I_5204_67       67      /* 10 cpi */
#define I_5204_36       36      /* 10 cpi */
#define I_5204_37       37      /* 12 cpi */
#define I_5204_38       38      /* 17 cpi */
#define I_5204_39       39      /* psm */


/* 4019 font ids used by esc I */
/* resident fonts */
#define I_4019_32       32      /* 10 cpi */
#define I_4019_33       33      /* 12 cpi */
#define I_4019_34       34      /* 17.1 cpi */
#define I_4019_35       35      /* psm */
#define I_4019_36       36      /* 10 cpi */
#define I_4019_37       37      /* 12 cpi */
#define I_4019_38       38      /* 17.1 cpi */
#define I_4019_39       39      /* psm */
#define I_4019_40       40      /* 10 cpi */
#define I_4019_41       41      /* 10 cpi */

/* TASHKEEL */
extern char TASHKEEL;
extern char UPPER_ESC[3];
extern char CONSONANT_ESC[3];
extern char LOWER_ESC[3];
extern char bidi_out_buff [4*BUFSIZ];   
extern unsigned short bidi_out_buff_len; 
/* TASHKEEL */

extern char 		TempStr[200];

extern LayoutObject    plh;             /* maya 12/5/1993 */
extern int             CodeSet;
extern EntryPoint      G;
extern unsigned short  counter;
extern int     	       eol_found;       /* to detect the end of line */
extern unsigned long   CURR_CHAR_ATTR;
extern unsigned long   character;
extern unsigned long   Char_Attr;
extern unsigned long   PRT_PSM_ATTR;

extern int  NextChar  (unsigned char *c );
extern int  get_char ();   /* to get a character from a file */
extern void WriteESC  (void);

#define MAX_FUNCTIONS 47
extern unsigned char  (*Functions[MAX_FUNCTIONS]) (void);

extern unsigned char   _PASS_THROUGH            (void);
extern unsigned char   _PRINT_SERVICE_1         (void);
extern unsigned char   _DESTRUCT_BS             (void);
extern unsigned char   _SPECIAL_ONE             (void);
extern unsigned char   _HT_FOUND                (void);
extern unsigned char   _FLUSH_BUFFER            (void);
extern unsigned char   _PRINT_BUFF              (void);
extern unsigned char   _SO_SI                   (void);
extern unsigned char   _PRT_SELECT              (void);
extern unsigned char   _PRT_DESELECT            (void);
extern unsigned char   _CAN_FOUND               (void);
extern unsigned char   _ESC_FOUND               (void);
extern unsigned char   _ESC_SINGLE              (void);
extern unsigned char   _IGNORE_ESC_n_0          (void);
extern unsigned char   _IGNORE_ESC_n            (void);
extern unsigned char   _ESC_SINGLE_0_1          (void);
extern unsigned char   _ZERO_TERMINATOR         (void);
extern unsigned char   _ESC_C_FOUND             (void);
extern unsigned char   _TAB_SET_FOUND           (void);
extern unsigned char   _RIGHT_MARGIN_FOUND      (void);
extern unsigned char   _ESC_SUB_SUPER           (void);
extern unsigned char   _LEFT_MARGIN_FOUND       (void);
extern unsigned char   _IGNORE_ESC_n_0_0        (void);
extern unsigned char   _REVERSE_LF_n            (void);
extern unsigned char   _GRAPHICS                (void);
extern unsigned char   _GRAPHICS_STAR           (void);
extern unsigned char   _DOWNLOAD_EPSON          (void);
extern unsigned char   _DOWNLOAD_PROPRINT       (void);
extern unsigned char   _RESET_TAB_SETTINGS      (void);
extern unsigned char   _PRINTER_RESET           (void);
extern unsigned char   _CHANGE_LL_OUT           (void);
extern unsigned char   _CHANGE_LL_IN            (void);
extern unsigned char   _SET_HOR_MARGINS         (void);
extern unsigned char   _IGNORE_ESC_n_000        (void);
extern unsigned char   _REVERSE_LF              (void);
extern unsigned char   _PRT_DESELECT_n          (void);
extern unsigned char   _PMP                     (void);
extern unsigned char   _RESERVED_1              (void);
extern unsigned char   _PRINTABLE               (void);
extern unsigned char   _PRT_NEXT                (void);
extern unsigned char   _PRT_ALL                 (void);
extern unsigned char   _RESTORE_JMP_2           (void);
extern unsigned char   _RESERVED_2              (void);
extern unsigned char   _HANDLE_ESC_BRKT         (void);
extern unsigned char   _SPACE_FOR_BAK           (void);
extern unsigned char   _BIDI_SEQ_FOUND          (void);
extern unsigned char   _ESC_I                   (void);


#define _ESCAPE 27L
#define _FLUSH  WRITE();




#define TRACE(fname,format, ch) {FILE * stream; \
                          if (debug) { stream = fopen (fname, "a+"); \
                          fprintf (stream, format, ch);\
                          fclose (stream);}}

unsigned char  _DW_DH  (unsigned char );
unsigned char  _PASSTHROUGH_IN_BRACKET (unsigned char c);
unsigned char  _SELECT_FONT (unsigned char c);
unsigned char  _SELECT_CURSOR_POSITION (unsigned char c);
unsigned char  _SAVE_RESTORE_CURSOR_POS (unsigned char c);
unsigned char  _SET_VERT_HORIZ_MARGIN (unsigned char c);
