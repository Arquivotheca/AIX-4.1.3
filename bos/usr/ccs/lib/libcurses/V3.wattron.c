#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)76  1.1  src/bos/usr/ccs/lib/libcurses/V3.wattron.c, libcurses, bos411, 9428A410j 9/3/93 15:00:17";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		wattron
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

/*# ident	"@(#)curses:screen/V3.wattron.c	1.1"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	wattron
wattron(win, attrs)
WINDOW		*win;
_ochtype	attrs;
{
    win->_attrs |= _FROM_OCHTYPE(attrs) & A_ATTRIBUTES;
    return (OK);
}
#endif	/* _VR3_COMPAT_CODE */
