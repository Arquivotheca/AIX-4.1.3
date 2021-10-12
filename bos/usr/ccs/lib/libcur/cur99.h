/* @(#)12	1.11  src/bos/usr/ccs/lib/libcur/cur99.h, libcur, bos411, 9428A410j 5/14/91 17:00:49 */
#ifndef _H_CUR99
#define _H_CUR99

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur99.h
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                cur99.h
 *
 * FUNCTION: This file contains the global variable
 *      declarations used by the other curses routines.
 */
#include 	<stdlib.h>
#include        "cur00.h"
#include        "cur04.h"

/* For internal representation, the bytes representing the first and second
 * bytes of a SJIS character will have the high byte set to a value which
 * cannot be confused with the high byte of a SJIS character or an NLS
 * character. */
#define FIRST_BYTE              0xfe00
#define SECOND_BYTE             0xff00
#define BYTE_MASK               0xff00
#define is_first_byte(wc)       ( wcwidth((wc))==2?1:\
				  (((wc)&BYTE_MASK) == FIRST_BYTE) )
#define is_second_byte(wc)       ( wcwidth((wc))==WEOF?1:\
				  (((wc)&BYTE_MASK) == SECOND_BYTE) )

#define PART_CHAR '@'           /* the partial character indicator */
/* During field input, characters may be temporarily split across lines.
 * They will become partial characters if they remain after field input
 * is done, but until then the data must be maintained.  These flags
 * occupy the BYTE_MASK portion. */
#define FIRST_PART              0xfc00
#define SECOND_PART             0xfd00
#define is_first_part(c)        (((c)&BYTE_MASK) == FIRST_PART)
#define is_second_part(c)       (((c)&BYTE_MASK) == SECOND_PART)
#define is_part(c)              (((c)&0xfe00) == 0xfc00)

/* the all important structure for the attribute information */
struct attr_mask {		/* structure to contain our masks */
    short   start_bit;		/* bit at which this mask starts */
    short   type_attr;		/* 1-switch, 2-f_color, 3-b_color, 4-font 
				*/
    short   num_val;		/* number of elements in this mask */
    ATTR    act_attr;           /* actual pattern for ANDing this mask */
    char   *set[8];		/* strings to set each element */
    char   *reset;		/* string to unset this set or individual 
				*/
};

extern  ATTR sw_mask;		/* mask for all switch type attributes    
				*/

extern char _echoit,
            _rawmode,
            My_term,
	    _endwin,
	    _noesckey;

extern char _keypad,
	    _nodelay,
	    _trackloc;

extern char *_unctrl[];

extern int  _tty_ch,
            LINES,
            COLS;

extern  SGTTY _tty,
        _res_flg;

char    eciopc ();

extern int  CE_cost,
            EC_cost;		/* cost analysis variables */

#define HIDE_MASK       0x01

#endif				/* _H_CUR99 */
