static char sccsid[] = "@(#)51  1.1  src/bos/usr/ccs/lib/libcurses/compat/has_ic.c, libcurses, bos411, 9428A410j 9/2/93 12:39:35";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   has_ic
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
 * NAME:        has_ic
 *
 * FUNCTION:
 *
 *      Check for insert/delete char.
 */

has_ic()
{
	return insert_character || enter_insert_mode;
}
