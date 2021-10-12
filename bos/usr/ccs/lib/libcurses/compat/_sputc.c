static char sccsid[] = "@(#)15  1.6  src/bos/usr/ccs/lib/libcurses/compat/_sputc.c, libcurses, bos411, 9428A410j 6/16/90 01:44:48";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _sputc
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

# include	"cursesext.h"

/*
 * NAME:        _sputc
 */

#ifdef DEBUG
_sputc(c, f)
chtype c;
FILE *f;
{
	int so;

	so = c & A_ATTRIBUTES;
	c &= 0177;
	if (so) {
		putc('<', f);
		fprintf(f, "%o,", so);
	}
	putc(c, f);
	if (so)
		putc('>', f);
}
#endif
