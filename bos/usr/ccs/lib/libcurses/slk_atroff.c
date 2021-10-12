#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)79  1.1  src/bos/usr/ccs/lib/libcurses/slk_atroff.c, libcurses, bos411, 9428A410j 9/3/93 15:12:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: slk_attroff
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

/* #ident	"@(#)curses:slk_atroff.c	1.1"		*/



#include "curses_inc.h"

slk_attroff (a)
chtype a;
{
    WINDOW *win;

    /* currently we change slk attribute only when using software */
    /* slk's.  However, we may introduce a new terminfo variable  */
    /* which would allow manipulating the hardware slk's as well  */

    if ((SP->slk == NULL) || ((win = SP->slk->_win) == NULL))
	return (ERR);

    return (wattroff (win, a));
}
    
