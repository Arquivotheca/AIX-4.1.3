static char sccsid[] = "@(#)09  1.6  src/bos/usr/ccs/lib/libcurses/compat/_sethl.c, libcurses, bos411, 9428A410j 6/16/90 01:44:27";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _sethl
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
 * NAME:        _sethl
 */

_sethl ()
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"_sethl().  SP->phys_gr=%o, SP->virt_gr %o\n",
			SP->phys_gr, SP->virt_gr);
#endif
#ifdef	 	VIDEO
	if (SP->phys_gr == SP->virt_gr)
		return;
	vidputs(SP->virt_gr, _outch);
	SP->phys_gr = SP->virt_gr;
	/* Account for the extra space the cookie takes up */
	if (magic_cookie_glitch >= 0)
		SP->phys_x += magic_cookie_glitch;
#endif 		VIDEO
}
