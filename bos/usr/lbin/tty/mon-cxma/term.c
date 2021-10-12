static char sccsid[] = "@(#)09	1.1  src/bos/usr/lbin/tty/mon-cxma/term.c, sysxtty, bos411, 9428A410j 6/23/94 16:46:07";
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 */
#include <stdio.h>
#include <termios.h>

static struct termios tbuf;		/* Terminal Structure		*/
static struct termios sbuf;		/* Saved Terminal Structure	*/

void
change_term(int vmin,
	    int vtime)
{
	(void) tcgetattr(fileno(stdout), &tbuf);
	sbuf = tbuf;
	tbuf.c_cc[VMIN] = vmin;
	tbuf.c_cc[VTIME] = vtime;
	(void) tcsetattr(fileno(stdout), TCSANOW, &tbuf);
}

void
restore_term(void)
{
	(void) tcsetattr(fileno(stdout), TCSANOW, &sbuf);
}

