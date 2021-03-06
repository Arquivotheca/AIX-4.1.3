#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)67  1.1  src/bos/usr/ccs/lib/libcurses/ripoffline.c, libcurses, bos411, 9428A410j 9/3/93 15:11:35";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _init_rip_func
 *		ripoffline
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

/* #ident	"@(#)curses:screen/ripoffline.c	1.8"		*/



/*
 * This routine is used by initialization routines. It sets it up
 * such that a line is removed from the user's screen by initscr. This
 * function must be called BEFORE initscr. It works by leaving a cookie 
 * which tells initscr to reduce the size of stdscr by one for each line
 * ripped off. This routine has been generalized so that other applications
 * can make use of it in a straightforward manner.
 */

#include "curses_inc.h"

static	struct _ripdef
{
    int line;
    int (*initfunction)();
} _ripstruct[5];

static char _ripcounter;

static	void
_init_rip_func ()

{
    int     i,flag;

    for (i = 0; i < _ripcounter; i++)
    {
	LINES = --SP->lsize;
/*
 * We don't need to check for newwin returning NULL because even if
 * we did and broke from the for loop, the application's initfunction
 * would not be called and they would have a NULL window pointer.  Their
 * code would then blow up if they don't check it anyway.  Therefore,
 * we can send in the newwin and their code has to check for NULL in either
 * case.
 *
 * NOTE:  The application has the responsibilty to do a delwin !!!
 */
	(*_ripstruct[i].initfunction) (newwin(1, COLS, ((flag = _ripstruct[i].line) > 0) ? 0 : LINES, 0), COLS);
	if (flag > 0)
	    SP->Yabove++;
    }
    _ripcounter = 0;
}

ripoffline (line, initfunction)
int line;
int (*initfunction)();
{
    if (_ripcounter < 5)
    {
	_ripstruct[_ripcounter].line = line;
	_ripstruct[_ripcounter++].initfunction = initfunction;
    }
    _rip_init = _init_rip_func;
    return (OK);
}
