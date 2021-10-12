static char sccsid[] = "@(#)49  1.1  src/bos/usr/ccs/lib/libcurses/compat/flushinp.c, libcurses, bos411, 9428A410j 9/2/93 12:34:22";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   flushinp
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
 * NAME:        flushinp
 */

flushinp()
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"flushinp(), file %x, SP %x\n", SP->term_file, SP);
#endif
#ifdef USG
	ioctl(cur_term -> Filedes, TCFLSH, 0);
#else
	/* for insurance against someone using their own buffer: */
	ioctl(cur_term -> Filedes, TIOCGETP, &(cur_term->Nttyb));

	/*
	 * SETP waits on output and flushes input as side effect.
	 * Really want an ioctl like TCFLSH but Berkeley doesn't have one.
	 */
	ioctl(cur_term -> Filedes, TIOCSETP, &(cur_term->Nttyb));
#endif
	/*
	 * Have to doupdate() because, if we've stopped output due to
	 * typeahead, now that typeahead is gone, so we'd better catch up.
	 */
	doupdate();
}
