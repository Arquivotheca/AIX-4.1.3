static char sccsid[] = "@(#)17	1.7  src/bos/usr/ccs/lib/libcurses/compat/_tscroll.c, libcurses, bos411, 9428A410j 10/10/91 16:11:22";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _tscroll
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
 * NAME:        _tscroll
 */

_tscroll( win )
register WINDOW	*win;
{

	register chtype	*sp;
	register int		i;
	register chtype	*temp;
	register int	top, bot;

#ifdef DEBUG
	if( win == stdscr )
	{
		if(outf) fprintf( outf, "scroll( stdscr )\n" );
	}
	else
	{
		if( win == curscr)
		{
			if(outf) fprintf( outf, "scroll( curscr )\n" );
		}
		else
		{
			if(outf) fprintf( outf, "scroll( %x )\n", win );
		}
	}
#endif
	if( !win->_scroll )
	{
		return ERR;
	}
	/* scroll the window lines themselves up */
	top = win->_tmarg;
	bot = win->_bmarg;
	temp = win->_y[top];
#ifdef	DEBUG
	if(outf)
	{
		fprintf( outf, "top = %d, bot = %d\n", top, bot );
	}
#endif	DEBUG
	for (i = top; i < bot; i++)
	{
	    if (win->_flags & _SUBWIN) { /* need to do char by char copy */
		int j;
		for (j=0; j < win->_maxx; j++)
		    win->_y[i][j] = win->_y[i+1][j];
	    }
	    else 
		win->_y[i] = win->_y[i+1];
	}
	/* Put a blank line in the opened up space */
	if (win->_flags & _SUBWIN) {
	    int j;
	    for (j=0; j < win->_maxx; j++)
		win->_y[bot][j] = ' ';
	}
	else {
	    for (sp = temp; sp - temp < win->_maxx; )
		*sp++ = ' ';
	    win->_y[bot] = temp;
	}
#ifdef	DEBUG
	if(outf)
	{
		fprintf(outf,"SCROLL win [0%o], curscr [0%o], top %d, bot %d\n",
				win, curscr, top, bot);
		fprintf( outf, "Doing --> touchwin( 0%o )\n", win );
	}
#endif	DEBUG
	win->_cury--;
	touchwin(win);
	return OK;
}
