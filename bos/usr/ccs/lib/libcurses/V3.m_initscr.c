#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)65  1.1  src/bos/usr/ccs/lib/libcurses/V3.m_initscr.c, libcurses, bos411, 9428A410j 9/3/93 14:59:42";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		m_initscr
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

/* #ident	"@(#)curses:screen/V3.m_initscr.c	1.1"	*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
WINDOW	*
m_initscr()
{
    return (initscr());
}
#endif	/* _VR3_COMPAT_CODE */
