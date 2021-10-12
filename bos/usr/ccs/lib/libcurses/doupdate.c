#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)09  1.7  src/bos/usr/ccs/lib/libcurses/doupdate.c, libcurses, bos411, 9428A410j 9/3/93 14:44:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: doupdate
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

/* #ident	"@(#)curses:screen/doupdate.c	1.7"		*/



#include	"curses_inc.h"

/*
 * Doupdate is a real function because _virtscr
 * is not accessible to application programs.
 */

doupdate()
{
    return (wrefresh(_virtscr));
}
