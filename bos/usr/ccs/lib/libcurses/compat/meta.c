static char sccsid[] = "@(#)61  1.1  src/bos/usr/ccs/lib/libcurses/compat/meta.c, libcurses, bos411, 9428A410j 9/2/93 13:01:48";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: meta
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

#include "cursesext.h"

/*
 * NAME:        meta
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => all 8 bits of input character should be passed through.
 */

meta(win,bf)
WINDOW *win; int bf;
{
	int _outch();

	if (!has_meta_key)
		return ERR;
	/*
	 * Do the appropriate fiddling with the tty driver to make it send
	 * all 8 bits through.  On USG this means clearing ISTRIP, on
	 * V7 you have to resort to RAW mode.
	 */
#ifdef USG
/*
 *  The following code has been removed because it is no
 * longer need to in the IBM environment where all terminals
 * are setup to 8 eight bits char
 *
 *  This problem was reported in the defect # 29255.
 *  It is set here just as a reference.
 */

	if (bf) {
		(cur_term->Nttyb).c_iflag &= ~ISTRIP;
		(cur_term->Nttyb).c_cflag &= ~CSIZE;
		(cur_term->Nttyb).c_cflag |= CS8;
		(cur_term->Nttyb).c_cflag &= ~PARENB;
	} else {
/*  Previous seven bit char terminal set */
	}
#else
	if (bf)
		raw();
	else
		noraw();
#endif
	reset_prog_mode();

	/*
	 * Do whatever is needed to put the terminal into meta-mode.
	 */
	if (bf)
		tputs(meta_on, 1, _outch);
	else
		tputs(meta_off, 1, _outch);

	/* Keep track internally. */
	win->_use_meta = bf;

	return OK;
}
