static char sccsid[] = "@(#)28  1.6  src/bos/usr/ccs/lib/libcurses/compat/resetshell.c, libcurses, bos411, 9428A410j 6/16/90 01:51:48";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   reset_shell_mode
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
#include <IN/uparm.h>

extern	struct term *cur_term;

#ifdef DIOCSETT

static struct termcb new, old;
#endif
#ifdef LTILDE
static int newlmode, oldlmode;
#endif

#ifdef USG
#define BR(x) (cur_term->x.c_cflag&CBAUD)
#else
#define BR(x) (cur_term->x.sg_ispeed)
#endif

/*
 * NAME:
 *
 * FUNCTION:
 *
 *      Disable CB/UNIX virtual terminals.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Getting the baud rate is different on the two systems.
 *      In either case, a baud rate of 0 hangs up the phone.
 *      Since things are often initialized to 0, getting the phone
 *      hung up on you is a common result of an error in your program.
 *      This is not very friendly, so if the baud rate is 0, we
 *      assume we're doing a reset_xx_mode with no def_xx_mode, and
 *      just don't do anything.
 */

reset_shell_mode()
{
#ifdef DIOCSETT
	/*
	 * Restore any virtual terminal setting.  This must be done
	 * before the TIOCSETN because DIOCSETT will clobber flags like xtabs.
	 */
	old.st_flgs |= TM_SET;
	ioctl(2, DIOCSETT, &old);
#endif
	if (BR(Ottyb)) {
		ioctl(cur_term -> Filedes,
#ifdef USG
			TCSETAW,
#else
			TIOCSETN,
#endif
			&(cur_term->Ottyb));
# ifdef LTILDE
	if (newlmode != oldlmode)
		ioctl(cur_term -> Filedes, TIOCLSET, &oldlmode);
# endif
	}
}
