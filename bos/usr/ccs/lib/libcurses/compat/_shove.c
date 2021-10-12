static char sccsid[] = "@(#)12  1.7.1.1  src/bos/usr/ccs/lib/libcurses/compat/_shove.c, libcurses, bos411, 9428A410j 4/29/92 14:49:31";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _shove, _prstr
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
 * NAME:        _shove
 *
 * FUNCTION:
 *
 *      Shove right in body as much as necessary.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Note that we give the space the same attributes as the upcoming
 *      character, to force the cookie to be placed on the space.
 */

_shove(body, len, lno)
register chtype *body;
register int len, lno;
{
	register int j, k, prev = 0;
	register int curscol = SP->virt_x, cursincr = 0, shoved = 0;
	static chtype buf[256];

#ifdef DEBUG
	if(outf) fprintf(outf, "_shove('");
	_prstr(body, len);
	if(outf) fprintf(outf, "', %d, %d), SP->virt_x %d\n",
			len, lno, SP->virt_x);
#endif
	for (j=0, k=0; j<len; ) {
		if ((int)(body[j]&A_ATTRIBUTES) != prev) {
			shoved++;
			if ((body[j]&A_CHARTEXT) == ' ') {
				/* Using an existing space */
				buf[k] = ' ' | body[j]&A_ATTRIBUTES;
			} else if ((body[j-1]&A_CHARTEXT) == ' ') {
				/* Using previous existing space */
				buf[k-1] = ' ' | body[j]&A_ATTRIBUTES;
			} else {
				/* A space is inserted here. */
				buf[k++] = ' ' | body[j]&A_ATTRIBUTES;
				if (j < curscol)
					cursincr++;
			}
		}
#ifdef DEBUG
		if(outf) fprintf(outf, "j %d, k %d, prev %o, new %o\n",
			j, k, prev, body[j] & A_ATTRIBUTES);
#endif
		prev = body[j] & A_ATTRIBUTES;
		buf[k++] = body[j++];
	}
	if (shoved) {
		/* k is 1 more than the last column of the line */
		if (k > columns)
			k = columns;
		if (buf[k-1]&A_ATTRIBUTES) {
			if (k < columns)
				k++;
			buf[k-1] = ' ';	/* All attributes off */
		}
		for (j=0; j<k; j++)
			body[j] = buf[j];
		len = k;
	}
	if (cursincr && lno == SP->virt_y+1)
		SP->virt_x += cursincr;
#ifdef DEBUG
	if(outf) fprintf(outf, "returns '");
	_prstr(body, len);
	if(outf) fprintf(outf, "', len %d, SP->virt_x %d\n",
						len, SP->virt_x);
#endif
	return len;
}

/*
 * NAME:        _prstr
 */

#ifdef DEBUG
static
_prstr(result, len)
chtype *result;
int len;
{
	register chtype *cp;

	for (cp=result; *cp && cp < result+len; cp++)
		if (*cp >= ' ' && *cp <= '~') {
			if(outf) fprintf(outf, "%c", *cp);
		} else {
			if(outf) fprintf(outf, "<%o,%c>",
				*cp&A_ATTRIBUTES, *cp&A_CHARTEXT);
		}
}
#endif
