#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)26  1.1  src/bos/usr/ccs/lib/libcurses/has_colors.c, libcurses, bos411, 9428A410j 9/3/93 15:08:50";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: has_colors
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

/* #ident	"@(#)curses:has_colors.c	1.1"		*/



#include "curses_inc.h"

bool has_colors()
{
    return ((max_pairs == -1) ? FALSE : TRUE);
}
