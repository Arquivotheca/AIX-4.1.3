/* @(#)31	1.12  src/bos/usr/include/cur02.h, libcurses, bos411, 9428A410j 5/14/91 17:17:42 */
#ifndef _H_CUR02
#define _H_CUR02

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur02.h
 *
 * ORIGINS: 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                cur02.h
 *
 * FUNCTION: This file contains #defines for the codes
 *      returned for defined key sequences.
 */

/* definitions for keypad codes - all greater than 0x100        */
/* except code for no data available which is negative          */

/* Note that keypad codes lie between one-byte and two-byte character
 * codes.  All SJIS two-byte codes are > 0x8000. */
#define IS_PADKEY(c)    ((c)>0xff && (c)<0x8000)

#define KEY_NOKEY       -2      /* no keyboard data, nodelay on */
#define KEY_BAD         -1      /* error return in place of key */
#define KEY_BREAK       0x101   /* break - unreliable           */
#define KEY_DOWN        0x102   /* cursor down                  */
#define KEY_UP          0x103   /* cursor up                    */
#define KEY_LEFT        0x104   /* cursor left                  */
#define KEY_RIGHT       0x105   /* cursor right                 */
#define KEY_HOME        0x106   /* home - top left              */
#define KEY_BACKSPACE   0x107   /* backspace - unreliable       */
#define KEY_DL          0x108   /* delete line                  */
#define KEY_IL          0x109   /* insert line                  */
#define KEY_DC          0x10a   /* delete character             */
#define KEY_IC          0x10b   /* insert character mode start  */
#define KEY_EIC         0x10c   /* exit insert character mode   */
#define KEY_CLEAR       0x10d   /* clear screen                 */
#define KEY_EOS         0x10e   /* clear to end of screen       */
#define KEY_EOL         0x10f   /* clear to end of line         */
#define KEY_SF          0x110   /* scroll forward toward end    */
#define KEY_SR          0x111   /* scroll backward toward start */
#define KEY_NPAGE       0x112   /* next page                    */
#define KEY_PPAGE       0x113   /* previous page                */
#define KEY_STAB        0x114   /* set tab stop                 */
#define KEY_CTAB        0x115   /* clear tab stop               */
#define KEY_CATAB       0x116   /* clear all tab stops          */
#define KEY_ENTER       0x117   /* enter key - unreliable       */
#define KEY_SRESET      0x118   /* soft reset key - unreliable  */
#define KEY_RESET       0x119   /* hard reset key - unreliable  */
#define KEY_PRINT       0x11a   /* print or copy                */
#define KEY_LL          0x11b   /* lower left (last line)       */
#define KEY_A1          0x11c   /* pad upper left               */
#define KEY_A3          0x11d   /* pad upper right              */
#define KEY_B2          0x11e   /* pad center                   */
#define KEY_C1          0x11f   /* pad lower left               */
#define KEY_C3          0x120   /* pad lower right              */
#define KEY_DO          0x121   /* DO key                       */
#define KEY_QUIT        0x122   /* QUIT key                     */
#define KEY_CMD         0x123   /* Command key                  */
#define KEY_PCMD        0x124   /* Previous command key         */
#define KEY_NPN         0x125   /* Next pane key                */
#define KEY_PPN         0x126   /* previous pane key            */
#define KEY_CPN         0x127   /* command pane key             */
#define KEY_END         0x128   /* end key                      */
#define KEY_HLP         0x129   /* help key                     */
#define KEY_SEL         0x12a   /* select key                   */
#define KEY_SCR         0x12b   /* scroll right key             */
#define KEY_SCL         0x12c   /* scroll left key              */
#define KEY_TAB         0x12d   /* tab key                      */
#define KEY_BTAB        0x12e   /* back tab key                 */
#define KEY_NEWL        0x12f   /* new line key                 */
#define KEY_ACT         0x130   /* action key                   */

#define KEY_F0          0x180   /* function key -               */
#define KEY_F(n)        (KEY_F0+(n))/*    reserve 128 values        */

