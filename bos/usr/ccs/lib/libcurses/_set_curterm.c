#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)77  1.1  src/bos/usr/ccs/lib/libcurses/_set_curterm.c, libcurses, bos411, 9428A410j 9/3/93 15:05:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: set_curterm
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

/* #ident	"@(#)curses:screen/_set_curterm.c	1.1"	*/



#define	NOMACROS

#include	"curses_inc.h"

TERMINAL *
set_curterm(newterminal)
TERMINAL *newterminal;
{
    return (setcurterm(newterminal));
}
