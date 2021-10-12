static char sccsid[] = "@(#)01	1.6  src/bos/usr/ccs/lib/libcurses/compat/_ll_move.c, libcurses, bos411, 9428A410j 6/16/90 01:43:57";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _ll_move
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

extern	struct	line	*_line_alloc();

/*
 * NAME:        _ll_move
 *
 * FUNCTION:
 *
 *      Position the cursor at position (row,col)
 *      in the virtual screen
 */

_ll_move (row, col)
register int row, col; 
{
	register struct line *p;
	register int l;
	register chtype *b1, *b2;
	register int rp1 = row+1;

#ifdef DEBUG
	if(outf) fprintf(outf, "_ll_move(%d, %d)\n", row, col);
#endif
	if (SP->virt_y >= 0 && (p=SP->std_body[SP->virt_y+1]) &&
		p->length < SP->virt_x)
		p->length = SP->virt_x >= columns ? columns : SP->virt_x;
	SP->virt_x = col;
	SP->virt_y = row;
	if (row < 0 || col < 0)
		return;
	if (!SP->std_body[rp1] || SP->std_body[rp1] == SP->cur_body[rp1]) {
		p = _line_alloc ();
		if (SP->cur_body[rp1]) {
			p->length = l = SP->cur_body[rp1]->length;
			b1 = &(p->body[0]);
			b2 = &(SP->cur_body[rp1]->body[0]);
			for ( ; l>0; l--)
				*b1++ = *b2++;
		}
		SP->std_body[rp1] = p;
	}
	p = SP->std_body[rp1];
	p -> hash = 0;
	while (p -> length < col)
		p -> body[p -> length++] = ' ';
	SP->curptr = &(SP->std_body[rp1] -> body[col]);
}
