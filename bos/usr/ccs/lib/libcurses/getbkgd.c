#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)20  1.1  src/bos/usr/ccs/lib/libcurses/getbkgd.c, libcurses, bos411, 9428A410j 9/3/93 15:08:26";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: getbkgd
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

/* #ident	"@(#)curses:screen/getbkgd.c	1.2"		*/



#include	"curses_inc.h"

chtype
getbkgd(win)
WINDOW	*win;
{
    return (win->_bkgd);
}
