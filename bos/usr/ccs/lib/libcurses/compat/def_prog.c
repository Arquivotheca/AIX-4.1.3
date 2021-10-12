static char sccsid[] = "@(#)41  1.1  src/bos/usr/ccs/lib/libcurses/compat/def_prog.c, libcurses, bos411, 9428A410j 9/2/93 12:19:43";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   def_prog_mode
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

#include	"curses.h"
#include	"term.h"

extern	struct term *cur_term;

/*
 * NAME:        def_prog_mode
 */

def_prog_mode()
{
#ifdef USG
	ioctl(cur_term -> Filedes, TCGETA, &(cur_term->Nttyb));
#else
	ioctl(cur_term -> Filedes, TIOCGETP, &(cur_term->Nttyb));
#endif
}
