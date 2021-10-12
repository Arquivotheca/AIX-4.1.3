#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)58  1.1  src/bos/usr/ccs/lib/libcurses/_mvwinstr.c, libcurses, bos411, 9428A410j 9/3/93 15:04:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwinstr
 *		
 *
 *   ORIGINS: 27, 4
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#endif /* _POWER_PROLOG_ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/_mvwinstr.c	1.1"		*/


#define		NOMACROS
#include	"curses_inc.h"

mvwinstr(win, y, x, s)
WINDOW *win;
int	y, x;
char	*s;
{
    return (wmove(win, y, x)==ERR?ERR:winstr(win, s));
}
