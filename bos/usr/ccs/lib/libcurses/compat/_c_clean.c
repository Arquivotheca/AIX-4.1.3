static char sccsid[] = "@(#)82	1.6  src/bos/usr/ccs/lib/libcurses/compat/_c_clean.c, libcurses, bos411, 9428A410j 6/16/90 01:42:41";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _c_clean
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
 * NAME:        _c_clean
 *
 * FUNCTION:    Clear screen.
 *
 */

_c_clean ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_c_clean().\n");
#endif
	_hlmode (0);
	_kpmode(0);
	SP->virt_irm = 0;
	_window(0, lines-1, 0, columns-1);
	_syncmodes();
	tputs(exit_ca_mode, 0, _outch);
	tputs(cursor_normal, 0, _outch);
}
