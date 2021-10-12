static char sccsid[] = "@(#)92	1.8  src/bos/usr/ccs/lib/libcurses/compat/_fixdelay.c, libcurses, bos411, 9428A410j 3/16/91 02:48:09";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _fixdelay
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
 * NAME:        _fixdelay
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The use has just changed his notion of whether we want nodelay
 *      mode.   Do any system dependent processing.
 *
 */

#ifndef FIONREAD

_fixdelay(old, new)
char old, new;
{
#ifdef USG
# include <fcntl.h>
	int fl, rv, fd;
	extern int errno;
	FILE	*inf;

	inf = SP -> term_file;
	if( inf == stdout )
	{
		inf = stdin;
	}
	fd = fileno( inf );
	fl = fcntl(fd, F_GETFL, 0);
	if (new)
		fl |= O_NDELAY;
	else
		fl &= ~O_NDELAY;
	if (old != new)
	{
		rv = fcntl(fd, F_SETFL, fl);
		SP->fl_nodelay = new;
	}
#else
	/* No system dependent processing on the V7 or Berkeley systems. */
#endif
}
#endif
