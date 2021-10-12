#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)29  1.1  src/bos/usr/ccs/lib/libcurses/immedok.c, libcurses, bos411, 9428A410j 9/3/93 15:09:04";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: immedok
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

/* #ident	"@(#)curses:screen/immedok.c	1.3"		*/



#include	"curses_inc.h"

extern	int	_quick_echo();

void
immedok(win, bf)
WINDOW	*win;
int	bf;
{
    if (bf)
    {
	win->_immed = TRUE;
	_quick_ptr = _quick_echo;
    }
    else
	win->_immed = FALSE;
}
