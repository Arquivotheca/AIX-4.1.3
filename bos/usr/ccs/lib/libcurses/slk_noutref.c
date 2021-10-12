#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)84  1.1  src/bos/usr/ccs/lib/libcurses/slk_noutref.c, libcurses, bos411, 9428A410j 9/3/93 15:12:49";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: slk_noutrefresh
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

/* #ident	"@(#)curses:screen/slk_noutref.c	1.2"	*/



#include	"curses_inc.h"

/* Wnoutrefresh for the softkey window. */

slk_noutrefresh()
{
    if (SP->slk == NULL)
	return (ERR);

    if (SP->slk->_win && _slk_update())
	(void) wnoutrefresh(SP->slk->_win);

    return (OK);
}
