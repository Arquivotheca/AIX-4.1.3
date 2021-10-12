#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)86  1.1  src/bos/usr/ccs/lib/libcurses/slk_restore.c, libcurses, bos411, 9428A410j 9/3/93 15:12:57";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: slk_restore
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

/* #ident	"@(#)curses:screen/slk_restore.c	1.4"	*/



#include	"curses_inc.h"

/* Restore screen labels. */

slk_restore()
{
    if (SP->slk)
    {
	SP->slk->_changed = TRUE;
	(void) slk_touch();
	(void) slk_refresh();
    }
    return (OK);
}