#define KEY_SF1         (KEY_F0+64)/* Special function key 1       */
#define KEY_SF2         (KEY_F0+65)/* Special function key 2       */
#define KEY_SF3         (KEY_F0+66)/* Special function key 3       */
#define KEY_SF4         (KEY_F0+67)/* Special function key 4       */
#define KEY_SF5         (KEY_F0+68)/* Special function key 5       */
#define KEY_SF6         (KEY_F0+69)/* Special function key 6       */
#define KEY_SF7         (KEY_F0+70)/* Special function key 7       */
#define KEY_SF8         (KEY_F0+71)/* Special function key 8       */
#define KEY_SF9         (KEY_F0+72)/* Special function key 9       */
#define KEY_SF10        (KEY_F0+73)/* Special function key 10      */


#define KEY_MAXC        0x200   /* max valid key code           */
#define KEY_MAXNLS      0x400   /* last P2 code point in array  */

/*      define base values for ESC sequence return codes                */

				/* the following value is added */
				/* to the ending character code */
				/* for ESC sequences in the form */
				/* ESC c with c in the range    */
				/* 0x30 - 0x7f, thus the value  */
				/* will be in the range 0x200   */
				/* to 0x24f                     */
#define KEY_ESC1        0x1d0

				/* the following value is added */
				/* to the ending character code */
				/* for ESC sequences in the form */
				/* ESC [ s c with c in the range */
				/* 0x40 - 0x7f, thus the value  */
				/* will be in the range 0x250   */
				/* to 0x28f                     */
#define KEY_ESC2        0x210


/************************************************************************/
/*                                                                      */
/*      The following define is for the value returned when the         */
/*      select button on the locator is detected                        */
/*                                                                      */
/************************************************************************/

#define KEY_SLL         0x300	/* locator select request       */

/************************************************************************/
/*                                                                      */
/*      The following specific codes are the codes returned which       */
/*      are followed by binary data and the length of that binary data  */
/*                                                                      */
/************************************************************************/

#define KEY_LOCESC      KEY_ESC2+'y'/* locator return value         */
#define KEY_LOCl        10	/* locator data size            */

#define KEY_NXKESC      KEY_ESC2+'w'/* non-translate keyboard data  */
#define KEY_NXKl        3	/* non-tran key data size       */

#define KEY_VTDESC      KEY_ESC2+'x'/* general VTD data stream      */
#define KEY_VTDl        4	/* VTD length field size        */

#define P1LO_SHIFT      0x1f
#define P1HI_SHIFT      0x1e
#define P2LO_SHIFT      0x1d
#define P2HI_SHIFT      0x1c

#define P1LO_PACK       0x1d80
#define P1HI_PACK       0x1c00
#define P2LO_PACK       0x1a80
#define P2HI_PACK       0x1900

/***********************************************************************/
/* These macros are for setting up and using the character validity    */
/* mask array passed to ecflin():                                      */
/*      SETBIT(a,k)                                                    */
/*      CLRBIT(a,k)                                                    */
/*      TSTBIT(a,k),    where a is the validity mask array and         */
/*                      k is the value of the character whose          */
/*                      corresponding bit is to be accessed.           */
/* For P0 characters and key codes, use the value directly. For P1 and */
/* P2 characters, the appropriate "packing" factor must be subtracted  */
/* from the character value before using these macros.  For example,   */
/* if the character's value is 0x1fb1 (which is on low P1 code page),  */
/* the macro should be used like this :                                */
/*      SETBIT(array, ch - P1LO_PACK) ;                                */
/***********************************************************************/

#define SETBIT(a,k)                             /* set key map bit   */\
								       \
       ( a[ (k) >> 3 ] |= ( 0x80 >> ( (k) & 0x07 ) ) )

#define CLRBIT(a,k)                             /* clear key map bit */\
								       \
       ( a[ (k) >> 3 ] &= ~( 0x80 >> ( (k) & 0x07 ) ) )

#define TSTBIT(a,k)                             /* test key map bit  */\
								       \
       ( a[ (k) >> 3 ] & ( 0x80 >> ( (k) & 0x07 ) ) )
# endif                        /* _H_CUR02 */
