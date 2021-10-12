static char sccsid[] = "@(#)48  1.5  src/bos/usr/ccs/lib/libcurses/compat/delayoutpt.c, libcurses, bos411, 9428A410j 6/16/90 01:46:29";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   delay_output
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

/*
 * Code for various kinds of delays.  Most of this is nonportable and
 * requires various enhancements to the operating system, so it won't
 * work on all systems.  It is included in curses to provide a portable
 * interface, and so curses itself can use it for function keys.
 */


#include "cursesext.h"
#include <signal.h>

/*
 * NAME:        delay_output
 *
 * FUNCTION:
 *
 *      Delay the output for ms milliseconds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Note that this is NOT the same as a high resolution sleep.  It will
 *      cause a delay in the output but will not necessarily suspend the
 *      processor.  For applications needing to sleep for 1/10th second,
 *      this is not a usable substitute.  It causes a pause in the displayed
 *      output, for example, for the eye wink in snake.  It is
 *      disrecommended for "delay" to be much more than 1/2 second,
 *      especially at high baud rates, because of all the characters it
 *      will output.  Note that due to system delays, the actual pause
 *      could be even more.
 *
 *      Some games won't work decently with this routine.
 */

delay_output(ms)
int ms;
{
	extern int _outchar();		/* it's in putp.c */

	return _delay(ms*10, _outchar);
}
