/* @(#)86	1.1  src/bos/usr/bin/bterm/main.h, libbidi, bos411, 9428A410j 8/26/93 13:35:14 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
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
/*	Documentation: main.h
**		These are the main definitions.
**
*/

#ifndef _MAIN_
#define _MAIN_

/*... file descriptors ...  */

int termfildes;        /* our true /dev/tty file descriptor  */
int ptyfildes;         /* pty controller file descriptor     */
int ttypfildes;        /* pty server     file descriptor     */
int ttypPfildes;       /* pty server for ehterm process      */
int ptystatus;         /* ptystatus from ioctl call          */

char ttydev[30] = "/dev/ttyxx";   /* name of pty server device      */
char ptydev[30] = "/dev/ptyxx";   /* name of pty controller device  */


/* operating modes */
int SCROLL = TRUE;  /* scroll after writing char in last column on last line */
int WRAP   = TRUE;  /* wrap to next line after writing char in last column */
int INSERT = FALSE; /* insert new characters, do not overwrite */
int AUTOLF = FALSE; /* carriage return does not move to next line */
int COLUMN_HEAD = FALSE;  /* columns heading mode for implicit */
int TERM;          /* current active terminal type */

/* some terminal specific escape sequences */
char *EraseInput;
char *Scroll;
char *NoScroll;
char *Jump;
char *TERM_INIT;
int  ATRIB_NORMAL;

void (*set_atrib)();
void (*set_grph)();
void (*do_reset_function_keys)();
int (*yylex_function)();

/* structure containing screen image and bidi data */
screen_type *SCR;
int  active_atrib;
char active_grph;
v_tab_type V_tabs;      /* vertical tab stops */
h_tab_type H_tabs;      /* horizontal tab stops */

/* flags for bidi locale activity */
int ActiveShaping;
int ActiveBidi;

/* bidi function addresses */
LayoutObject plh;

/*... all other stuff ...    */

int COLUMNS         = 80;      /* COLUMNS from environment           */
int LINES           = 24;      /* LINES from environment             */
int X_Status        = 0;       /* x position of status info          */
int Y_Status        = 0;       /* y position of status info          */
			       /* if zero, status not displayed      */
int NO_INVERSE[Max_Lines];     /* lines not to be inverted           */
struct termios orgtermios;     /* original tty setting in here       */
struct winsize W_Size;         /* current window size                */

int jdx;
char *new_jdx;                 /* buffer for for pty input */

unsigned char function_keys[36][64];   /* function key definition buffer */
int status_line=FALSE;  /* indicates that status line is displayed */

push_type push_buffer;    /* buffer for push mode */
int push_boundary;       /* on push boundary */

/* for yylex input stream */
unsigned char buffer[1030];
int buffer_index;
int buffer_length;
int Lex_Mode;   /* 1=screen input parsing, 2=kbd input parsing */
int STATE;      /* parsing state of input parsing. 0=not in middle
                   of parsing, 1=got new input and has not identified it yet */

/* DEBUG information */
#ifdef DEBUG
FILE *TRACE;
#endif DEBUG

#endif _MAIN_
