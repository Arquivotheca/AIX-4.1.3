#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)27  1.1  src/bos/usr/ccs/lib/libcurses/idcok.c, libcurses, bos411, 9428A410j 9/3/93 15:08:55";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: idcok
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

/* #ident	"@(#)curses:screen/idcok.c	1.1"		*/



#include	"curses_inc.h"

void
idcok(win, bf)
WINDOW	*win;
int	bf;
{
    win->_use_idc = (bf) ? TRUE : FALSE;
}
