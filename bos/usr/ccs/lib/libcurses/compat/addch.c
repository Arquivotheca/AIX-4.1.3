static char sccsid[] = "@(#)19  1.7.2.1  src/bos/usr/ccs/lib/libcurses/compat/addch.c, libcurses, bos411, 9428A410j 9/21/93 17:26:48";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   waddch
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
#ifdef NLS
static char copy[2];
#endif

/*
 * NAME:        waddch
 *
 * FUNCTION:
 *
 *      This routine prints the character in the current position.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Think of it as putc.
 */

 /* modified by K.E. for increased performance 3/1/93 */

waddch(win, c)
register WINDOW	*win;
register chtype		c;
{
	register int		x, y;
	char *uctr;
	int lc,fc;
	int winmaxx;
	int winmaxy;
	int winbmarg;
	int winscroll;
	int winflags;
	chtype *s;
#ifdef NLS
	register NLchar rawc = c & A_CHARTEXT;
#else
	register char rawc = c & A_CHARTEXT;
#endif

	x = win->_curx;
	y = win->_cury;
	winmaxx= win->_maxx;
	winmaxy= win->_maxy;
	/* (unsigned)x >= winmaxy is used in lieue of
	   (x>=winmaxy|| x<0) */
	if ((unsigned)y >= winmaxy) return ERR;
	if ((unsigned)x >= winmaxx) return ERR;
	s= &(win->_y[y][x]);
	if (rawc!=0177 && 
#ifdef NLS
	     !(rawc<' ' && !NCisshift(rawc))
#else
	     !(rawc<' ') 
#endif
    		) {
		/* the usual case (e.g. alphanumeric characters)
		is handled first */
		/* \n \t \b are all < ' ' and not a shift char*/
		c |= win->_attrs;
		if (*s != c) {
			lc = win->_lastch[y];
			fc = win->_firstch[y];
			if( fc != _NOCHANGE )
			{
				if( x < fc)
				{
					win->_firstch[y] = x;
				}
				else if( x > lc)
				{
					win->_lastch[y] =x;
				}
			}
			else
			{
				win->_firstch[y] = win->_lastch[y]  = x;
			}
			*s = c;
		}
		x++;
#ifdef NLS
		/*
	   	 * Don't worry about overflowing if we're adding a
		 * pending shift character.
		 */

		if (x >= winmaxx && !NCisshift(rawc))
#else
		if (x >= winmaxx)
#endif
		{
			x = 0;
new_line:
                        winbmarg= win->_bmarg;
			if (++y > winbmarg)
			{
				winscroll = win->_scroll;
				winflags=win->_flags;
				if (winscroll && !(winflags&_ISPAD))
				{
					_tscroll( win );
					--y;
				}
				else
				{
					return ERR;
				}
			}
		}
		win->_curx = x;
		win->_cury = y;
		return OK;
	}
	else if (rawc=='\t') 

		{
			register int newx;

			for( newx = x + (8 - (x & 07)); x < newx; x++ )
			{
				if( waddch(win, ' ') == ERR )
				{
					return ERR;
				}
			}
			return OK;
		}
	  else if(rawc=='\n') {
		wclrtoeol(win);
		x = 0;
		goto new_line;
          }
	  else if(rawc=='\b') {
		if (--x < 0)
			x = 0;
		win->_curx = x;
		win->_cury = y;
		return OK;
	  }
	  else 
		{ /* control character */
			uctr = unctrl(rawc);
			waddch(win, (chtype)uctr[0]|(c&A_ATTRIBUTES));
			waddch(win, (chtype)uctr[1]|(c&A_ATTRIBUTES));
			return OK;
		}
}

