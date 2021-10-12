#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)71  1.6  src/bos/usr/ccs/lib/libcurses/typeahead.c, libcurses, bos411, 9428A410j 9/3/93 14:49:23";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: typeahead
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

/* #ident	"@(#)curses:screen/typeahead.c	1.5"		*/



#include	"curses_inc.h"

/*
 * Set the file descriptor for typeahead checks to fd.  fd can be -1
 * to disable the checking.
 */

extern	char	*ttyname();

typeahead(fd)
int	fd;
{
#ifdef	SYSV
    /*
     * Doing fcntls before and after each typeahead check 
     * read is a serious problem on the 3b2. Profiling
     * results indicated that a simple program (edit.c from
     * "The New Curses and Terminfo Package") was spending
     * 9.2% of the time in fcntl().
     */

#include	<fcntl.h>
    register	int	savefd = cur_term->_check_fd;
    register	char	*tty;

    /* Close the previous duped file descriptor. */
    if (savefd >= 0)
	(void) close(savefd);

    /*
     * Duplicate the file descriptor so we have one to play with.
     * We cannot use dup(2), unfortunately, so we do typeahead checking
     * on terminals only. Besides, if typeahead is done when input is
     * coming from a file, nothing would EVER get drawn on the screen!
     */

    cur_term->_check_fd = (tty = ttyname(fd)) ? open(tty, O_RDONLY | O_NDELAY) : -1;
#else	/* SYSV */
    int savefd = cur_term->_check_fd;
    /* Only do typeahead checking if the input is a tty. */
    if (isatty(fd))
	cur_term->_check_fd = fd;
    else
	cur_term->_check_fd = -1;
#endif	/* SYSV */
    return (savefd);
}
