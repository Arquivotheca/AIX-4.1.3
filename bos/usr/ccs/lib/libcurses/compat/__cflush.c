static char sccsid[] = "@(#)79	1.6  src/bos/usr/ccs/lib/libcurses/compat/__cflush.c, libcurses, bos411, 9428A410j 6/16/90 01:42:28";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS: __cflush
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
 * NAME:        __cflush
 *
 * FUNCTION:    Flush stdout.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is one of the main things
 *      in this level of curses that depends on the outside
 *      environment.
 */

__cflush()
{
	fflush(SP->term_file);
}
