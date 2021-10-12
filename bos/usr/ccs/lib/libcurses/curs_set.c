#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)05  1.1  src/bos/usr/ccs/lib/libcurses/curs_set.c, libcurses, bos411, 9428A410j 9/3/93 15:07:13";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: curs_set
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

/* #ident	"@(#)curses:screen/curs_set.c	1.7"		*/



#include	"curses_inc.h"

/* Change the style of cursor in use. */

curs_set(visibility)
register	int	visibility;
{
    extern	int	_outch();
    int		ret = cur_term->_cursorstate;
    char	**cursor_seq = cur_term->cursor_seq;

    if ((visibility < 0) || (visibility > 2) || (!cursor_seq[visibility]))
	ret = ERR;
    else
	if (visibility != ret)
	    tputs(cursor_seq[cur_term->_cursorstate = visibility], 0, _outch);
    (void) fflush(SP->term_file);
    return (ret);
}
