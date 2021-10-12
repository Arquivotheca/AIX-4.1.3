static char sccsid[] = "@(#)83	1.7  src/bos/usr/ccs/lib/libcurses/compat/_clearhl.c, libcurses, bos411, 9428A410j 6/16/90 01:42:45";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _clearhl
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
 * NAME:        _clearhl
 */

_clearhl ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_clearhl().\n");
#endif
	if (SP->phys_gr) {
		chtype oldes = SP->virt_gr;
		SP->virt_gr = 0;
		_sethl ();
		SP->virt_gr = oldes;
	}
}
