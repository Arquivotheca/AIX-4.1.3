#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)59  1.1  src/bos/usr/ccs/lib/libcurses/V3.box.c, libcurses, bos411, 9428A410j 9/3/93 14:59:20";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: box
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

/* #ident	"@(#)curses:screen/V3.box.c	1.1"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	box
box(win, v, h)
WINDOW		*win;
_ochtype 	v, h;
{
    return (wborder(win, _FROM_OCHTYPE(v), _FROM_OCHTYPE(v),
	_FROM_OCHTYPE(h), _FROM_OCHTYPE(h), 0, 0, 0, 0));
}
#endif	/* _VR3_COMPAT_CODE */
