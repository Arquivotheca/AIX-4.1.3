#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)64  1.1  src/bos/usr/ccs/lib/libcurses/reset_sh.c, libcurses, bos411, 9428A410j 9/3/93 15:11:23";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: reset_shell_mode
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

/* #ident	"@(#)curses:screen/reset_sh.c	1.5"		*/



#include	"curses_inc.h"

reset_shell_mode()
{
#ifdef	DIOCSETT
    /*
     * Restore any virtual terminal setting.  This must be done
     * before the TIOCSETN because DIOCSETT will clobber flags like xtabs.
     */
    cur_term -> old.st_flgs |= TM_SET;
    (void) ioctl(cur_term->Filedes, DIOCSETT, &cur_term -> old);
#endif	/* DIOCSETT */
    if (_BR(SHELLTTY))
    {
	(void) ioctl(cur_term -> Filedes,
#ifdef	SYSV
	    TCSETAW,
#else	/* SYSV */
	    TIOCSETN,
#endif	/* SYSV */
		    &SHELLTTY);
#ifdef	LTILDE
	if (cur_term -> newlmode != cur_term -> oldlmode)
	    (void) ioctl(cur_term -> Filedes, TIOCLSET, &cur_term -> oldlmode);
#endif	/* LTILDE */
    }
    return (OK);
}
