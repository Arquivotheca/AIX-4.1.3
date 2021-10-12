/* @(#)22	1.3  src/bos/usr/lpp/kls/dictutil/huke.h, cmdkr, bos411, 9428A410j 11/30/93 17:04:42 */
/*
 * COMPONENT_NAME:	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS:		huke.h
 *
 * ORIGINS:		27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       huke.h
 *
 *  Description:  header file.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#ifndef _HUKE_H_
#define _HUKE_H_

/* AIX standard header file */

#include <curses.h>
#include <term.h>
#include <sys/termio.h>

/* user definition header file include */

/* Extended Curses Librualy Emulation macro */

/* Undefine following identifier for curses substitution function in curses.c */
#undef initscr
#undef werase
#undef fullbox
#undef addch
#undef getch
/* Undefine following identifier for curses substitution macro */
#undef mvwaddch
#undef addstr
#undef waddstr
#undef mvaddstr
#undef mvwaddstr
#undef refresh
#undef wrefresh
#undef wcolorout
#undef colorout
#undef clear
#undef clrtoeol
#undef wclrtoeol
#undef move
#undef wclear
#undef erase
#undef flash
#undef beep
/* Undefine following identifier for include cur02 */
#undef KEY_BREAK
#undef KEY_DOWN
#undef KEY_UP
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_HOME
#undef KEY_BACKSPACE
#undef KEY_DL
#undef KEY_IL
#undef KEY_DC
#undef KEY_IC
#undef KEY_EIC
#undef KEY_CLEAR
#undef KEY_EOS
#undef KEY_EOL
#undef KEY_SF
#undef KEY_SR
#undef KEY_NPAGE
#undef KEY_PPAGE
#undef KEY_STAB
#undef KEY_CTAB
#undef KEY_CATAB
#undef KEY_ENTER
#undef KEY_SRESET
#undef KEY_RESET
#undef KEY_PRINT
#undef KEY_LL
#undef KEY_A1
#undef KEY_A3
#undef KEY_B2
#undef KEY_C1
#undef KEY_C3
#undef KEY_F0
/* Define Function key code for getch() (getcharacter())*/
#include <cur02.h>

/* Define new macro */
#define initscr()                       init_scr()
#define getch()                         getcharacter(GET_CHAR)
#define waddch(win,c)                   addch(c)
#define mvwaddch(win,row,col,c)         (wmove(win,row,col), waddch(win,c))
#define addstr(str)                     putp(str)
#define waddstr(win,str)                putp(str)
#define mvaddstr(row,col,str)           (move(row,col), addstr(str))
#define mvwaddstr(win,row,col,str)      (wmove(win,row,col), waddstr(win,str))
#define refresh()                       fflush(stdout)
#define wrefresh(win)                   fflush(stdout)
#define wcolorout(win,mode)             colorout(mode)
#define colorout(mode)                  putp(mode)
#define clear()                         putp(clear_screen)
#define clrtoeol()                      putp(clr_eol)
#define wclrtoeol(win)                  putp(clr_eol)
#define move(row, col)                  putp(tparm(cursor_address,row,col))
#define wclear(win)                     werase(win)
#define erase()                         clrear()
#define flash()                         putp(bell_sound)
#define beep()                          putp(bell_sound)
#define REVERSE                         enter_reverse_mode
#define NORMAL                          exit_attribute_mode
#define KEY_MAXC1                       (KEY_MAXC + 2)
#define KEY_INFO_DISP                   (KEY_MAXC1 - 1)
#define BELL_CHAR (0x07)
#define DC3_CHAR  (0x13)
#define DC1_CHAR  (0x11)


/* include getcharacter define code     */
#include "hucode.h"

/* standerd file description */

#define STDIN  (fileno(stdin))
#define STDOUT (fileno(stdout))
#define STDERR (fileno(stdout))

/* Flag status */

#define ON  TRUE
#define OFF FALSE

/* Special constant value for xke */

#define NULLCELL sizeof(*eob)
#define MAX_TEXT 512*1024 /* Max text buffer size (512KB = 524288byte)  */
#define NAMELEN (256)     /* In case of UNIX System V, File name length */
			  /* is ONLY 14. But that is eficient length.   */
			  /* User can specify lenger name.              */
			  /* therefor the xke allocate 256 byte length  */
			  /* character dimension.                       */
#define STD_NAMELEN (14)  /* Standerd File name length in case of UNIX  */
			  /* System V.                                  */
#define ONELINE (255)     /* Maximam character length per line          */
#define ONELINEX (254)    /* Maximam HEX data length per line           */
#define ONELINE_CHR (ONELINE + 1 + 1)
			  /* Editing buffer size (225 char + \n + \0)   */

/* Keyboard Function key mapping manupiration */

#define CTRL_KEY_NUM (26)     /* number of Ctrl + ? key                 */
#define PF_KEY_NUM   (12)     /* number of Function key                 */
#define KMAP_SIZE (CTRL_KEY_NUM + PF_KEY_NUM + 1)
			      /* keyboard map table size                */
#define PASS_NOT  (0)         /* Do not pass this code                  */
#define PASS_NORM (1)         /* Pass through with no change            */
#define PASS_CTRL (2)         /* Pass through with conversion (ctrl + ?)*/
#define PASS_PF   (3)         /* Pass through with conversion (PF??)    */

/* Return code of fullpathname() */

#define INVALID_FN (-1)       /* Invalid filename                       */
#define BAD_DIR    (-2)       /* Bad file name                          */

