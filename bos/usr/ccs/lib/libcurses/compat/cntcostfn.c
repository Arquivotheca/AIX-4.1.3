static char sccsid[] = "@(#)34  1.5  src/bos/usr/ccs/lib/libcurses/compat/cntcostfn.c, libcurses, bos411, 9428A410j 6/16/90 01:45:59";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _countchar, _cost_fn
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

static counter = 0;

/*
 * NAME:        _countchar
 */

_countchar(ch)
char ch;
{
	counter++;
}

/*
 * NAME:        _cost_fn
 *
 * FUNCTION:
 *
 *      Figure out the _cost in characters to print this string.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Due to padding, we can't just use strlen, so instead we
 *      feed it through tputs and trap the results.
 *      Even if the terminal uses xon/xoff handshaking, count the
 *      pad chars here since they estimate the real time to do the
 *      operation, useful in calculating costs.
 */

_cost_fn(str, affcnt)
char *str;
{
	int save_xflag = xon_xoff;

	if (str == NULL)
		return INFINITY;
	counter = 0;
	xon_xoff = 0;
	tputs(str, affcnt, _countchar);
	xon_xoff = save_xflag;
	return counter;
}
