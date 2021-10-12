static char sccsid[] = "@(#)81	1.7  src/bos/usr/ccs/lib/libcurses/compat/_blanks.c, libcurses, bos411, 9428A410j 3/6/91 01:58:24";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _blanks
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

extern	int	_outch();
extern	int	_sethl();
extern	int	_setmode();
extern	char	*tparm();
extern	int	tputs();

/*
 * NAME:        _blanks
 *
 * FUNCTION:    Output n blanks, or the equivalent.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is done to erase text, and
 *      also to insert blanks.  The semantics of this call do not define
 *      where it leaves the cursor - it might be where it was before, or
 *      it might be at the end of the blanks.  We will, of course, leave
 *      SP->phys_x properly updated.
 */

_blanks (n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_blanks(%d).\n", n);
#endif
	if (n == 0)
		return;
	_setmode ();
	_sethl ();
	if (SP->virt_irm==1 && parm_ich) {
		if (n == 1)
			tputs(insert_character, 1, _outch);
		else
			tputs(tparm(parm_ich, n), n, _outch);
		return;
	}
	if (erase_chars && SP->phys_irm != 1 && n > 5) {
		tputs(tparm(erase_chars, n), n, _outch);
		return;
	}
	if (repeat_char && SP->phys_irm != 1 && n > 5) {
		int more = n;

		/* Don't trust repeat char into the last column */
		if ((SP->phys_x + more) >= (columns - 1))
			more = SP->phys_x - (columns - 2);
		tputs(tparm(repeat_char, ' ', more), more, _outch);
		SP->phys_x += more;
		n -= more;
	}
	while (--n >= 0) {
		if (SP->phys_irm == 1 && insert_character)
			tputs (insert_character,
				columns - SP->phys_x, _outch);
		if (++SP->phys_x >= columns && auto_right_margin) {
			if (SP->phys_y >= lines-1) {
				/*
				 * We attempted to put something in the last
				 * position of the last line.  Since this will
				 * cause a scroll (we only get here if the
				 * terminal has auto_right_margin) we refuse
				 * to put it out.
				 */
				SP->phys_x--;
				return;
			}
			SP->phys_x = 0;
			SP->phys_y++;
		}
		_outch (' ');
		if (SP->phys_irm == 1 && insert_padding)
			tputs (insert_padding, 1, _outch);
	}
	/* If we "think" we moved past the right edge, then more back */
	if (!auto_right_margin && SP->phys_x >= columns)
		SP->phys_x = columns - 1;
}
