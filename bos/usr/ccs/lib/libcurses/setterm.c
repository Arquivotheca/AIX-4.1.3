#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)62  1.6  src/bos/usr/ccs/lib/libcurses/setterm.c, libcurses, bos411, 9428A410j 9/3/93 14:47:56";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: setterm
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

/* #ident	"@(#)curses:screen/setterm.c	1.3"		*/



#include	"curses_inc.h"
#undef	setterm
setterm(name)
char	*name;
{
    return (setupterm(name, 1, (int *) NULL));
}
