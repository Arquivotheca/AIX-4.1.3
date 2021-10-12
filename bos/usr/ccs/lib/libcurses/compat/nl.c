static char sccsid[] = "@(#)50  1.7  src/bos/usr/ccs/lib/libcurses/compat/nl.c, libcurses, bos411, 9428A410j 6/16/90 01:50:36";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   nl
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
 * NAME:        nl
 */

nl()	
{


#ifdef USG
	(cur_term->Nttyb).c_iflag |= ICRNL;
	(cur_term->Nttyb).c_oflag |= ONLCR;

# ifdef DEBUG
	if(outf) fprintf(outf,
		"nl(), file %x, SP %x, flags %x,%x\n",
		SP->term_file, SP, cur_term->Nttyb.c_iflag,
		cur_term->Nttyb.c_oflag);
# endif
#else
	(cur_term->Nttyb).sg_flags |= CRMOD;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nl(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	reset_prog_mode();
}
