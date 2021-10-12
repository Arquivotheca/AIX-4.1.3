static char sccsid[] = "@(#)99  1.1  src/bos/usr/ccs/lib/libcurses/compat/tgoto.c, libcurses, bos411, 9428A410j 9/2/93 14:05:36";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   tgoto
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

/*
 * NAME:        tgoto
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Function included only for upward compatibility with old termcap
 *      library.  Assumes exactly two parameters in the wrong order.
 */

char *
tgoto(cap, col, row)
char *cap;
int col, row;
{
	char *cp;
	char *tparm();

	cp = tparm(cap, row, col);
	return cp;
}
