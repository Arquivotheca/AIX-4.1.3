static char sccsid[] = "@(#)08  1.6  src/bos/usr/ccs/lib/libcurses/compat/_scrollf.c, libcurses, bos411, 9428A410j 6/16/90 01:44:23";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _scrollf
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

extern	int	_outch();

/*
 * NAME:        _scrollf
 *
 * FUNCTION:
 *
 *      Scroll the terminal forward n lines, bringing up blank lines from
 *      bottom.  This only affects the current scrolling region.
 */

_scrollf(n)
int n;
{
	register int i;

	if( scroll_forward )
	{
		_setwind();
		_pos( SP->des_bot_mgn, 0 );
		for( i=0; i<n; i++ )
		{
			tputs(scroll_forward, 1, _outch);
		}
		SP->ml_above += n;
		if( SP->ml_above + lines > lines_of_memory )
		{
			SP->ml_above = lines_of_memory - lines;
		}
	}
	else
	{
		/* If terminal can't do it, try delete line. */
		_pos(0, 0);
		_dellines(n);
	}
}
