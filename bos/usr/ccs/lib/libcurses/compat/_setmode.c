static char sccsid[] = "@(#)10  1.6  src/bos/usr/ccs/lib/libcurses/compat/_setmode.c, libcurses, bos411, 9428A410j 6/16/90 01:44:31";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _setmode
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
 * NAME:        _setmode
 */

_setmode ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_setmode().\n");
#endif
	if (SP->virt_irm == SP->phys_irm)
		return;
	tputs(SP->virt_irm==1 ? enter_insert_mode : exit_insert_mode,
							0, _outch);
	SP->phys_irm = SP->virt_irm;
}
