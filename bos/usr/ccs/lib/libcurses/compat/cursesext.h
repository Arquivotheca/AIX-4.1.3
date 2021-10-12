/* @(#)40       1.9  src/bos/usr/ccs/lib/libcurses/compat/cursesext.h, libcurses, bos411, 9428A410j 3/16/91 02:48:12 */
#ifndef _H_CURSESEXT
#define _H_CURSESEXT

/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   cursesext.h
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

/*
 * NAME:        cursesext.h
 *
 * FUNCTION:
 *
 *      External variables for the library.
 */


/* LINTLIBRARY */

# define CURSES	/* We are internal to curses */

# ifdef NONSTANDARD
#  include "RecStruct.h"
#  include "VTio.h"
# endif ITC

# include "curses.h"
# include "term.h"
# include "curshdr.h"
# include "unctrl.h"

#ifdef FIONREAD
#define _fixdelay(a, b)
#endif

#endif /* _H_CURSESEXT */
