#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)54  1.1  src/bos/usr/ccs/lib/libcurses/pair_cont.c, libcurses, bos411, 9428A410j 9/3/93 15:10:46";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: pair_content
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

/* #ident	"@(#)curses:pair_cont.c	1.3"			*/



#include "curses_inc.h"

pair_content(pair, f, b)
short pair, *f, *b;
{
    register _Color_pair *ptp;

    if (pair < 1 || pair >= COLOR_PAIRS)
	return (ERR);

    ptp = SP->_pairs_tbl + pair;

    if (!ptp->init)
        return (ERR);

    *f = ptp->foreground;
    *b = ptp->background;
    return (OK);
}
