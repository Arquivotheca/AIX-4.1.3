static char sccsid[] = "@(#)50  1.5  src/bos/usr/ccs/lib/libcurses/compat/setbuffred.c, libcurses, bos411, 9428A410j 6/16/90 01:52:34";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _setbuffered
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

static short    baud_convert[] =
{
	0, 50, 75, 110, 135, 150, 200, 300, 600, 1200,
	1800, 2400, 4800, 9600, 19200, 38400
};

/*
 * NAME:        _setbuffered
 *
 * FUNCTION:
 *
 *      Force output to be buffered.
 *      Also figures out the baud rate.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Grouped here because they are machine dependent.
 *      This routine is one of the main things
 *      in this level of curses that depends on the outside
 *      environment.
 */

_setbuffered(fd)
FILE *fd;
{
	char *sobuf;
	char *calloc();
	SGTTY   sg;

	sobuf = calloc(1, BUFSIZ);
	setbuf(fd, sobuf);

# ifdef USG
	ioctl (fileno (fd), TCGETA, &sg);
	SP->baud = sg.c_cflag&CBAUD ? baud_convert[sg.c_cflag&CBAUD] : 1200;
# else
	ioctl (fileno (fd), TIOCGETP, &sg);
	SP->baud = sg.sg_ospeed ? baud_convert[sg.sg_ospeed] : 1200;
# endif
}
