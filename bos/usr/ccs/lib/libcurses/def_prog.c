#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)43  1.6  src/bos/usr/ccs/lib/libcurses/def_prog.c, libcurses, bos411, 9428A410j 9/3/93 14:43:59";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: def_prog_mode
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

/* #ident	"@(#)curses:screen/def_prog.c	1.3"		*/



#include "curses_inc.h"

def_prog_mode()
{
    /* ioctl errors are ignored so pipes work */
#ifdef SYSV
    (void) ioctl(cur_term -> Filedes, TCGETA, &(PROGTTY));
#else
    (void) ioctl(cur_term -> Filedes, TIOCGETP, &(PROGTTY));
#endif
    return (OK);
}
