static char sccsid[] = "@(#)84	1.6  src/bos/usr/ccs/lib/libcurses/compat/_clearline.c, libcurses, bos411, 9428A410j 6/16/90 01:42:49";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _clearline
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

/*
 * NAME:        _clearline
 *
 * FUNCTION:
 *
 *      Position the cursor at the beginning of the
 *      indicated line and clears the line (in the image)
 */

_clearline (row)
{
	_ll_move (row, 0);
	SP->std_body[row+1] -> length = 0;
}
