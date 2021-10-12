static char sccsid[] = "@(#)06  1.6  src/bos/usr/ccs/lib/libcurses/compat/writechars.c, libcurses, bos411, 9428A410j 3/6/91 01:58:27";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _writechars
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

char *tparm();

extern	int	_outch();

/*
 * NAME:        _writechars
 */

_writechars (start, end)
register char	*start, *end; 
{
	register int c;
	register char *p;
	extern int didntdobotright;	/* did not output char in corner */

#ifdef DEBUG
	if(outf) fprintf(outf, "_writechars(%d:'", end-start+1);
	if(outf) fwrite(start, sizeof (char), end-start+1, outf);
	if(outf) fprintf(outf, "').\n");
#endif	DEBUG
	_setmode ();
	_sethl();
	while( start <= end ) {
#ifdef FULLDEBUG
		if(outf) fprintf(outf,
				 "wc loop: repeat_char '%s', SP->phys_irm %d, *start '%c'\n",
				 repeat_char, SP->phys_irm, *start);
#endif  FULLDEBUG
		if (repeat_char && SP->phys_irm != 1 &&
		    ((p=start+1),*start==*p++) && (*start==*p++) &&
		    (*start==*p++) && (*start==*p++) && p<=end) {
			/* We have a run of at least 5 characters */
			c = 5;
			while (p <= end && *start == *p++)
				c++;
			SP->phys_x += c;
			/*
			 * Don't assume anything about how repeat and auto
			 * margins interact.  The concept is different.
			 * Backup to the last column minus 1 just in case
			 */
			if (SP->phys_x >= columns - 1) {
				int back = SP->phys_x - (columns - 2);
				SP->phys_x -= back;
				c -= back;
				p -= back;
			}
#ifdef DEBUG
			if(outf) fprintf(outf,
					 "using repeat, count %d, char '%c'\n",
					 c, *start);
#endif	DEBUG
			tputs(tparm(repeat_char, *start, c), c, _outch);
			start = p-1;
			if (*start == start[-1])
				start++;
			continue;
		}
		c = *start++;
#ifdef DEBUG
		if (outf) fprintf(outf,
			"c is '%c', phys_x %d, phys_y %d\n",
			c, SP->phys_x, SP->phys_y);
#endif  DEBUG
		if(SP->phys_irm == 1 && insert_character)
		{
			tputs(insert_character, columns-SP->phys_x, _outch);
		}
		/*
		 * If transparent_underline && !erase_overstrike,
		 * should probably do clr_eol.  No such terminal yet.
		 */
		if(transparent_underline && erase_overstrike &&
		   c == '_' && SP->phys_irm != 1)
		{
			_outch (' ');
			tputs(cursor_left, 1, _outch);
		}			
		if( ++SP->phys_x >= columns && auto_right_margin )
		{
					/* Have to on c100 anyway..*/
			if( SP->phys_y >= lines-1 /*&& !eat_newline_glitch*/ )
			{
				/*
				 * We attempted to put something in the last
				 * position of the last line.  Since this will
				 * cause a scroll (we only get here if the
				 * terminal has auto_right_margin) we refuse
				 * to put it out.
				 */
#ifdef DEBUG
				if(outf) fprintf(outf,
					"Avoiding lower right corner\n");
#endif
				didntdobotright = 1;
				SP->phys_x--;
				return;
			}
			SP->phys_x = 0;
			SP->phys_y++;
		}
		if( tilde_glitch && c == '~' )
		{
			_outch('`');
		}
		else
		{
			_outch (c);
		}
		/* Only 1 line can be affected by insert char here */
		if( SP->phys_irm == 1 && insert_padding ) {
			tputs(insert_padding, 1, _outch);
		}
		if( eat_newline_glitch && SP->phys_x == 0 ) {
			/*
			 * This handles both C100 and VT100, which are
			 * different.  We don't output carriage_return
			 * and cursor_down because it might confuse a
			 * terminal that is looking for return and linefeed.
			 */
			_outch('\r');
			_outch('\n');
		}
	}
	/*
	 * If auto right margin is not set, then we can't possibly go
	 * past the right edge of the screen.
	 */
	if (!auto_right_margin && SP->phys_x >= columns)
		SP->phys_x = columns - 1;
}
