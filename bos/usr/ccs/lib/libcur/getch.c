static char sccsid[] = "@(#)55	1.9  src/bos/usr/ccs/lib/libcur/getch.c, libcur, bos411, 9428A410j 9/5/91 16:32:49";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wgetch
 *
 * ORIGINS: 3, 10, 27
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

#define 	_KEY_NOKEY	65534
#include	<cur02.h>
#include        "cur99.h"

extern	char	_extended;

/*
 * NAME:                wgetch
 *
 * FUNCTION: This routine reads in a character and
 *      will add it to the specified window if echo is on, otherwise it
 *      simply returns the character (in integer form).
 *
 * EXTERNAL REFERENCES: crmode(), nocrmode(), echo(), noecho()
 *                      waddch(), wrefresh()
 *
 * DATA STRUCTURES =    WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> <the character>       error -> ERR
 */

wgetch(win)
register	WINDOW	*win;
{

register	int	inp;		/* keych returns integer	*/
register char   was_not_set = FALSE;	/* reflect old raw state        */

static	int		mbindex = 0;	/* length of multi byte char	*/
static	unsigned char	mbchar[8];	/* store multi byte char, if	*/
					/* _extended OFF		*/
	wchar_t		wc;		/* convert mbchar[] -> wc	*/
static	int		esc_flag = FALSE;	/* ESC sequence switch	*/

 /* check scrolling boundry conditions first... */
if (win->_curx >= win->_maxx - 1 && win->_cury >= win->_maxy - 1
    && _echoit && !win->_scroll && (win->_flags & _SCROLLWIN))
    return ERR;

if (!_rawmode) {		/* if not in raw mode           */
    was_not_set = TRUE;		/* remember that state          */
    crmode();			/* turn off canonical processing */
}

inp = keych();			/* get the character            */

#define	K_ESC		0x1b
#define	K_ESCBR		'['
#define	K_ESCL2		0x40
#define	K_ESCU2		0x7f
#define	ESC_END		0x01
#define	ESC_CONT	0x10

if (inp == K_ESC) {
	esc_flag = K_ESC;
} else if (esc_flag) {
	if (esc_flag == K_ESC) {
		if (inp == K_ESCBR) {
			esc_flag = ESC_CONT;
		} else {
			esc_flag = ESC_END;
		}
	} else if ((inp >= K_ESCL2) && (inp <= K_ESCU2)) {
		esc_flag = ESC_END;
	}
}

if (esc_flag) {
	wc = (wchar_t)'\0';
} else if (_extended == FALSE) {	/* case of _extended OFF */
	if (inp > 0xff) {
		wc = (wchar_t)'\0';
		mbindex = 0;
	} else {
		mbchar[mbindex++] = inp;
		if (mbtowc(&wc, mbchar, mbindex) < 0) {
			if (mbindex > MB_CUR_MAX) {
				mbindex = 0;
			}
			wc = (wchar_t)'\0';
		} else {
			mbindex = 0;
		}
	}
} else {
	wc = inp;
}

if (_echoit) {
  if (wc != (wchar_t)'\0'  && wc != _KEY_NOKEY) {
	waddch (win, wc);
	wrefresh (win);
    }
}

if (was_not_set) {		/* if we turned off canonical   */
    nocrmode();			/* set it back on               */
}

if (esc_flag == ESC_END) {
	esc_flag = FALSE;
}

return inp;
}
