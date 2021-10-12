#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)34  1.6  src/bos/usr/ccs/lib/libcurses/keypad.c, libcurses, bos411, 9428A410j 9/3/93 14:45:07";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: keypad
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/keypad.c	1.6"		*/



#include	"curses_inc.h"

/* TRUE => special keys should be passed as a single value by getch. */

keypad(win, bf)
WINDOW	*win;
int	bf;
{
    extern	int	_outch();
    /*
     * Since _use_keypad is a bit and not a bool, we must check
     * bf, in case someone uses an odd number instead of 1 for TRUE
     */

    win->_use_keypad = (bf) ? TRUE : FALSE;
    if (bf && (!SP->kp_state))
    {
	tputs(keypad_xmit, 1, _outch);
	(void) fflush(SP->term_file);
	SP->kp_state = TRUE;
	return (setkeymap());
    }
    return (OK);
}
