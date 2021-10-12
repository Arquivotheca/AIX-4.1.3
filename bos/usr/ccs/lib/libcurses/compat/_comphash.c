static char sccsid[] = "@(#)85	1.6  src/bos/usr/ccs/lib/libcurses/compat/_comphash.c, libcurses, bos411, 9428A410j 6/16/90 01:42:53";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _comphash
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
 * NAME:        _comphash
 */

_comphash (p)
register struct line   *p;
{
	register chtype  *c, *l;
	register int rc;

	if (p == NULL)
		return;
	if (p -> hash)
		return;
	rc = 1;
	c = p -> body;
	l = &p -> body[p -> length];
	while (--l > c && *l == ' ')
		;
	while (c <= l && *c == ' ')
		c++;
	p -> bodylen = l - c + 1;
	while (c <= l)
		rc = (rc<<1) + *c++;
	p -> hash = rc;
	if (p->hash == 0)
		p->hash = 123;
}
