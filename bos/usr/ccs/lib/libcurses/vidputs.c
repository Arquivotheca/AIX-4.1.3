#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)06  1.8  src/bos/usr/ccs/lib/libcurses/vidputs.c, libcurses, bos411, 9428A410j 9/3/93 14:49:35";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: vidputs
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

/* #ident	"@(#)curses:screen/vidputs.c	1.9"		*/



#include	"curses_inc.h"

vidputs(a,b)
chtype	a;
int	(*b)();
{
    vidupdate(a,cur_term->sgr_mode,b);
    return (OK);
}
