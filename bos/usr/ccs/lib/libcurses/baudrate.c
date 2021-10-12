#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)21  1.7  src/bos/usr/ccs/lib/libcurses/baudrate.c, libcurses, bos411, 9428A410j 9/3/93 14:43:23";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: baudrate
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

/* #ident	"@(#)curses:screen/baudrate.c	1.4"		*/


/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses_inc.h"

int
baudrate()
{
    return (SP->baud);
}
