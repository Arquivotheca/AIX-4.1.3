static char sccsid[] = "@(#)01	1.9  src/bos/usr/ccs/lib/libcur/addfld.c, libcur, bos411, 9428A410j 7/16/91 12:07:07";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: waddfld
 *
 * ORIGINS: 10, 27
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

#include        "cur99.h"

/*
 * NAME:                waddfld
 *
 * FUNCTION: This routine adds a field and
 *      attribute (if different) to the current position in the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      waddfld( win, fld, leng, depth, width, attr, fill ), where
 *              win   - pointer to the window
 *              fld   - pointer to field data
 *              leng  - leng of field data
 *              depth - number of rows in field
 *              width - number of columns in field
 *              attr  - attribute byte for field output
 *              fill  -  default fill character for empty field
 *
 * EXTERNAL REFERENCES: waddch(), scroll_check(), wmove()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 *
 */

#define UWEOF 65535		/* unsigned value of WEOF */

waddfld(win, fld, leng, depth, width, attr, fill)
WINDOW  *win;
register char  *fld;
int     leng;
int     depth;
int     width;
int     attr;
NLSCHAR fill;

{
    register char  *last_fld;
    register int    col;
    register int    ocol;
    register    NLSCHAR *ptr_y;
    ATTR    *ptr_a;
    int     new_col;
    register int    max_col;
    register int    row;
    int     max_row;
    register short *first;
    short   *last;
    NLSCHAR c;

/*
 *   Modified - 07/15/91
 */

    NLSCHAR *wchar, wc ; /* work array and NLSchar */
    int stri, wchari, r, i ;    
    int srow, scol, next_tab ;

    getyx(win, row, col);	/* get current row and column   */
    srow = row ;
    scol = col ;

    max_row = row + depth;
    max_col = col + width - 1;
    first = win->_firstch;
    last = win->_lastch;
    last_fld = fld + leng;	/* pointer to 1st char past field */

    if (max_col >= win->_maxx || max_row > win->_maxy)
	return(ERR);
    if (wcwidth(fill) != 1)
	return ERR;

    wchar = (NLSCHAR *) malloc (width * sizeof (NLSCHAR)) ;
    if (wchar == NULL)
        return (ERR) ;

    wchari = 0 ;
    stri = 0 ; /* first position of the string to be examined */
    while (row < max_row) {
        if (* fld != '\0')
            {
            r = mbtowc (&c, fld, MB_CUR_MAX) ;
	    if (r == -1)
               {
               r = 1 ;
               c = fill ;
               }
            fld += r ;
            }   /* if the string still contains multibyte chars,
                   get wchar from the string */
        else
            {
            r = 1 ;
            c = fill ;
            }   /* else, assume that the fill char will be used */

        switch (c) {
	case '\t': /* simulate (expand) tab character */
            next_tab = (wchari + scol) % 8 ;            
	    next_tab = wchari + scol + 8 - next_tab ;
			/* next tab position */
	    if (next_tab <= max_col)
	       {
	       for ( ; wchari + scol < next_tab ; wchari)
		   wchar [wchari++] = fill ;
	       break ;
	       }
				/* else fall into '\n' */

        case '\n': 
				/* simulate <nl>, i.e. clrtoeol and <cr> +
				   <lf> */

	    for (; wchari < width ; wchari++)
                wchar [wchari] = fill ;
	    break;

	case '\r': 
				/* simulate <cr>, i.e. goto beginning of
				   line */
            wchari = 0 ;
            break;

        case '\b': /* simulate <bs>, i.e. backspace */

            if (wchari > 0)
               if (wchar [--wchari] == UWEOF)
                  wchari-- ;
            break ; 

	default:
            
            if (wcwidth (c) != 2)
               wchar [wchari++] = c ;
            else
               {
               if ((width - wchari) == 1)
                  {
                  wchar [wchari++] = fill ;
                  fld -= r ;
                  } /* if there is no space for the double byte char,
                       add a fill to the end of line and restore the
                       fld pointer to the multibyte char */
               else
                  {
                  wchar [wchari++] = c ;
                  wchar [wchari++] = UWEOF ;
		  }
	       }
	}
        if (wchari == width)
           {
           for (wchari = 0 ; wchari < width ; wchari++)
   	       if (wchar [wchari] != UWEOF)
 		  waddch (win, wchar [wchari]) ; 
			/* Do not add a WEOF because addch does it */
	   wchari = 0 ;
           wmove (win, row, scol) ; 
           wchgat (win, width, attr) ;
	   row++ ; /* goto the next row */
           wmove (win, row, scol) ;
	   } 
   }

    wmove (win, srow, scol) ;     /* restore the current coordinates */  
    free (wchar) ;
    return(OK);
}
