static char sccsid[] = "@(#)39  1.5  src/bos/usr/ccs/lib/libcurses/compat/resetterm.c, libcurses, bos411, 9428A410j 6/16/90 01:51:52";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   resetterm
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
#include <IN/uparm.h>

extern	struct term *cur_term;

/*
 * NAME:        resetterm
 */

resetterm()
{
	reset_shell_mode();
}
