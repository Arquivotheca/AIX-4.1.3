static char sccsid[] = "@(#)00  1.3  src/bos/usr/ccs/lib/libcurses/compat/getcap.c, libcurses, bos411, 9428A410j 6/16/90 01:47:32";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   getcap
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
 * NAME:        getcap
 *
 * FUNCTION:
 *
 *      Return a capability from termcap
 */

char *
getcap(name)
char *name;
{
	char *tgetstr();
	char *aoftspace;                        /* For compatibility.   */

	return tgetstr(name, &aoftspace);
}
