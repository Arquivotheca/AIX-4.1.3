static char sccsid[] = "@(#)71  1.1  src/bos/usr/ccs/lib/libcurses/compat/nocbreak.c, libcurses, bos411, 9428A410j 9/2/93 13:19:44";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: nocbreak
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
 * NAME:        nocbreak
 */

nocbreak()
{
#ifdef USG
	(cur_term->Nttyb).c_lflag |= ICANON;
	(cur_term->Nttyb).c_cc[VEOF] = (cur_term->Ottyb).c_cc[VEOF];
	(cur_term->Nttyb).c_cc[VEOL] = (cur_term->Ottyb).c_cc[VEOL];
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nocrmode(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.c_lflag);
# endif
#else
	(cur_term->Nttyb).sg_flags &= ~CBREAK;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nocrmode(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	SP->fl_rawmode=FALSE;
	reset_prog_mode();

	return OK; /*P46613*/
}
