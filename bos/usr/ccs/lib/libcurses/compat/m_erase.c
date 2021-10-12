static char sccsid[] = "@(#)32  1.5  src/bos/usr/ccs/lib/libcurses/compat/m_erase.c, libcurses, bos411, 9428A410j 6/16/90 01:49:09";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_erase
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

/*
 * NAME:        m_erase
 *
 * FUNCTION:
 *
 *      Erase the contents of the desired screen - set to all _blanks.
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

m_erase()
{
	register int n;

#ifdef DEBUG
	if (outf) fprintf(outf, "M_ERASE, erasing %d lines\n", lines);
#endif
	for (n = 0; n < lines; n++) {
		_clearline(n);
	}
}
