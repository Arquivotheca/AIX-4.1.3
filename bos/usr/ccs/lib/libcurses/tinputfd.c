#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)06  1.1  src/bos/usr/ccs/lib/libcurses/tinputfd.c, libcurses, bos411, 9428A410j 9/3/93 15:14:19";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: tinputfd
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

/* #ident	"@(#)curses:screen/tinputfd.c	1.1"		*/



#include	"curses_inc.h"

/* Set the input channel for the current terminal. */

void
tinputfd(fd)
int	fd;
{
    cur_term->_inputfd = fd;
    cur_term->_delay = -1;

    /* so that tgetch will reset it to be _inputd */
    /* cur_term->_check_fd = -2; */
}
