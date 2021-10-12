#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)65  1.1  src/bos/usr/ccs/lib/libcurses/_noqiflush.c, libcurses, bos411, 9428A410j 9/3/93 15:04:51";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: noqiflush
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

/* #ident	"@(#)curses:screen/_noqiflush.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

void
noqiflush()
{
    _setqiflush(FALSE);
}
