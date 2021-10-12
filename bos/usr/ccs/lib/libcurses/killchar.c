#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)45  1.6  src/bos/usr/ccs/lib/libcurses/killchar.c, libcurses, bos411, 9428A410j 9/3/93 14:45:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: killchar
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

/* #ident	"@(#)curses:screen/killchar.c	1.3"		*/



/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses_inc.h"

char
killchar()
{
#ifdef SYSV
    return (SHELLTTY.c_cc[VKILL]);
#else
    return (SHELLTTY.sg_kill);
#endif
}
