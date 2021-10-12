static char sccsid[] = "@(#)04  1.7  src/bos/usr/ccs/lib/libcurses/compat/nonl.c, libcurses, bos411, 9428A410j 6/16/90 01:50:56";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   nonl
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

#include        "cursesext.h"

/*
 * NAME:        nonl
 */

nonl()	
{


#ifdef USG
	(cur_term->Nttyb).c_iflag &= ~ICRNL;
	(cur_term->Nttyb).c_oflag &= ~ONLCR;

# ifdef DEBUG
	if(outf) fprintf(outf,
		"nonl(), file %x, SP %x, flags %x,%x\n",
		SP->term_file, SP, cur_term->Nttyb.c_iflag,
		cur_term->Nttyb.c_oflag);
# endif
#else
	(cur_term->Nttyb).sg_flags &= ~CRMOD;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nonl(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	reset_prog_mode();
}
