/* @(#)88	1.1  src/bos/kernext/lft/inc/lft_swkbd.h, lftdd, bos411, 9428A410j 10/15/93 14:30:58 */
/*
 *   COMPONENT_NAME: LFTDD
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

/* ---------------------------------------------------------------------- *
 * The following structure holds information in the software keyboard     *
 * table to map one keystroke (scan code, position code and status flag)  *
 * to a code page/code point, escape control, control code, or user-      *
 * defined string.  							  *
 * ---------------------------------------------------------------------- */
#define LFT_MAX_POSITIONS	134
#define LFT_MAX_STATES		7

typedef struct {
        uchar   flag;           	/* type of keystroke  		  */
#define FLAG_GRAPHIC            0x0     /* normal printable character     */
#define FLAG_SINGLE_CONTROL     0x1     /* control code (0x00 to 0x20) 	  */
#define FLAG_CHAR_STRING        0x4     /* char string mapped to key 	  */

#define FLAG_ESC_FUNCTION       0x5     /* escape function 		  */
#define FLAG_CNTL_FUNCTION      0x6     /* control function 		  */
#define FLAG_APP_FUNCTION       0x7

        uchar   stat;           	/* state of keystroke 		  */
#define CODE_STAT_NONE          0x0     /* normal key 			  */
#define CODE_STAT_ERROR         0x1     /* error in parsing control sequence */
#define CODE_STAT_DECODED       0x2     /* esc/ctl seq successfully parsed */
#define CODE_STAT_CURSOR        0x3     /* key affects cursor 		  */
#define CODE_STAT_PF_KEY        0x4     /* programmable function key 	  */
#define CODE_STAT_DEAD          0x5     /* diacritic key 		  */
#define CODE_STAT_STATE         0x6     /* used on japanese keyboards 	  */

        uchar   str_page;       	/* position in char string key map */

        uchar   code_page;      	/* code page may be one of 	  */
#define CHARSET_P0              0x3c    /* final character values specifying */
			                /*  character sets in ANSI 	  */
                                        /* '<', page 0 			  */
        uchar   code;           	/* code point 			  */
} lft_keystroke_t;

/* ---------------------------------------------------------------------- */
/* Software keyboard map structure					  */
/* ---------------------------------------------------------------------- */
typedef struct {
        char            disp_set[8];    /* display set this maps 	  */
        char            kbdname[8];     /* descriptive name     	  */
        lft_keystroke_t keystrokes[LFT_MAX_STATES][LFT_MAX_POSITIONS];
        ulong           capslock[5];    /* bit map of keys affected 	  */
                                        /* by caps lock key     	  */
} lft_swkbd_t;



