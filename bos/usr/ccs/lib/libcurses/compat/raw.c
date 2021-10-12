static char sccsid[] = "@(#)77  1.1  src/bos/usr/ccs/lib/libcurses/compat/raw.c, libcurses, bos411, 9428A410j 9/2/93 13:29:49";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: raw
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
 * NAME:        raw
 */

raw()
{
#ifdef USG
	/* Disable interrupt characters */
	(cur_term->Nttyb).c_cc[VMIN] = 1;
	(cur_term->Nttyb).c_cc[VTIME] = 1;
	(cur_term->Nttyb).c_lflag &= ~(ICANON|ISIG);
#else
	(cur_term->Nttyb).sg_flags|=RAW;
#ifdef DEBUG
	if(outf) fprintf(outf,
		"raw(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
#endif
#endif
	SP->fl_rawmode=TRUE;
	reset_prog_mode();
	 
	return OK; /*P46613*/
}
