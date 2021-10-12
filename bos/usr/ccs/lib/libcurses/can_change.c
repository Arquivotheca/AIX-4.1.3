#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)97  1.1  src/bos/usr/ccs/lib/libcurses/can_change.c, libcurses, bos411, 9428A410j 9/3/93 15:06:46";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: can_change_color
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

/* #ident	"@(#)curses:can_change.c	1.1"		*/



#include "curses_inc.h"

bool can_change_color()
{
     return (can_change);
}
