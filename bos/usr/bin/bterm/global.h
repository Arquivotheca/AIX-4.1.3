/* @(#)81	1.1  src/bos/usr/bin/bterm/global.h, libbidi, bos411, 9428A410j 8/26/93 13:34:55 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: is_column_head
 *		reset_column_head
 *		set_column_head
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
/*	Documentation: global.h
**		These are the global definitions.
**
*/
#ifndef _GLOBAL_
#define _GLOBAL_

#include <sys/lc_layout.h>
#include "BDSeg.h"
/* terminals supported */
#define IBM3151 1
#define VT220   2
#define HFT     3
#define AIXTERM 4

#define pty_R_size  1024  /* pty size */
#define Max_Lines     50  /* maximum number of lines supported 0..49*/
#define Max_Columns  200  /* maximum number of columns supported 0..199*/
#define to_hft_SIZE  510  /* Size of real hft write buffer RS/6000 */

/* line structure */
struct line {
   char _line_chars[Max_Columns]; /* buffer of characters */
   int  _line_atrib[Max_Columns]; /* buffer of attributes */
   char _line_grph[Max_Columns];  /* buffer of alternate graphics */
};
typedef struct line line_type;
/* array to contain tab stops */
typedef char v_tab_type[Max_Lines];
typedef char h_tab_type[Max_Columns];
typedef char push_type[Max_Columns];
/* structure containing screen images and cursor positions */
struct screen {
   line_type _scr_image[Max_Lines];         /* buffer of characters */
   line_type _phys_image[Max_Lines];        /* physical screen image */
   int          _changed[Max_Lines];        /* MDT for lines */
   int          _cur_atrib;                 /* current character attribute */
   char         _cur_grph;                  /* current graphics mode */
   short	_logx;                      /* logical text x cursor */
   short	_logy;                      /* logical text y cursor */
   short	_visx;                      /* visual text x cursor */
};
typedef struct screen screen_type;

/*... file descriptors ...  */
extern int termfildes;        /* our true /dev/tty file descriptor  */
extern int ptyfildes;         /* pty controller file descriptor     */
extern int ttypfildes;        /* pty server     file descriptor     */
extern int ttypPfildes;       /* pty server for ehterm process      */
extern int ptystatus;         /* ptystatus from ioctl call          */

extern char ttydev[30];   /* name of pty server device      */
extern char ptydev[30];   /* name of pty controller device  */

/* flags for bidi locale activity */
extern int ActiveShaping;
extern int ActiveBidi;

/* operating modes */
extern int SCROLL;  /* scroll after writing char in last column on last line */
extern int WRAP;  /* wrap to next line after writing char in last column */
extern int INSERT; /* insert new characters , do not overwrite */
extern int AUTOLF; /* carriage return does not move to next line */
extern int COLUMN_HEAD; /* column heading mode for implicit */
extern int TERM;          /* current active terminal type */

#define is_column_head()     COLUMN_HEAD
#define set_column_head()    COLUMN_HEAD=TRUE
#define reset_column_head()  COLUMN_HEAD=FALSE

/* some terminal specific escape sequences */
extern char *EraseInput;
extern char *Scroll;
extern char *NoScroll;
extern char *Jump;
extern char *TERM_INIT;
extern int  ATRIB_NORMAL;

extern void (*set_atrib)();
extern void (*set_grph)();
extern void (*do_reset_function_keys)();
extern int (*yylex_function)();

/* structure containing screen image and bidi data */
extern screen_type *SCR;
extern int  active_atrib;
extern char active_grph;
extern v_tab_type V_tabs;      /* vertical tab stops */
extern h_tab_type H_tabs;      /* horizontal tab stops */

/* bidi function addresses */
extern LayoutObject plh;

/*... all other stuff ...    */
extern int COLUMNS;                   /* COLUMNS from environment           */
extern int LINES;                     /* LINES from environment             */
extern int X_Status;                  /* x position of status info          */
extern int Y_Status;                  /* y position of status info          */
			              /* if zero, status not displayed      */
extern int NO_INVERSE[Max_Lines];     /* lines not to be inverted           */
extern struct termios orgtermios;     /* original tty setting in here       */
extern struct winsize W_Size;         /* current window size                */

extern int jdx;
extern char *new_jdx;                 /* buffer for for pty input */

extern int status_line;  /* indicates whether status line is displayed or not */
extern unsigned char function_keys[36][64]; /*function keys definition buffer */

extern push_type push_buffer;       /* buffer for push mode                  */
extern int push_boundary;           /* on push boundary                      */

/* for yylex input stream */
extern unsigned char buffer[1030];
extern int buffer_index;
extern int buffer_length;
extern int Lex_Mode;                  /* 1=screen input   2=kbd input         */
extern int STATE;                     /* 0=not in  middle of sequence parsing
                                         1=in middle of sequence parsing      */

#endif _GLOBAL_
