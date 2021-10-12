static char sccsid[] = "@(#)90	1.10.1.5  src/bos/usr/ccs/lib/libcur/addch.c, libcur, bos411, 9428A410j 8/28/92 16:06:34";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: waddch, mark_change, dounctrl
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <cur04.h>
#include        "cur99.h"
#include 	<stdlib.h>
#include	<ctype.h>
#include	<stdio.h>

extern char _dounctrl;

/*
 * NAME:                waddch
 *
 * FUNCTION: This routine adds the character and
 *      attribute (if different) to the current position in the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      waddch(win, c), where 'win' is a pointer to a window
 *      structure and 'c' is the character to add; the attribute is taken
 *      to be the current window attribute.
 *
 * EXTERNAL REFERENCES: waddch(), scroll_check(), wmove()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

waddch(win, c)
register    WINDOW  *win;
register    wchar_t c;
{
    	int    x, y, cury;
    	register char  *uctr;
	int ret = 0;
	int mbcodeset = (MB_CUR_MAX >1);


    	getyx(win, y, x);		/* set x & y to the current location */
	cury = y;

    	switch (c) {

	case '\t': {		
		/* simulate (expand) tab character */
		register int    newx;

		for (newx = x + (8 - (x & 07)); x < newx && y == cury;){
			if(add_wchar(win, NLSBLANK, &y, &x) == ERR)  {
				return(ERR);
			}
                        if (y == win->_bmarg && x == 0)
                                break ;
		}
		return OK;
	    }

	case '\n': {
		/* simulate <nl>, i.e. clrtoeol and <cr> + <lf> */
		for (; x < win->_maxx && y == cury;) {
			if( add_wchar(win, NLSBLANK, &y, &x) == ERR) 
				return(ERR);
                        if (x == 0)
                                break ;
		}
                return OK;
	    }

	case '\r': 
		/* simulate <cr>, i.e. goto beginning of line */
	    x = 0;
    	    /* reflect adding character by changing current location */
            return(wmove(win, y, x));

	case '\b': 		/* simulate <bs>, i.e. backspace */
	   	if(mbcodeset){
	    		/* 
			** back up two if on second byte of a
			** double display width  character 
			*/
			if(is_second_byte(win->_y[y][x-1]))
				x--;
		}
		x--;
	    	if (x < 0)
			x = 0;
    	    	/* reflect adding character by changing current location */
    	    	return(wmove(win, y, x));

	}	/* end of switch */

	/* just a regular character... */
	
	
	if (_dounctrl && ((c < NLSBLANK ) || c == 0x7f)) {
		uctr = unctrl(c);
		ret = add_wchar(win, uctr[0], &y, &x);
		if(ret == ERR)
			return(ERR);
		add_wchar(win, uctr[1], &y, &x);
		if(ret == ERR)
			return(ERR);
		return OK;
	}
			
	if( !mbcodeset ){
    		/* reflect adding character by changing current location */

		return (add_wchar(win, c, &y, &x)) ;

    	}else { 
		/* case of multibyte code sets */

		/*
		** If the width of the character is greater than one,
		** we store the wide character code followed by
		** WEOF to indicate a double(only) width character.
		*/
		

	    	/* 
		** No room possible for character, so put in partial 
		** character indicator 
		*/
		if (wcwidth(c) > 1) {
			if (win->_maxx == 1) {
				return(add_wchar(win, PART_CHAR, &y, &x));
			}

		    	/* check if we need to wrap to next line */
			if (x >= win->_maxx - 1) {
				if (!is_second_byte(win->_y[y][x])) {
					ret = add_wchar(win, NLSBLANK, &y, &x);
					if (ret == ERR) {
						return(ERR);
					}
				} else {
					x = 0;
					if (++y > win->_bmarg) {
						if(check_scroll(win, y) == ERR){
							return(ERR);
						}
					y = win->_bmarg;
					}
				}
			}
		}

		/* 
		** Now there is enough space to add a double width 
		** character. SO add it.
		*/

		/* If we overlay 2nd column, fix previous column */
		if (is_second_byte(win->_y[y][x])) {
			x--;
			if (add_wchar(win, (wchar_t)PART_CHAR, &y, &x) == ERR) {
				return(ERR);
			}
		}

		/* Add 1st column */
		if (add_wchar(win, c, &y, &x) == ERR) {
			return (ERR);
		}

		/* Add 2nd column, if any */
		if (wcwidth(c) == 2) {
			if (add_wchar(win, (wchar_t)WEOF, &y, &x) == ERR) {
				return (ERR);
			}
		}

		/* If we overlayed the fist column, fix the next column */
		/* Cursor position is at the PART_CHAR */
		if (is_second_byte(win->_y[y][x])) {
			int	tx, ty;
			tx = x;
			ty = y;
			if(add_wchar(win, (wchar_t)PART_CHAR, &ty, &tx) == ERR){
				return(ERR);
			}
			return(wmove(win, y, x));
		}

	}/* End of multibyte code set handling */

	return(OK);
}

/*
 * NAME:                mark_change
 *
 * FUNCTION: update change array
 */

mark_change(win, y, x)
register    WINDOW  *win;
register int    y,
                x;
{
    	if (win->_firstch[y] == _NOCHANGE)
		win->_firstch[y] = win->_lastch[y] = x;
    	else
		if (x > win->_lastch[y])
	    		win->_lastch[y] = x;
    	else
		if (x < win->_firstch[y])
	    		win->_firstch[y] = x;
}

/*
 * NAME:                dounctrl
 *
 * FUNCTION:  _dounctrl is set to reflect argument; controls
 *            printing of control chars in waddch().
 */

dounctrl(keysw)			/* function definition          */
char    keysw;			/* argument is boolean variable */

{
    	_dounctrl = keysw;		/* set global switch            */

    	return(OK);			/* return to caller             */
}					/* end of function              */


/*
**	Name:	add_wchar()
**
** 	Function:	adds a wide character to the screen buffer.
**			It checks for attributes and makes changes.
**			On end of line, simulatea automatic margins.
**			
**	Returns:	ERR on error. Else returns OK.
**			It updates x and y in the win strcuture.
**			It updates the x and y in the waddch().
*/			

static int
add_wchar( win, c, row, col)
register	WINDOW *win;
wchar_t c;
int *row;
int *col;
{
     int x, y;
     y = *row;
     x = *col;

     if (win->_y[y][x]!= c){ 
	  
	  win->_y[y][x] = c;  /* chg character in win */
	  
	  /* Change the attribute if different */
	  if (win->_a[y][x] != win->_csbp)
	       win->_a[y][x] = win->_csbp;
	  
	  mark_change(win, y, x);	/* update change array */
	  
     } else{	/* char is same, check attribute */
	  if (win->_a[y][x] != win->_csbp) {
	       win->_a[y][x] = win->_csbp;
	       mark_change(win, y, x);/* update change array */
	  }
     }

     x++;
     /* Update the  column position */
     *col = x;
     
     if (x >= win->_maxx) {      	/* end of line? */
	  x = 0;			/* simulate automatic margins */
	  /* Update the column position */
	  *col = x;
	  y++;
	  if (y > win->_bmarg) {
	       /* Update the  row position */
	       /* end of line & last line? */
	       if (check_scroll(win, y) == ERR){
		    return ERR;
	       }
	       y = win->_bmarg ;
	       return(wmove(win, win->_bmarg, 0));
	  }
	  *row = y ;
     }

     /* 
      ** Update the screen structure to indicate the new 
      ** x and y positions.
      */
     return(wmove(win, y, x));
}
