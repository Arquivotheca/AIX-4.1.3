#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)96  1.1  src/bos/usr/ccs/lib/libcurses/_crmode.c, libcurses, bos411, 9428A410j 9/3/93 15:01:45";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		crmode
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

/* #ident	"@(#)curses:screen/_crmode.c	1.2"		*/



#define		NOMACROS
#include	"curses_inc.h"

#undef	crmode
crmode()
{
    return (cbreak());
}
