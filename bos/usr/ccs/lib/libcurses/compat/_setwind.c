static char sccsid[] = "@(#)11  1.6  src/bos/usr/ccs/lib/libcurses/compat/_setwind.c, libcurses, bos411, 9428A410j 6/16/90 01:44:35";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _setwind
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
 * NAME:        _setwind
 *
 * FUNCTION:
 *
 *      Force the window to be as desired
 */

_setwind()
{
	if (	SP->phys_top_mgn == SP->des_top_mgn &&
		SP->phys_bot_mgn == SP->des_bot_mgn) {
#ifdef DEBUG
		if(outf) fprintf(outf,
			"_setwind, same values %d & %d, do nothing\n",
				SP->phys_top_mgn, SP->phys_bot_mgn);
#endif
		return;
	}
	if (set_window)
		tputs(tparm(set_window, SP->des_top_mgn,
			SP->des_bot_mgn, 0, columns-1), 1, _outch);
	else if (change_scroll_region) {
		/* Save & Restore SP->curptr since it becomes undefined */
		tputs(save_cursor, 1, _outch);
		tputs(tparm(change_scroll_region,
			SP->des_top_mgn, SP->des_bot_mgn), 1, _outch);
					/* put SP->curptr back */
		tputs(restore_cursor, 1, _outch);
	}
#ifdef DEBUG
	if(outf) fprintf(outf, "set phys window from (%d,%d) to (%d,%d)\n",
	SP->phys_top_mgn, SP->phys_bot_mgn, SP->des_top_mgn, SP->des_bot_mgn);
#endif
	SP->phys_top_mgn = SP->des_top_mgn;
	SP->phys_bot_mgn = SP->des_bot_mgn;
}
