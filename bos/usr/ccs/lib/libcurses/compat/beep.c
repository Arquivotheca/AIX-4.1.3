static char sccsid[] = "@(#)22  1.6  src/bos/usr/ccs/lib/libcurses/compat/beep.c, libcurses, bos411, 9428A410j 6/16/90 01:45:18";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   beep
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
 * NAME:        beep
 */

beep()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "beep().\n");
#endif
    if (bell)
	tputs (bell, 0, _outch);
    else
	tputs (flash_screen, 0, _outch);
    __cflush();
}