#define CTRL_BACKSLASH (034)  /* to Set Quit key */
#define MINIMUM_CHR    (1)    /* if there are MINIMUM_CHR byte in buffer, */
			      /*   send them                              */
#define MAXIMUM_CHR    (255)  /* max c_cc[VMIN] value in raw mode         */

#define WAIT_LOOP_1  (20)
#define WAIT_LOOP_2  (30)
#define WAIT_LOOP_3  (40)
#define WAIT_LOOP_4  (60)
#define WAIT_LOOP_5  (120)

#define WAIT_TIME_1  (1)
#define WAIT_TIME_2  (1)
#define WAIT_TIME_3  (1)
#define WAIT_TIME_4  (1)
#define WAIT_TIME_5  (2)

#define WAIT_TIME      (3)    /* if the time-out value WAIT_TIME has      */
			      /*   expiered since the last chaeacter      */
			      /*   received, send data                    */
#define UNKNOWN_CHR    (0)    /* If program received unknown string,      */
			      /*   return code means unknown character    */
/* define error reasion */

#define NOREEOR    (0)      /* No error */
#define MEMORYFULL (1)      /* Memory full (text buffer full) */

/* define edit mode */

#define INSERT    (0)
#define OVERWRITE (1)

/* shft value for editorial mode displaying */

#define OVR70     (0)
#define OVR50     (2)
#define OVR30     (4)
#define OVR20     (4)

/* define text window refresh mode */

#define NOREF     (0)
#define INFOREF   (1)
#define LINEREF   (2)
#define LINEREF2  (3)
#define TOENDREF  (4)
#define SCRDWREF  (5)
#define SCRUPREF  (6)
#define ALLREF    (7)

/* Message supress flag */

#define MSG_SUPRESS (TRUE)
#define MSG_OK      (FALSE)

/* message window condition flag */

#define AUTOERASE    (0)
#define MANUALERASE (-1)
#define ANYKEYERASE (-2)

/* move and copy pointer flag set and reset */

#define UNSETFLG (0)
#define SETFLG   (1)

/* Begine or  End */

/* #define UNSETFLG (0) */
#define BOXBEGIN (1)
#define LINBEGIN (2)
#define BLOCKEND (3)

/* Move Type */

#define BADCRT  (0)
#define INUPCRT (1)
#define INUNCRT (2)
#define OUTCRT  (3)
#define XUPCRT  (4)
#define XUNCRT  (5)
/* add LINE BLOCK DELETE special Type */
#define TPUNCRT (6)
#define BTUNCRT (7)
#define XOUTCRT (8)

/* BOX BLOCK DELETE special Type */

#define TOPINCRT    (0)
#define TOPLEFTCRT  (1)
#define TOPRIGHTCRT (2)
#define TOPOUTCRT   (3)

/* Type for cptptext() */
#define MVCPTEXT  (0)
#define DELTEXT   (1)

/* Character and Graphic Box */

#define G_TOP_LEFT_CORNER     (*(box_chars_1+0))
#define G_HORIZONTAL_LINE     (*(box_chars_1+1))
#define G_TOP_RIGHT_CORNER    (*(box_chars_1+2))
#define G_VERTICAL_LINE       (*(box_chars_1+3))
#define G_BOTTOM_RIGHT_CORNER (*(box_chars_1+4))
#define G_BOTTOM_LEFT_CORNER  (*(box_chars_1+5))

#define C_VERTICAL_LINE       '|'
#define C_HORIZONTAL_LINE     '-'
#define C_CORNER              '+'

/* function type for getcharacter() */
#define SET_UP_TBL   (0)
#define GET_CHAR     (1)
#define CHECK_TABLE  (2)

/* Serch result in getcharacter() */
#define NOSTRING   (0)
#define PARTSTRING (1)
#define FULLSTRING (2)

/* I/O buffer flash type. (ioctrl(fileno, TCFLSH, type)) */
#define FLASHINPUT  (0)
#define FLASHOUTPUT (1)
#define FLASHINOUT  (2)

/* text stract */
struct cell {
       struct   cell *pred;     /* pointer to next line                 */
       struct   cell *suc;      /* pointer to previous line             */
       int           ll;        /* character number of this line        */
       unsigned char text[1];   /* use for storeing character           */
};

/* copy and move pointer struct */
struct mvcp {
       int         stat;     /* flag set or unset. */
       int         bl;       /* box or line.       */
       struct cell *lineptr; /* text pointer.      */
       int         cpx;      /* cursor position.   */
};

/* Screen size */
struct ScreenSize {
	int     row;    /* row   number of screen       */
	int     col;    /* colum number of screen       */
};

/* Function key data */
struct fkkey {
	char *fstring;
	int  fcode;
};

/* Curses Window information */
struct CURWIN {
	int     lins;   /* Curses window lines                          */
	int     cols;   /* Curses window colums                         */
	int     topx;   /* Curses window top position(X cordinate)      */
	int     topy;   /* Curses window top position(Y cordinate)      */
};

/* xke all window information */

struct XKEWIN {
	struct CURWIN text;     /* Texst window description             */
	struct CURWIN info;     /* Information line window description  */
	struct CURWIN mess;     /* Pop-up box window description        */
};

/* cursor position */
struct cursor {
	int cp; /* current cursor colum position                                */
	int cl; /* current cursor line  number                                  */
	int cx; /* X-cordinate(colums) of current cursor position on the screen */
	int cy; /* Y-cordinate(rows)   of current cursor position on the screen */
	int clc;/* current line characters, (because the text of current line   */
		/* data in memory is differ from editing  buffer.               */
};

extern char bell_sound[];

#endif
