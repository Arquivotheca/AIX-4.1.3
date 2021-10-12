static char sccsid[] = "@(#)56  1.1  src/bos/usr/ccs/lib/libcurses/compat/killchar.c, libcurses, bos411, 9428A410j 9/2/93 12:50:58";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   killchar
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
 * NAME:        killchar
 */

char
killchar()
{
#ifdef USG
	return cur_term->Ottyb.c_cc[VKILL];
#else
	return cur_term->Ottyb.sg_kill;
#endif
}
