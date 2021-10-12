static char sccsid[] = "@(#)67  1.5  src/bos/usr/ccs/lib/libcurses/compat/line_alloc.c, libcurses, bos411, 9428A410j 6/16/90 01:48:43";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _line_alloc
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

/*
 * NAME:        _line_alloc
 *
 * FUNCTION:
 *
 *      Return a pointer to a new line structure.
 */

struct line *
_line_alloc ()
{
	register struct line   *rv = SP->freelist;
	char *calloc();

#ifdef DEBUG
	if(outf) fprintf(outf,
	"mem: _line_alloc (), prev SP->freelist %x\n", SP->freelist);
#endif
	if (rv) {
		SP->freelist = rv -> next;
	} else {
#ifdef NONSTANDARD
		_ec_quit("No lines available in line_alloc", "");
#else
		rv = (struct line *) calloc (1, sizeof *rv);
		rv -> body = (chtype *) calloc (columns, sizeof (chtype));
#endif
	}
	rv -> length = 0;
	rv -> hash = 0;
	return rv;
}
