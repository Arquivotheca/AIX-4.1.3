static char sccsid[] = "@(#)97	1.6  src/bos/usr/ccs/lib/libcurses/compat/_inschars.c, libcurses, bos411, 9428A410j 6/16/90 01:43:41";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _inschars
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

char *tparm();

extern	int	_outch();

/*
 * NAME:        _inschars
 *
 * FUNCTION:
 *
 *      Insert n blank characters.
 */

_inschars(n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_inschars(%d).\n", n);
#endif
	if (enter_insert_mode && SP->phys_irm == 0) {
		tputs(enter_insert_mode, 1, _outch);
		SP->phys_irm = 1;
	}
	if (parm_ich && n > 1)
		tputs(tparm(parm_ich, n), n, _outch);
	else
		while (--n >= 0)
			tputs(insert_character, 1, _outch);
}
