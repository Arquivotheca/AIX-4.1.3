static char sccsid[] = "@(#)47  1.1  src/bos/usr/ccs/lib/libcurses/compat/endwin.c, libcurses, bos411, 9428A410j 9/2/93 12:29:25";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   endwin
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"cursesext.h"

extern	int	_endwin;
extern	int	_c_clean();
extern	int	_outch();
extern	int	_pos();
extern	int	doupdate();
extern	int	reset_shell_mode();
extern	int	tputs();

/*
 * NAME:        endwin
 *
 * FUNCTION:
 *
 *      Clean things up before exiting.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      endwin is TRUE if we have called endwin - this avoids calling it
 *      twice.
 */

int
endwin()
{
	int saveci = SP->check_input;

	if (_endwin)
		return;

	/* Flush out any output not output due to typeahead */
	SP->check_input = 9999;
	doupdate();
	SP->check_input = saveci;	/* in case of another initscr */

	_fixdelay(SP->fl_nodelay, FALSE);
	if (stdscr->_use_meta)
		tputs(meta_off, 1, _outch);
	_pos(lines-1, 0);
	_c_clean();
	_endwin = TRUE;
	reset_shell_mode();
	fflush(stdout);
}
