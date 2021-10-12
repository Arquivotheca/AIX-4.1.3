static char sccsid[] = "@(#)88  1.1  src/bos/usr/ccs/lib/libcurses/compat/savetty.c, libcurses, bos411, 9428A410j 9/2/93 13:48:53";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: savetty
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
 * NAME:        savetty
 */

savetty()
{
	SP->save_tty_buf = cur_term->Nttyb;
#ifdef DEBUG
# ifdef USG
	if(outf) fprintf(outf,
		"savetty(), file %x, SP %x, flags %x,%x,%x,%x\n",
		SP->term_file, SP, cur_term->Nttyb.c_iflag,
		cur_term->Nttyb.c_oflag, cur_term->Nttyb.c_cflag,
		cur_term->Nttyb.c_lflag);
# else
	if(outf) fprintf(outf,
		"savetty(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
}
