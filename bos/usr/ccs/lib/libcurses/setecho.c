#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)74  1.1  src/bos/usr/ccs/lib/libcurses/setecho.c, libcurses, bos411, 9428A410j 9/3/93 15:12:04";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _setecho
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

/* #ident	"@(#)curses:screen/setecho.c	1.4"		*/



#include "curses_inc.h"

_setecho(bf)
int	bf;
{
    SP->fl_echoit = bf;
    return (OK);
}
