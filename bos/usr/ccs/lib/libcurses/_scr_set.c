#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)74  1.1  src/bos/usr/ccs/lib/libcurses/_scr_set.c, libcurses, bos411, 9428A410j 9/3/93 15:05:20";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: scr_set
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

/* #ident	"@(#)curses:screen/_scr_set.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

scr_set(file)
char	*file;
{
    return (_scr_all(file, 2));
}
