static char sccsid[] = "@(#)93	1.6  src/bos/usr/ccs/lib/libcurses/compat/_forcehl.c, libcurses, bos411, 9428A410j 6/16/90 01:43:23";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _forcehl
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
 * NAME:        _forcehl
 *
 * FUNCTION:
 *
 *      Output the string to get us in the right highlight mode,
 *      no matter what mode we are currently in.
 */

_forcehl()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_forcehl().\n");
#endif
	SP->phys_gr = -1;
	_sethl();
}
