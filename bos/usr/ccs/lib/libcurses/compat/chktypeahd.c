static char sccsid[] = "@(#)27  1.5  src/bos/usr/ccs/lib/libcurses/compat/chktypeahd.c, libcurses, bos411, 9428A410j 6/16/90 01:45:35";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _chk_typeahead
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
extern	int InputPending;

/*
 * NAME:        _chk_typeahead
 *
 * FUNCTION:
 *
 *      If it's been long enough, check to see if we have any typeahead
 *      waiting.  If so, we quit this update until next time.
 */

_chk_typeahead()
{

#ifdef FIONREAD
# ifdef DEBUG
if(outf) fprintf(outf,
"end of _id_char: --SP->check_input %d, InputPending %d, chars buffered %d: ",
SP->check_input-1, InputPending, (SP->term_file->_ptr-SP->term_file->_base));
# endif
	if(--SP->check_input<0 && !InputPending &&
	    ((SP->term_file->_ptr - SP->term_file->_base) > 20)) {
		__cflush();
		if (SP->check_fd >= 0)
			ioctl(SP->check_fd, FIONREAD, &InputPending);
		else
			InputPending = 0;
		SP->check_input = SP->baud / 2400;
# ifdef DEBUG
		if(outf) fprintf(outf,
		"flush, ioctl returns %d, SP->check_input set to %d\n",
			InputPending, SP->check_input);
# endif
	}
# ifdef DEBUG
	if(outf) fprintf(outf, ".\n");
# endif
#endif
}
