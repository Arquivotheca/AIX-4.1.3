#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)61  1.7  src/bos/usr/ccs/lib/libcurses/nocbreak.c, libcurses, bos411, 9428A410j 9/3/93 14:46:39";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: nocbreak
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

/* #ident	"@(#)curses:screen/nocbreak.c	1.5"		*/



#include	"curses_inc.h"

nocbreak()
{
#ifdef	SYSV
    /* See comment in cbreak.c about ICRNL. */
    PROGTTY.c_iflag |= ICRNL;
    PROGTTY.c_lflag |= ICANON;
    PROGTTY.c_cc[VEOF] = _CTRL('D');
    PROGTTY.c_cc[VEOL] = 0;
#else	/* SYSV */
    PROGTTY.sg_flags &= ~(CBREAK | CRMOD);
#endif	/* SYSV */

#ifdef	DEBUG
#ifdef	SYSV
    if (outf)
	fprintf(outf, "nocbreak(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.c_lflag);
#else	/* SYSV */
    if (outf)
	fprintf(outf, "nocbreak(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.sg_flags);
#endif	/* SYSV */
#endif	/* DEBUG */

    cur_term->_fl_rawmode = FALSE;
    cur_term->_delay = -1;
    reset_prog_mode();
#ifdef	FIONREAD
    cur_term->timeout = 0;
#endif	/* FIONREAD */
    return (OK);
}
