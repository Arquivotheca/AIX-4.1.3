static char sccsid[] = "@(#)62  1.6  src/bos/usr/ccs/lib/libcurses/compat/pnoutrfrsh.c, libcurses, bos411, 9428A410j 2/13/91 22:06:21";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   pnoutrefresh
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
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

#include	"cursesext.h"

extern	WINDOW *lwin;

/*
 * NAME:        pnoutrefresh
 *
 * FUNCTION:
 *
 *      Put out pad but don't actually update screen.
 */

pnoutrefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
register WINDOW	*pad;
int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
{
	register int pr, r, c;
	register chtype	*nsp, *lch;

# ifdef DEBUG
	if(outf) fprintf(outf,
		"PREFRESH(pad %x, pcorner %d,%d, smin %d,%d, smax %d,%d)",
		pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol);
	_dumpwin(pad);
	if(outf) fprintf(outf, "PREFRESH:\n\tfirstch\tlastch\n");
# endif

	/* Make sure everything fits */
	if (pminrow < 0) pminrow = 0;
	if (pmincol < 0) pmincol = 0;
	if (sminrow < 0) sminrow = 0;
	if (smincol < 0) smincol = 0;
	if (smaxrow >= lines) smaxrow = lines-1;
	if (smaxcol >= columns) smaxcol = columns-1;
	if (smaxrow - sminrow > pad->_maxy - pminrow)
		smaxrow = sminrow + (pad->_maxy - pminrow);

	/* Copy it out, like a refresh, but appropriately offset */
	for (pr=pminrow,r=sminrow; r <= smaxrow; r++,pr++) {
		/* No record of what previous loc looked like, so do it all */
		lch = &pad->_y[pr][pad->_maxx-1];
		nsp = &pad->_y[pr][pmincol];
		_ll_move(r, smincol);
		for (c=smincol; nsp<=lch; c++) {
			if (SP->virt_x++ < columns && c <= smaxcol)
				*SP->curptr++ = *nsp++;
			else
				break;
		}
		pad->_firstch[pr] = _NOCHANGE;
	}
	/*
	 * If the cursor is on the screen then lets move it to the
	 * proper place.  This might not always be true.
	 */
	c = pad->_curx - pmincol + smincol;
	if (c < 0 || c > smaxcol)
	    c = -1;
	r = pad->_cury - pminrow + sminrow;
	if (r < 0 || r > smaxrow)
	    r = -1;
	_ll_move(r, c);

	lwin = pad;
	return OK;
}
