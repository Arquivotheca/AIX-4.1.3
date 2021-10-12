static char sccsid[] = "@(#)65  1.6  src/bos/usr/ccs/lib/libcurses/compat/m_tstp.c, libcurses, bos411, 9428A410j 6/16/90 01:49:22";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_tstp
 *
 * ORIGINS: 3, 10, 26, 27
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

# include	"cursesext.h"
# include	<signal.h>

# ifdef SIGTSTP

/*
 * NAME:        m_tstp
 *
 * FUNCTION:
 *
 *      Handle stop and start signals.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mini.c contains versions of curses routines for minicurses.
 *      They work just like their non-mini counterparts but draw on
 *      std_body rather than stdscr.  This cuts down on overhead but
 *      restricts what you are allowed to do - you can't get stuff back
 *      from the screen and you can't use multiple windows or things
 *      like insert/delete line (the logical ones that affect the screen).
 */

m_tstp() {

# ifdef DEBUG
	if (outf) fflush(outf);
# endif
	_ll_move(lines-1, 0);
	endwin();
	fflush(stdout);
	kill(0, SIGTSTP);
	signal((int)SIGTSTP, ((void (*)(int))(int) m_tstp));
	reset_prog_mode();
	SP->doclear = 1;
	_ll_refresh(0);
}
# endif
