static char sccsid[] = "@(#)87	1.6  src/bos/usr/ccs/lib/libcurses/compat/_delchars.c, libcurses, bos411, 9428A410j 6/16/90 01:43:02";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _delchars
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
 * NAME:        _delchars
 *
 * FUNCTION:
 *
 *      Delete n characters.
 *
 */

_delchars (n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_delchars(%d).\n", n);
#endif
	if (enter_delete_mode) {
		if (strcmp(enter_delete_mode, enter_insert_mode) == 0) {
			if (SP->phys_irm == 0) {
				tputs(enter_delete_mode,1,_outch);
				SP->phys_irm = 1;
			} 
		}
		else {
			if (SP->phys_irm == 1) {
				tputs(exit_insert_mode,1,_outch);
				SP->phys_irm = 0;
			}
			tputs(enter_delete_mode,1,_outch);
		}
	}
	while (--n >= 0) {
		/* Only one line can be affected by our delete char */
		tputs(delete_character, 1, _outch);
	}
	if (exit_delete_mode) {
		if (strcmp(exit_delete_mode, exit_insert_mode) == 0)
			SP->phys_irm = 0;
		else
			tputs(exit_delete_mode, 1, _outch);
	}
}
