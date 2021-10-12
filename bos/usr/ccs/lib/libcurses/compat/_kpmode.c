static char sccsid[] = "@(#)99	1.6  src/bos/usr/ccs/lib/libcurses/compat/_kpmode.c, libcurses, bos411, 9428A410j 6/16/90 01:43:49";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _kpmode
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
 * NAME:        _kpmode
 */

_kpmode(m)
{
#ifdef DEBUG
	if (outf) fprintf(outf, "kpmode(%d), SP->kp_state %d\n",
	m, SP->kp_state);
#endif
	if (m == SP->kp_state)
		return;
	if (m)
		tputs(keypad_xmit, 1, _outch);
	else
		tputs(keypad_local, 1, _outch);
	SP->kp_state = m;
}
