static char sccsid[] = "@(#)39  1.5  src/bos/usr/ccs/lib/libcurses/compat/set_term.c, libcurses, bos411, 9428A410j 6/16/90 01:52:30";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   set_term
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

/*
 * NAME:        set_term
 */

struct screen *
set_term(new)
struct screen *new;
{
	register struct screen *rv = SP;

#ifdef DEBUG
	if(outf) fprintf(outf, "setterm: old %x, new %x\n", rv, new);
#endif

#ifndef		NONSTANDARD
	SP = new;
#endif		NONSTANDARD

	cur_term = SP->tcap;
	LINES = lines;
	COLS = columns;
	stdscr = SP->std_scr;
	curscr = SP->cur_scr;
	return rv;
}
