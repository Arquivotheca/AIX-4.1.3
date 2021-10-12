static char sccsid[] = "@(#)52  1.1  src/bos/usr/ccs/lib/libcurses/compat/has_il.c, libcurses, bos411, 9428A410j 9/2/93 12:41:52";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   has_il
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
 * NAME:        has_il
 *
 * FUNCTION:
 *
 *      Check for insert/delete line.
 */

has_il()
{
	return insert_line && delete_line || change_scroll_region;
}
