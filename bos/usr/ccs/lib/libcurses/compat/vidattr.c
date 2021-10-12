static char sccsid[] = "@(#)95  1.5  src/bos/usr/ccs/lib/libcurses/compat/vidattr.c, libcurses, bos411, 9428A410j 6/16/90 01:54:21";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   vidattr
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

#include <stdio.h>

extern	int	_outchar();

/*
 * NAME:        vidattr
 *
 * FUNCTION:
 *
 *      Handy function to put out a string with padding.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      It makes two assumptions:
 *           (1) Output is via stdio to stdout through putchar.
 *           (2) There is no count of affected lines.  Thus, this
 *               routine is only valid for certain capabilities,
 *               i.e. those that don't have *'s in the documentation.
 */

vidattr(newmode)
int newmode;
{
	vidputs(newmode, _outchar);
}
