#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)93  1.6  src/bos/usr/ccs/lib/libcurses/flushinp.c, libcurses, bos411, 9428A410j 9/3/93 14:44:35";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: flushinp
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

/* #ident	"@(#)curses:screen/flushinp.c	1.6"		*/



#include	"curses_inc.h"

flushinp()
{
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "flushinp(), file %x, SP %x\n", cur_term->Filedes, SP);
#endif	/* DEBUG */

#ifdef	SYSV
    (void) ioctl(cur_term -> Filedes, TCFLSH, 0);
#else	/* SYSV */
    /* for insurance against someone using their own buffer: */
    (void) ioctl(cur_term -> Filedes, TIOCGETP, &(PROGTTY));

    /*
     * SETP waits on output and flushes input as side effect.
     * Really want an ioctl like TCFLSH but Berkeley doesn't have one.
     */
    (void) ioctl(cur_term -> Filedes, TIOCSETP, &(PROGTTY));
#endif	/* SYSV */

    /*
     * Get rid of any typeahead which was read().
     * Leave characters which were ungetch()'d.
     */
    cur_term->_chars_on_queue = cur_term->_ungotten;

    /*
     * Have to doupdate() because, if we have stopped output due to
     * typeahead, now that typeahead is gone, so we had better catch up.
     */
    if (_INPUTPENDING)
	(void) doupdate();
    return (OK);
}
