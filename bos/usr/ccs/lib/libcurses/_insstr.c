#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)19  1.1  src/bos/usr/ccs/lib/libcurses/_insstr.c, libcurses, bos411, 9428A410j 9/3/93 15:02:46";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: insstr
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

/* #ident	"@(#)curses:screen/_insstr.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

insstr(s)
char	*s;
{
    return (winsstr(stdscr, s));
}
