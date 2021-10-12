static char sccsid[] = "@(#)06  1.6  src/bos/usr/ccs/lib/libcurses/compat/_reset.c, libcurses, bos411, 9428A410j 6/16/90 01:44:14";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _reset
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
 * NAME:        _reset
 */

_reset ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_reset().\n");
#endif
	tputs(enter_ca_mode, 0, _outch);
	tputs(cursor_visible, 0, _outch);
	tputs(exit_attribute_mode, 0, _outch);
	tputs(clear_screen, 0, _outch);
	SP->phys_x = 0;
	SP->phys_y = 0;
	SP->phys_irm = 1;
	SP->virt_irm = 0;
	SP->phys_top_mgn = 4;
	SP->phys_bot_mgn = 4;
	SP->des_top_mgn = 0;
	SP->des_bot_mgn = lines-1;
	SP->ml_above = 0;
	_setwind();
}
