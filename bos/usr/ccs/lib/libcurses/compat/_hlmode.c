static char sccsid[] = "@(#)94	1.6  src/bos/usr/ccs/lib/libcurses/compat/_hlmode.c, libcurses, bos411, 9428A410j 6/16/90 01:43:28";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _hlmode
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
 * NAME:        _hlmode
 */

_hlmode (on)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_hlmode(%o).\n", on);
#endif
	SP->virt_gr = on;
}
