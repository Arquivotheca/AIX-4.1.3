static char sccsid[] = "@(#)09  1.1  src/bos/usr/ccs/lib/libcurses/compat/typeahead.c, libcurses, bos411, 9428A410j 9/2/93 14:12:00";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   typeahead
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
 * NAME:        typeahead
 *
 * FUNCTION:
 *
 *      Set the file descriptor for typeahead checks to fd.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      fd can be -1 to disable the checking.
 */

typeahead(fd)
int fd;
{
	SP->check_fd = fd;
}
