static char sccsid[] = "@(#)03	1.6  src/bos/usr/ccs/lib/libcurses/compat/_outch.c, libcurses, bos411, 9428A410j 6/16/90 01:44:02";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _outch
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

#ifdef NLS
#include <NLchar.h>
#endif NLS

#include "cursesext.h"

int outchcount;

/*
 * NAME:        _outch
 *
 * FUNCTION:
 *
 *      Write out one character to the tty.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is one of the main things
 *      in this level of curses that depends on the outside
 *      environment.
 */

_outch (c)
chtype c;
{
#ifdef DEBUG
# ifndef LONGDEBUG
	if (outf)
		if (c < ' ')
			fprintf(outf, "^%c", (c+'@')&0177);
		else
			fprintf(outf, "%c", c&0177);
# else LONGDEBUG
	if(outf) fprintf(outf, "_outch: char '%s' term %x file %x=%d\n",
		unctrl(c&0177), SP, SP->term_file, fileno(SP->term_file));
# endif LONGDEBUG
#endif DEBUG

#ifdef NLS
	if (!NCisshift(c))
		outchcount++;
#else
	outchcount++;
#endif
	if (SP && SP->term_file)
#ifdef NLS
		putc (c, SP->term_file);
#else
		putc (c&0177, SP->term_file);
#endif
	else
#ifdef NLS
		putc (c, stdout);
#else
		putc (c&0177, stdout);
#endif
}
