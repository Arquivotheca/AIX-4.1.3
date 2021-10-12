#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)63  1.1  src/bos/usr/ccs/lib/libcurses/V3.m_clear.c, libcurses, bos411, 9428A410j 9/3/93 14:59:34";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		m_clear
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

/* #ident	"@(#)curses:screen/V3.m_clear.c	1.1"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
m_clear()
{
    return (wclear(stdscr));
}
#endif	/* _VR3_COMPAT_CODE */
