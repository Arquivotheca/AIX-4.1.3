#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)36  1.6  src/bos/usr/ccs/lib/libcurses/has_ic.c, libcurses, bos411, 9428A410j 9/3/93 14:44:45";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: has_ic
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

/* #ident	"@(#)curses:screen/has_ic.c	1.3"		*/



#include "curses_inc.h"

/* Query: Does it have insert/delete char? */

has_ic()
{
    return ((insert_character || enter_insert_mode || parm_ich) &&
		(delete_character || parm_dch));
}
