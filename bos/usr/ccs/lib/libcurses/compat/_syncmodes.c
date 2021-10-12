static char sccsid[] = "@(#)16  1.6  src/bos/usr/ccs/lib/libcurses/compat/_syncmodes.c, libcurses, bos411, 9428A410j 6/16/90 01:44:52";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _syncmodes
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
 * NAME:        _syncmodes
 */

_syncmodes()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_syncmodes().\n");
#endif
	_sethl();
	_setmode();
	_setwind();
}
