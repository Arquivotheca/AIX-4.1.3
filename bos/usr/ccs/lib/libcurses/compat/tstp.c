static char sccsid[] = "@(#)06  1.1  src/bos/usr/ccs/lib/libcurses/compat/tstp.c, libcurses, bos411, 9428A410j 9/2/93 14:10:41";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   _tstp
 *
 * ORIGINS: 3, 10, 26, 27
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

# include	<signal.h>

# ifdef SIGTSTP

# include	"cursesext.h"

/*
 * NAME:        _tstp
 *
 * FUNCTION:
 *
 *      Handle stop and start signals
 */

_tstp() {

# ifdef DEBUG
	if (outf) fflush(outf);
# endif
	_ll_move(lines-1, 0);
	endwin();
	fflush(stdout);
	kill(0, SIGTSTP);
	signal((int)SIGTSTP, ((void (*)(int))(int) _tstp));
	fixterm();
	SP->doclear = 1;
	curscr->_clear = FALSE;
	doupdate();
}
# endif
