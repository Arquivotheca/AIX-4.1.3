#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)52  1.1  src/bos/usr/ccs/lib/libcurses/outch.c, libcurses, bos411, 9428A410j 9/3/93 15:10:38";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _outch
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

/* #ident	"@(#)curses:screen/outch.c	1.1"		*/



#include	"curses_inc.h"

int	outchcount;

/* Write out one character to the tty and increment outchcount. */

_outch(c)
chtype	c;
{
#ifdef	DEBUG
#ifndef	LONGDEBUG
    if (outf)
	if (c < ' ' || c == 0177)
	    fprintf(outf, "^%c", c^0100);
	else
	    fprintf(outf, "%c", c&0177);
#else	/* LONGDEBUG */
	if (outf)
	    fprintf(outf, "_outch: char '%s' term %x file %x=%d\n",
		unctrl(c&0177), SP, cur_term->Filedes, fileno(SP->term_file));
#endif	/* LONGDEBUG */
#endif	/* DEBUG */

    outchcount++;
    (void) putc((int) c, SP->term_file);
}
