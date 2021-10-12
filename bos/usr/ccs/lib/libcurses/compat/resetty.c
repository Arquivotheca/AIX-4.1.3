static char sccsid[] = "@(#)79  1.1  src/bos/usr/ccs/lib/libcurses/compat/resetty.c, libcurses, bos411, 9428A410j 9/2/93 13:46:43";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:  resetty
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
 * NAME:        resetty
 */

resetty()
{
#ifdef USG
	if (SP == NULL || (SP->save_tty_buf.c_cflag&CBAUD) == 0)
		return;	/* Never called savetty */
#else
	if (SP == NULL || SP->save_tty_buf.sg_ospeed == 0)
		return;	/* Never called savetty */
#endif
	cur_term->Nttyb = SP->save_tty_buf;
#ifdef DEBUG
# ifdef USG
	if(outf) fprintf(outf,
		"savetty(), file %x, SP %x, flags %x,%x,%x,%x\n",
		SP->term_file, SP, cur_term->Nttyb.c_iflag,
		cur_term->Nttyb.c_oflag, cur_term->Nttyb.c_cflag,
		cur_term->Nttyb.c_lflag);
# else
	if(outf) fprintf(outf,
		"resetty(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	reset_prog_mode();


	return OK; /*P46613*/
}
