static char sccsid[] = "@(#)20  1.6.1.1  src/bos/usr/ccs/lib/libcurses/compat/addstr.c, libcurses, bos411, 9428A410j 9/21/93 17:27:13";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   waddstr
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
 * NAME:        waddstr
 *
 * FUNCTION:
 *
 *      This routine adds a string starting at (_cury,_curx)
 *
 */

 /* Modified by K.E. for increased performance 3/1/93 */


waddstr(win,str)
register WINDOW	*win; 
register char	*str;
{
int x,y;
int lc,fc;
int winmaxx,winmaxy,winbmarg,winscroll,winflags,winattrs;
char *p;
int c,cnext;
int firstx;
int lastx;
chtype *s;
int tt;
int tt2;

# ifdef DEBUG
	if(outf)
	{
		if( win == stdscr )
		{
			fprintf(outf, "WADDSTR(stdscr, ");
		}
		else
		{
			fprintf(outf, "WADDSTR(%o, ", win);
		}
		fprintf(outf, "\"%s\")\n", str);
	}
# endif	DEBUG
/* check if the string has "unusual characters" in it. If so
go to the original code */
p=str;
while (1) {
        tt2= *p-' ';
	if ((unsigned)tt2>=(0177-' ')) break;
	p++;
}
if (tt2!=(0-' ')) goto general_case;
/* otherwise all characters were in the range ' ' <= x < 0177 */
/* use fast inlined version of waddch for the usual cases */
p=str;
x= win->_curx;
y= win->_cury;
winmaxx= win->_maxx;
winmaxy=win->_maxy;
if ((unsigned)x>=winmaxx) return ERR;
if ((unsigned)y>=winmaxy) return ERR;
c= *p++;
/* move out invariant code that can be moved out of the loop */
winattrs = win->_attrs;
firstx= _NOCHANGE;
s= &(win->_y[y][x]);
while(c) {
                cnext= *p++;
		/* place the next character in the input string
		into the window */
		tt=(c|winattrs);
		if (*s!= tt) {
	        	*s = tt;
			/* remember first and last chars that
			actually changed on this line y */
			if (firstx==_NOCHANGE) firstx=x;
                        lastx=x;
		}
		x++;
		s++;
#ifdef NLS
		if (x >= winmaxx && !NCisshift(c))
#else
		if (x >= winmaxx)
#endif
		{
			/* end of line y reached, and not a
			shift character */
                        winbmarg= win->_bmarg;
			/* update first and last changed chars
			on line y */
			if(firstx!=_NOCHANGE) {
				fc = win->_firstch[y];
				lc = win->_lastch[y];
				if( fc != _NOCHANGE )
				{
					if( firstx < fc)
					{
						win->_firstch[y] = firstx;
					}
					if( lastx > lc)
					{
						win->_lastch[y]=lastx;
					}
				}
				else
				{
					win->_firstch[y] = firstx;
					win->_lastch[y] = lastx;
				}
			}
			/* reset column number to 0 */
			x = 0;
			firstx = _NOCHANGE;
			if (++y > winbmarg)
			{
				/* scroll window if bottom of screen
				reached */
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
			s= &(win->_y[y][0]);
		}
		c=cnext;
	}
	/* end of loop */
	/* update the first and last changed characters on 
	current line, unless we already did just update them
	or there were no changed characters */
	if(firstx!=_NOCHANGE) {
		lc = win->_lastch[y];
		fc = win->_firstch[y];
		if( fc != _NOCHANGE )
		{
			 /* use DOZ ? */
			if( firstx < fc)
			{
				win->_firstch[y]= firstx;
			}
			if( lastx > lc)
			{
			        win->_lastch[y]=lastx;
			}
		}
		else
		{
			win->_firstch[y]= firstx;
			win->_lastch[y]= lastx;
		}
	}

        /* commit final x,y position to the window */
	win->_curx = x;
	win->_cury = y;

	return OK;



general_case:
/* this is the original code */

	while( *str )
	{
		if( waddch( win, ( chtype ) *str++ ) == ERR )
		{
			return ERR;
		}
	}
	return OK;
}

