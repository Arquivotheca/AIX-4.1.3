#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)91  1.1  src/bos/usr/ccs/lib/libcurses/_border.c, libcurses, bos411, 9428A410j 9/3/93 15:01:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		border
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

/* #ident	"@(#)curses:screen/_border.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

border(ls, rs, ts, bs, tl, tr, bl, br)
chtype	ls, rs, ts, bs, tl, tr, bl, br;
{
    return (wborder(stdscr, ls, rs, ts, bs, tl, tr, bl, br));
}
