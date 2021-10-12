#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)35  1.1  src/bos/usr/ccs/lib/libcurses/wbkgdset.c, libcurses, bos411, 9428A410j 9/3/93 15:46:25";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wbkgdset
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

/* #ident	"@(#)curses:screen/wbkgdset.c	1.3"		*/



#include	"curses_inc.h"

void
wbkgdset(win,c)
WINDOW	*win;
chtype	c;
{
    win->_attrs = (win->_attrs & ~(win->_bkgd & A_ATTRIBUTES)) | (c & A_ATTRIBUTES);
    win->_bkgd = c;
}
