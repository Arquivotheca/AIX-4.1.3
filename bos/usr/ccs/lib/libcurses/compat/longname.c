static char sccsid[] = "@(#)65  1.1  src/bos/usr/ccs/lib/libcurses/compat/longname.c, libcurses, bos411, 9428A410j 9/2/93 13:08:15";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: longname
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
 * NAME:        longname
 *
 * FUNCTION:
 *
 *      Return the long name of the terminal.
 */

char *
longname()
{
	register char	*cp;
	extern char ttytype[];

	for (cp=ttytype; *cp++; )		/* Go to end of string */
		;
	while (*--cp != '|' && cp>=ttytype)     /* Backup to | or beginning */
		;
	return ++cp;
}
