#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)12  1.1  src/bos/usr/ccs/lib/libcurses/ungetch.c, libcurses, bos411, 9428A410j 9/3/93 15:14:41";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: ungetch
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/ungetch.c	1.4"		*/



#include "curses_inc.h"

/* Place a char onto the beginning of the input queue. */

ungetch(ch)
int	ch;
{
    register	int	i = cur_term->_chars_on_queue, j = i - 1;
    register	short	*inputQ = cur_term->_input_queue;

    /* Place the character at the beg of the Q */

    while (i > 0)
	inputQ[i--] = inputQ[j--];
    cur_term->_ungotten++;
    inputQ[0] = -ch - 0100;
    cur_term->_chars_on_queue++;
}
