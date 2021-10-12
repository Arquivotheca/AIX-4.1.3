static char sccsid[] = "@(#)93  1.1  src/bos/usr/ccs/lib/libcurses/compat/setterm.c, libcurses, bos411, 9428A410j 9/2/93 13:54:46";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:  setterm
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

# include	"cursesext.h"
# include	<signal.h>

char	*calloc();
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:        setterm
 *
 * FUNCTION:
 *
 *      Low level interface, for compatibility with old curses.
 */

setterm(type)
char *type;
{
	setupterm(type, 1, 0);
}
