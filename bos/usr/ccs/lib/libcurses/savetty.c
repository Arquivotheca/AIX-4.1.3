#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)84  1.6  src/bos/usr/ccs/lib/libcurses/savetty.c, libcurses, bos411, 9428A410j 9/3/93 14:47:34";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: savetty
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

/* #ident	"@(#)curses:screen/savetty.c	1.6"		*/



/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses_inc.h"

savetty()
{
    SP->save_tty_buf = PROGTTY;
#ifdef DEBUG
# ifdef SYSV
    if (outf)
	fprintf(outf,"savetty(), file %x, SP %x, flags %x,%x,%x,%x\n",
	    cur_term->Filedes, SP, PROGTTY.c_iflag, PROGTTY.c_oflag,
	    PROGTTY.c_cflag, PROGTTY.c_lflag);
# else
    if (outf)
	fprintf(outf, "savetty(), file %x, SP %x, flags %x\n",
	    cur_term->Filedes, SP, PROGTTY.sg_flags);
# endif
#endif
    return (OK);
}
