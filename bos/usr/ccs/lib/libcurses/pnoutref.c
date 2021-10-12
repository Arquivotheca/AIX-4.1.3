#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)56  1.1  src/bos/usr/ccs/lib/libcurses/pnoutref.c, libcurses, bos411, 9428A410j 9/3/93 15:10:54";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: pnoutrefresh
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

/* #ident	"@(#)curses:screen/pnoutref.c	1.11"		*/



#include	"curses_inc.h"

/* wnoutrefresh for pads. */

pnoutrefresh(pad, pby, pbx, sby, sbx, sey, sex)
WINDOW	*pad;
int	pby, pbx, sby, sbx, sey, sex;
{
    extern	int	wnoutrefresh();

    return (_prefresh(wnoutrefresh, pad, pby, pbx, sby, sbx, sey, sex));
}
