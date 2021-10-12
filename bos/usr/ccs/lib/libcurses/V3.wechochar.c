#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)78  1.1  src/bos/usr/ccs/lib/libcurses/V3.wechochar.c, libcurses, bos411, 9428A410j 9/3/93 15:00:23";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		wechochar
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

/* #ident	"@(#)curses:screen/V3.wechochar.c	1.1"	*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	wechochar
wechochar(win, c)
WINDOW		*win;
_ochtype	c;
{
    return (w32echochar(win, _FROM_OCHTYPE(c)));
}
#endif	/* _VR3_COMPAT_CODE */
