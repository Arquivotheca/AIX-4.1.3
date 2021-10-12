static char sccsid[] = "@(#)13	1.3  src/bos/usr/ccs/lib/libcur/inch.c, libcur, bos411, 9428A410j 7/15/94 17:49:45";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecmove, winch
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cur99.h"

#define NIL	0
#define T 	!NIL

extern	short	_ly, _lx;	/* current glass cursor position */
extern  char    _curwin;        /* refresh() on curscr? */
extern	WINDOW	*_win;		/* global copy of 'win' pointer for mvcur */

/*
 * NAME:                winch
 *
 * FUNCTION:    Return character at cursor location in window.
 */

wchar_t
winch (win)
register WINDOW *win;
{
    register    NLSCHAR *p = win->_y[win->_cury]+win->_curx;
    int         ch = (int)(*p);
    wchar_t	wc;

    wc = (wchar_t)ch;

    if ( MB_CUR_MAX > 1 ){
	 char c[2];

	 if ( ((ch)&BYTE_MASK) == FIRST_BYTE  ){ /* First byte? */
#define I_HAVE_NO_IDEA	0
	      c[0] = ch;
	      c[1] = p[1];
	      switch( mbtowc(&wc, c, MB_CUR_MAX) ){
              case -1: case 0:
                    return (wchar_t)I_HAVE_NO_IDEA;
	       }
	 } else if (((ch & BYTE_MASK) == SECOND_BYTE) &&
		    ((p[-1] & BYTE_MASK) == FIRST_BYTE)) { /* second byte ? */
	      c[0] = p[-1];
	      c[1] = ch;
	      switch( mbtowc(&wc, c, MB_CUR_MAX) ){
	      case -1: case 0:
		   return (wchar_t)I_HAVE_NO_IDEA;
	      }

	 }
    }

    return (wc);
}


