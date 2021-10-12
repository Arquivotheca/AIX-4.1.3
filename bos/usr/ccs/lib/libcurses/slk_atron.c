#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)80  1.1  src/bos/usr/ccs/lib/libcurses/slk_atron.c, libcurses, bos411, 9428A410j 9/3/93 15:12:33";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: slk_attron
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

/* #ident	"@(#)curses:screen/slk_atron.c	1.1"		*/



#include "curses_inc.h"

slk_attron (a)
chtype a;
{
    WINDOW *win;

    /* currently we change slk attribute only when using software */
    /* slk's.  However, we may introduce a new terminfo variable  */
    /* which would allow manipulating the hardware slk's as well  */

    if ((SP->slk == NULL) || ((win = SP->slk->_win) == NULL))
	return (ERR);

    return (wattron (win, a));
}
    
